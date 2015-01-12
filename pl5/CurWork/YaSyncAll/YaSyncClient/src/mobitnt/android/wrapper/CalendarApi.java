package mobitnt.android.wrapper;

import android.annotation.TargetApi;
import android.content.ContentResolver;
import android.content.ContentUris;
import android.content.ContentValues;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Build;
import android.provider.CalendarContract;
import android.provider.CalendarContract.Events;

import java.util.TimeZone;

import mobitnt.android.data.EventInfo;
import mobitnt.net.PageGen;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;


@TargetApi(Build.VERSION_CODES.ICE_CREAM_SANDWICH)
public class CalendarApi {
	static public int InsertEvent(EventInfo evt) {
		if (IsEventExist(evt)) {
			return EADefine.EA_RET_ITEM_ALREADY_EXIST;
		}

		ContentResolver cr = EAUtil.GetContentResolver();
		ContentValues values = new ContentValues();
		values.put(Events.CALENDAR_ID, evt.lCalID);
		values.put(Events.DTSTART, evt.lStartMillis);
		values.put(Events.DTEND, evt.lEndMillis);
		values.put(Events.TITLE, evt.sTitle);
		values.put(Events.VISIBLE, 1);
		values.put(Events.DESCRIPTION, evt.sDescription);
		values.put(Events.EVENT_LOCATION, evt.sLoction);
		values.put(Events.RRULE, evt.sRRULE);
		values.put(Events.DURATION, evt.lDuration);
		TimeZone timeZone = TimeZone.getDefault();
		values.put(CalendarContract.Events.EVENT_TIMEZONE, timeZone.getID());

		try {
			cr.insert(CalendarContract.Events.CONTENT_URI, values);
		} catch (Exception e) {
			return EADefine.EA_RET_FAILED;
		}

		return EADefine.EA_RET_OK;
	}

	static public int UpdateEvent(EventInfo evt) {

		Uri updateUri = ContentUris.withAppendedId(Events.CONTENT_URI, evt.lID);

		ContentResolver cr = EAUtil.GetContentResolver();
		ContentValues values = new ContentValues();
		values.put(Events.CALENDAR_ID, evt.lCalID);
		values.put(Events.DTSTART, evt.lStartMillis);
		values.put(Events.DTEND, evt.lEndMillis);
		values.put(Events.TITLE, evt.sTitle);
		values.put(Events.VISIBLE, 1);
		values.put(Events.DESCRIPTION, evt.sDescription);
		values.put(Events.EVENT_LOCATION, evt.sLoction);
		values.put(Events.RRULE, evt.sRRULE);
		TimeZone timeZone = TimeZone.getDefault();
		values.put(CalendarContract.Events.EVENT_TIMEZONE, timeZone.getID());

		try {
			cr.update(updateUri, values, null, null);
		} catch (Exception e) {
			return EADefine.EA_RET_FAILED;
		}

		return EADefine.EA_RET_OK;
	}

	static public String GetMaxEvtID(String calID) {
		final String sortOrder = "_id DESC";

		String selection = "calendar_id=?";
		String[] selectionArgs = new String[] { calID };

		Cursor cur = EAUtil.GetContentResolver().query(
				CalendarContract.Events.CONTENT_URI, null, selection,
				selectionArgs, sortOrder);
		try {
			if (cur == null || !cur.moveToFirst()) {
				if (cur != null) {
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

	static String GetReminderList(String sEvtID) {
		String selection = "event_id=?";
		String[] selectionArgs = new String[] { sEvtID };

		Cursor cur = EAUtil.GetContentResolver().query(
				CalendarContract.Reminders.CONTENT_URI, null, selection,
				selectionArgs, null);

		if (cur == null) {
			return null;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return null;
		}

		StringBuilder sXml = new StringBuilder();

		// sXml.append("<Reminders>");
		do {
			sXml.append("<Reminder>");
			
			sXml.append(EAUtil.Cursor2Xml(cur));

			sXml.append("</Reminder>");

		} while (cur.moveToNext());
		cur.close();
		// sXml.append("</Reminders>");

		return sXml.toString();
	}

	static boolean IsEventExist(EventInfo evt) {
		if (evt.sTitle.length() < 1) {
			return true;
		}

		String selection = "calendar_id=? and title = ?";
		String[] selectionArgs = new String[] { String.valueOf(evt.lCalID),
				evt.sTitle };

		Cursor cur = EAUtil.GetContentResolver().query(
				CalendarContract.Events.CONTENT_URI, null, selection,
				selectionArgs, null);

		if (cur == null) {
			return false;
		}

		if (!cur.moveToFirst() || cur.getCount() < 1) {
			cur.close();
			return false;
		}

		cur.close();
		return true;
	}

	public static String GetEventList(String calID, String startDate) {

		final String sortOrder = "_id ASC";

		String selection = "calendar_id=? and dtstart >= ?";
		String[] selectionArgs = new String[] { calID, startDate };

		Cursor cur = EAUtil.GetContentResolver().query(
				CalendarContract.Events.CONTENT_URI, null, selection,
				selectionArgs, sortOrder);

		if (cur == null) {
			return null;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return null;
		}

		int iCount = 0;

		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);

		sXml.append("<CalEvents>");
		do {
			if (++iCount > EADefine.EA_RESPONSE_LIST_SIZE) {
				break;
			}

			sXml.append("<Event>");
			
			sXml.append(EAUtil.Cursor2Xml(cur));

			String sEvtID = cur.getString(cur.getColumnIndex("_id"));
			try{
				String sReminders = GetReminderList(sEvtID);
				sXml.append(sReminders);
			}catch (Exception e){
				
			}

			sXml.append("</Event>");

		} while (cur.moveToNext());
		cur.close();
		sXml.append("</CalEvents>");

		return sXml.toString();
	}


	static public String GetCalendarList() {
		long lDefaultCalID = GetDefaultCalendarID();
		
		Uri uri = CalendarContract.Calendars.CONTENT_URI;
		String selection = "visible=1 and _id !=?";
		String[] selectionArgs = new String[] { String.valueOf(lDefaultCalID) };
		Cursor cur = EAUtil.GetContentResolver().query(uri, null, selection,
				selectionArgs, null);
		if (cur == null) {
			return null;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return null;
		}

		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);
		int iCount = 0;
		sXml.append("<Calendars>");
		do {
			if (iCount++ > 30) {
				break;
			}

			sXml.append("<Calendar>");
			sXml.append(EAUtil.Cursor2Xml(cur));
			sXml.append("</Calendar>");

		} while (cur.moveToNext());

		cur.close();

		sXml.append("</Calendars>");

		return sXml.toString();
	}

	static final String DEFAULT_CAL_ACCOUNT_NAME = "YaSync";

	static public void createDefaultCalendar() {
		try{
		Uri calUri = CalendarContract.Calendars.CONTENT_URI;
		ContentValues cv = new ContentValues();
		cv.put(CalendarContract.Calendars.ACCOUNT_NAME,
				DEFAULT_CAL_ACCOUNT_NAME);
		cv.put(CalendarContract.Calendars.ACCOUNT_TYPE,
				CalendarContract.ACCOUNT_TYPE_LOCAL);
		cv.put(CalendarContract.Calendars.NAME, DEFAULT_CAL_ACCOUNT_NAME);
		cv.put(CalendarContract.Calendars.CALENDAR_DISPLAY_NAME,
				DEFAULT_CAL_ACCOUNT_NAME);
		cv.put(CalendarContract.Calendars.CALENDAR_COLOR, Color.GREEN);
		cv.put(CalendarContract.Calendars.CALENDAR_ACCESS_LEVEL,
				CalendarContract.Calendars.CAL_ACCESS_OWNER);
		cv.put(CalendarContract.Calendars.OWNER_ACCOUNT, true);
		cv.put(CalendarContract.Calendars.VISIBLE, 1);
		// cv.put(CalendarContract.Calendars.SYNC_EVENTS, 1);

		calUri = calUri
				.buildUpon()
				.appendQueryParameter(CalendarContract.CALLER_IS_SYNCADAPTER,
						"true")
				.appendQueryParameter(CalendarContract.Calendars.ACCOUNT_NAME,
						DEFAULT_CAL_ACCOUNT_NAME)
				.appendQueryParameter(CalendarContract.Calendars.ACCOUNT_TYPE,
						CalendarContract.ACCOUNT_TYPE_LOCAL).build();

		EAUtil.GetContentResolver().insert(calUri, cv);
		}catch (Exception e) {
			
		}
	}

	static public long GetDefaultCalendarID() {
		long lCalID = getDefaultCalID();
		if (lCalID > 0) {
			return lCalID;
		}

		createDefaultCalendar();
		return getDefaultCalID();
	}

	static long m_lDefaultCalID = 0;
	static long getDefaultCalID() {
		if (m_lDefaultCalID > 0){
			return m_lDefaultCalID;
		}
		
		String[] projection = new String[] { "_id", "name", "visible" };
		String selection = "name = ? AND visible=1";
		String[] selectionArgs = new String[] { DEFAULT_CAL_ACCOUNT_NAME };

		Uri uri = CalendarContract.Calendars.CONTENT_URI;
		Cursor cur = EAUtil.GetContentResolver().query(uri, projection,
				selection, selectionArgs, null);
		if (cur == null) {
			return 0;
		}

		if (!cur.moveToFirst()) {
			cur.close();
			return 0;
		}

		try {
			m_lDefaultCalID = cur.getLong(0);
		} catch (Exception e) {

		}

		cur.close();

		return m_lDefaultCalID;
	}

}
