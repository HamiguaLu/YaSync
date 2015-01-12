package mobitnt.android.wrapper;

import java.util.ArrayList;
import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import mobitnt.net.PageGen;
import mobitnt.util.DBHelper;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.telephony.SmsManager;

public class SmsApi {

	static public final String SMS_URI_ALL = "content://sms/"; // 0
	static public final Uri mSmsUri = Uri.parse("content://sms/inbox");

	static public int GetSmsCount() {
		ContentResolver cr = EAUtil.GetContentResolver();
		Cursor cur = cr.query(Uri.parse(SMS_URI_ALL), null, null, null,
				"_id DESC");

		if (cur == null) {
			return 0;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return 0;
		}

		int iCount = cur.getCount();

		cur.close();

		return iCount;
	}

	static public String GetMaxSmsID() {
		ContentResolver cr = EAUtil.GetContentResolver();
		Cursor cur = cr.query(Uri.parse(SMS_URI_ALL), null, null, null,
				"_id DESC");

		try {
			if (cur == null || !cur.moveToFirst()) {
				return "0";
			}

			int iCallIDIndex = cur.getColumnIndexOrThrow("_id");
			String sID = cur.getString(iCallIDIndex);

			cur.close();

			return sID;

		} catch (Exception e) {
			cur.close();
			return "0";
		}
	}

	public static boolean IsSmsExist(String sBody, String sPhone) {

		String selection = "body=? and address = ?";
		String[] selectionArgs = new String[] { sBody, sPhone };

		Cursor cursor = EAUtil.GetContentResolver().query(
				Uri.parse(SMS_URI_ALL), null, selection, selectionArgs, null);
		if (cursor == null || cursor.getCount() < 1 || !cursor.moveToFirst()) {
			if (cursor != null) {
				cursor.close();
			}
			return false;
		}

		return true;
	}

	public static String GetSmsList(String startDate) {
		String sortOrder = "date ASC";

		String selection = "date >= ?";
		String[] selectionArgs = new String[] { startDate };

		Cursor cursor = EAUtil.GetContentResolver().query(
				Uri.parse(SMS_URI_ALL), null, selection, selectionArgs,
				sortOrder);

		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);
		sXml.append("<SmsList>");
		if (cursor == null || cursor.getCount() < 1 || !cursor.moveToFirst()) {
			if (cursor != null) {
				cursor.close();
			}
			sXml.append("</SmsList>");
			return sXml.toString();
		}

		int iCount = 0;

		do {
			if (++iCount > EADefine.EA_RESPONSE_LIST_SIZE) {
				break;
			}

			sXml.append("<SMS>");
			sXml.append(EAUtil.Cursor2Xml(cursor));

			String sPhoneNo = cursor
					.getString(cursor.getColumnIndex("address"));
			String sName = ContactApi.getContactNameByPhoneNum(sPhoneNo);
			sXml.append("<Name>");
			sXml.append(EAUtil.EncodeItem(sName));
			sXml.append("</Name>");

			sXml.append("</SMS>");

		} while (cursor.moveToNext());
		cursor.close();

		sXml.append("</SmsList>");
		return sXml.toString();
	}

	public static void insertsms(String sPhoneNo, String sBody, String sRead,
			String sType, long lDate) {
		if (IsSmsExist(sBody, sPhoneNo)) {
			return;
		}

		ContentValues values = new ContentValues();
		values.put("address", sPhoneNo);
		values.put("body", sBody);
		values.put("date", lDate);
		values.put("read", 1);
		values.put("type", sType);
		EAUtil.GetContentResolver().insert(mSmsUri, values);
	}

	static public int DeleteSms(String threadID, String id) {
		int iCount = 0;
		if (id == null || id.length() < 1) {
			iCount = EAUtil.GetContentResolver().delete(
					Uri.parse("content://sms/conversations/" + threadID), null,
					null);
		} else {
			iCount = EAUtil.GetContentResolver().delete(
					Uri.parse("content://sms/conversations/" + threadID),
					"_id = ?", new String[] { id });
		}

		if (iCount < 1) {
			return EADefine.EA_RET_FAILED;
		}

		return EADefine.EA_RET_OK;
	}

	static public String GetPhoneNo(String sPhoneEntry) {
		if (sPhoneEntry == null || sPhoneEntry.length() < 1) {
			return "";
		}

		String sNo = "";
		for (int i = 0; i < sPhoneEntry.length(); ++i) {
			char c = sPhoneEntry.charAt(i);
			if (Character.isDigit(c)) {
				sNo += c;
			}
		}

		return sNo;
	}

	static int sendSMS(String sPhone, String message, String sTimeStamp) {
		
		String sVal = sPhone + sTimeStamp;
		sPhone = GetPhoneNo(sPhone);

		/*
		 * Pattern pattern = Pattern.compile("[0-9]*"); Matcher isNum =
		 * pattern.matcher(sPhone); if (!isNum.matches()) { String phoneNo =
		 * sVal; phoneNo += "=" +
		 * String.valueOf(EADefine.EA_RET_INVALID_PHONE_NO);
		 * SysApi.PushSysEvt(SysApi.SYS_EVT_SMS_SENT_STATUS, phoneNo); return
		 * EADefine.EA_RET_INVALID_PHONE_NO; }
		 */

		String SENT = "SMS_SENT" + System.currentTimeMillis();
		String DELIVERED = "SMS_DELIVERED" + System.currentTimeMillis();

		Intent sentIntent = new Intent(SENT);
		sentIntent.putExtra("timestamp", sVal);
		sentIntent.putExtra("smscontent", message);
		sentIntent.putExtra("phoneNo", sPhone);
		PendingIntent sentPI = PendingIntent.getBroadcast(
				EAUtil.GetEAContext(), (int) System.currentTimeMillis(),
				sentIntent, 0);

		Intent deliveredIntent = new Intent(DELIVERED);
		deliveredIntent.putExtra("timestamp", sVal);
		deliveredIntent.putExtra("smscontent", message);
		deliveredIntent.putExtra("phoneNo", sPhone);

		PendingIntent deliveredPI = PendingIntent.getBroadcast(
				EAUtil.GetEAContext(), (int) System.currentTimeMillis(),
				deliveredIntent, 0);

		// ---when the SMS has been sent---
		EAUtil.GetEAContext().registerReceiver(new BroadcastReceiver() {
			@Override
			public void onReceive(Context arg0, Intent arg1) {
				String timestamp = arg1.getExtras().getString("timestamp");
				String phoneNo = arg1.getExtras().getString("phoneNo");
				String sContent = arg1.getExtras().getString("smscontent");
				String sState = timestamp;
				switch (getResultCode()) {
				case Activity.RESULT_OK:
					sState += ":" + String.valueOf(EADefine.EA_RET_OK) + ":"
							+ phoneNo;
					long lDate = new java.util.Date().getTime();
					insertsms(phoneNo, sContent, "0", "2", lDate);
					break;
				case SmsManager.RESULT_ERROR_GENERIC_FAILURE:
				case SmsManager.RESULT_ERROR_NO_SERVICE:
				case SmsManager.RESULT_ERROR_NULL_PDU:
				case SmsManager.RESULT_ERROR_RADIO_OFF:
				default:
					sState += ":" + String.valueOf(EADefine.EA_RET_FAILED)
							+ ":" + phoneNo;
					break;
				}
				SysApi.PushSysEvt(EADefine.SYS_EVT_SMS_SENT_STATUS, sState);
			}
		}, new IntentFilter(SENT));

		// ---when the SMS has been delivered---
		EAUtil.GetEAContext().registerReceiver(new BroadcastReceiver() {

			@Override
			public void onReceive(Context arg0, Intent arg1) {
				String timestamp = arg1.getExtras().getString("timestamp");
				String phoneNo = arg1.getExtras().getString("phoneNo");
				// String sContent = arg1.getExtras().getString("smscontent");
				String sState = timestamp;
				switch (getResultCode()) {
				case Activity.RESULT_OK:
					sState += ":" + String.valueOf(EADefine.EA_RET_OK) + ":"
							+ phoneNo;
					break;
				case Activity.RESULT_CANCELED:
				default:
					sState += ":" + String.valueOf(EADefine.EA_RET_FAILED)
							+ ":" + phoneNo;
					break;
				}

				SysApi.PushSysEvt(EADefine.SYS_EVT_SMS_DELIVER_STATUS, sState);
			}
		}, new IntentFilter(DELIVERED));

		try {
			SmsManager smsMgr = SmsManager.getDefault();
			ArrayList<String> msgArray = smsMgr.divideMessage(message);
			int iSize = msgArray.size();
			if (iSize  > 1){
				
				smsMgr.sendMultipartTextMessage(sPhone, null, msgArray, null, null);
				String sState = sVal;
				sState += ":" + String.valueOf(EADefine.EA_RET_OK) + ":" + sPhone;
				long lDate = new java.util.Date().getTime();
				insertsms(sPhone,message, "0", "2", lDate);
				SysApi.PushSysEvt(EADefine.SYS_EVT_SMS_SENT_STATUS, sState);
			}else{
				smsMgr.sendTextMessage(sPhone, null, message, sentPI, deliveredPI);
			}
		} catch (Exception e) {
			String sState = ":" + String.valueOf(EADefine.EA_RET_FAILED) + ":"
					+ sPhone;
			SysApi.PushSysEvt(EADefine.SYS_EVT_SMS_SENT_STATUS, sState);
			return EADefine.EA_RET_FAILED;
		}

		return EADefine.EA_RET_OK;
	}

	public static void DelScheduledSms(String sID) {
		DBHelper db = new DBHelper();
		db.delete(sID);
	}

	public static void AddScheduledSms(String sTo, String sContent, String sTime) {
		if (sContent == null || sContent.length() < 1) {
			// should never happen
			return;
		}

		if (sTo == null || sTo.length() < 1) {
			return;
		}
		
		DBHelper db = new DBHelper();
		db.insert(sTo, sContent, sTime);

		ScheduleSms(Long.parseLong(sTime));
	}

	public static void ReScheduleSms() {
		DBHelper db = new DBHelper();
		Cursor cursor = db.query();
		if (cursor == null || cursor.getCount() < 1 || !cursor.moveToFirst()) {
			if (cursor != null) {
				cursor.close();
			}

			return;
		}

		do {
			String sID = cursor.getString(0);
			String sTime = cursor.getString(3);

			long lTime = Long.parseLong(sTime);
			long lNowTime = new Date().getTime();

			lNowTime = Math.abs(lNowTime - lTime) / 1000;
			if (lNowTime > 1000 * 60 * 60 * 12) {
				// discard missed SMS
				db.delete(sID);
				continue;
			}

			ScheduleSms(lTime);

		} while (cursor.moveToNext());
		cursor.close();
	}

	public static boolean ScheduleSms(long lSendTime) {
		Date now = new Date();
		long lNowTime = now.getTime();
		if (lSendTime < lNowTime) {
			return false;
		}

		lSendTime -= lNowTime;

		new Timer().schedule(new TimerTask() {
			@Override
			public void run() {
				DBHelper db = new DBHelper();
				Cursor cursor = db.query();
				if (cursor == null || cursor.getCount() < 1
						|| !cursor.moveToFirst()) {
					if (cursor != null) {
						cursor.close();
					}

					return;
				}

				do {
					String sTime = cursor.getString(3);
					long lTime = Long.parseLong(sTime);
					long lNowTime = new Date().getTime();

					lNowTime = Math.abs(lNowTime - lTime) / 1000;
					if (lNowTime > 1000 * 60) {
						continue;
					}

					String sID = cursor.getString(0);
					String sTo = cursor.getString(1);
					String sContent = cursor.getString(2);

					SmsApi.SendSMS(sTo, sContent, "");

					db.delete(sID);

				} while (cursor.moveToNext());
				cursor.close();
			}
		}, lSendTime);

		return true;
	}

	public static String GetScheduledSmsList() {
		DBHelper db = new DBHelper();

		Cursor cursor = db.query();
		StringBuilder sXml = new StringBuilder();
		sXml.append("<SmsList>");
		if (cursor == null || cursor.getCount() < 1 || !cursor.moveToFirst()) {
			if (cursor != null) {
				cursor.close();
			}
			sXml.append("</SmsList>");
			return sXml.toString();
		}

		int iCount = 0;

		do {
			if (++iCount > EADefine.EA_RESPONSE_LIST_SIZE) {
				break;
			}

			sXml.append("<SMS>");
			sXml.append(EAUtil.Cursor2Xml(cursor));
			sXml.append("</SMS>");

		} while (cursor.moveToNext());
		cursor.close();

		sXml.append("</SmsList>");
		return sXml.toString();
	}

	public static int SendSMS(String sPhones, String sContent, String sTimeStamp) {

		if (sContent == null || sContent.length() < 1) {
			// should never happen
			return EADefine.EA_RET_INVALID_SMS_CONTENT;
		}

		if (sPhones == null || sPhones.length() < 1) {
			return EADefine.EA_RET_INVALID_PHONE_NO;
		}

		String[] sPhoneList = sPhones.split(",");
		if (sPhoneList.length < 1) {
			return EADefine.EA_RET_INVALID_PHONE_NO;
		}

		// new a sms
		for (int i = 0; i < sPhoneList.length; ++i) {
			if (sPhoneList[i].length() < 2) {
				// skip invalid number
				continue;
			}

			sendSMS(sPhoneList[i], sContent, sTimeStamp);
		}

		return EADefine.EA_RET_QUERY_STATE_LATER;
	}

}
