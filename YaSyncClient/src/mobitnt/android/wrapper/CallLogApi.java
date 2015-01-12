package mobitnt.android.wrapper;

import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;

import android.content.ContentResolver;
import android.content.ContentValues;
import android.database.Cursor;
import android.provider.CallLog;

public class CallLogApi {
	
	static public int GetCallLogCount() {
		ContentResolver cr = EAUtil.GetContentResolver();
		Cursor cur = cr
				.query(CallLog.Calls.CONTENT_URI, null, null, null, null);
		if (cur == null){
			return 0;
		}
		
		int iCount = cur.getCount();
		cur.close();
		return iCount;
	}
	
	static public String GetMaxCallLogID() {
		ContentResolver cr = EAUtil.GetContentResolver();
		Cursor cur = cr.query(CallLog.Calls.CONTENT_URI, null, null, null,
				"_id DESC");

		try {
			if (cur == null || !cur.moveToFirst()) {
				if (cur != null){
					cur.close();
				}
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

	static public String GetCallLogList(String startDate) {
		ContentResolver cr = EAUtil.GetContentResolver();

		String selection = "date >= ?";
		String[] selectionArgs = new String[] { startDate };

		Cursor cur = cr.query(CallLog.Calls.CONTENT_URI, null, selection,
				selectionArgs, "date ASC");

		if (cur == null || !cur.moveToFirst()) {
			if (cur != null){
				cur.close();
			}
			return null;
		}

		int iCount = 0;
		
		StringBuilder sXml = new StringBuilder();

		do {
			if (++iCount > EADefine.EA_RESPONSE_LIST_SIZE){
				break;
			}
			
			sXml.append("<Calllog>");
			sXml.append(EAUtil.Cursor2Xml(cur));
			
			String sPhoneNo = cur.getString(cur.getColumnIndex(CallLog.Calls.NUMBER));
			String sName = ContactApi.getContactNameByPhoneNum(sPhoneNo);
			sXml.append("<NAME>");
			sXml.append(EAUtil.EncodeItem(sName));
			sXml.append("</NAME>");

			sXml.append("</Calllog>");

		} while (cur.moveToNext());
		cur.close();
		return sXml.toString();
	}

	static public void deleteCall(String id){
		ContentResolver resolver = EAUtil.GetContentResolver();
		resolver.delete(CallLog.Calls.CONTENT_URI, "_id = ?", new String[] { id });
	}
	
	public static boolean IsCallExist(String sNumber,String sDate) {

		String selection = "number=? and date = ?";
		String[] selectionArgs = new String[] {sNumber,sDate};
		Cursor cur = EAUtil.GetContentResolver().query(CallLog.Calls.CONTENT_URI, null, selection,
				selectionArgs, null);
		if (cur == null || cur.getCount() < 1 || !cur.moveToFirst()) {
			if (cur != null) {
				cur.close();
			}
			return false;
		}

		return true;
	}
	
	static public void InsertCallLog(String sName,String sNumber,String sType,String sStartTime,String sDuration){
		ContentValues values = new ContentValues();
		values.put(CallLog.Calls.CACHED_NAME, sName);
		values.put(CallLog.Calls.NUMBER, sNumber);
		values.put(CallLog.Calls.DATE, sStartTime);
		values.put(CallLog.Calls.DURATION, sDuration);
		values.put(CallLog.Calls.TYPE, sType);// 未接
		values.put(CallLog.Calls.NEW, "0");// 0已看1未看

		EAUtil.GetContentResolver()
				.insert(CallLog.Calls.CONTENT_URI, values);

	}
	
}
