package mobitnt.net;

import java.util.Properties;

import mobitnt.android.data.EventInfo;
import mobitnt.android.wrapper.CalendarApi;
import mobitnt.util.EADefine;

public class CalendarManager extends PageGen {

	public String ProcessRequest(String request, Properties parms) {
		if (android.os.Build.VERSION.SDK_INT < 14) {
			return GenRetCode(EADefine.EA_RET_NOT_SUPPORTED);
		}

		String sAction = parms.getProperty(EADefine.EA_ACTION_TAG,
				EADefine.EA_ACT_GET_APP_LIST);

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CALENDAR_LIST)) {
			return CalendarApi.GetCalendarList();
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CALENDAR_EVENT)) {
			String sCalId = parms.getProperty(EADefine.EA_CALENDAR_ID_TAG, "");
			String startDate = parms.getProperty(EADefine.EA_START_DATE_TAG,
					"0");
			return CalendarApi.GetEventList(sCalId, startDate);
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_ADD_CALENDAR_EVENT)) {
			EventInfo evt = new EventInfo();

			String sEvtID = parms.getProperty(EADefine.EA_CALENDAR_EVT_ID_TAG,"0");
			evt.lID = Long.parseLong(sEvtID);
			evt.sTitle = parms.getProperty(EADefine.EA_CALENDAR_TITLE_TAG, "");
			evt.sDescription = parms.getProperty(EADefine.EA_CALENDAR_DESC_TAG,
					"");
			evt.sTimeZone = parms.getProperty(
					EADefine.EA_CALENDAR_TIMEZONE_TAG, "");

			evt.sLoction = parms.getProperty(EADefine.EA_CALENDAR_LOCATION_TAG,
					"");

			evt.sRRULE = parms.getProperty(EADefine.EA_CALENDAR_RRULE_TAG, "");
			String sCalID = parms.getProperty(EADefine.EA_CALENDAR_ID_TAG, "0");
			String sStartTime = parms.getProperty(
					EADefine.EA_CALENDAR_STARTTIME_TAG, "0");
			String sEndTIme = parms.getProperty(
					EADefine.EA_CALENDAR_ENDTIME_TAG, "0");
			String sDuration = parms.getProperty(EADefine.EA_CALENDAR_DURATION_TAG, "0");
			// String sAllDay =
			// parms.getProperty(EADefine.EA_ACT_CALENDAR_ALLDAY_TAG, "0");
			String sHasAlarm = parms.getProperty(
					EADefine.EA_CALENDAR_HASALARM_TAG, "0");
			String sReminderDelta = parms.getProperty(
					EADefine.EA_CALENDAR_REMINDER_DELTA_TAG, "0");

			evt.lStartMillis = Long.parseLong(sStartTime);
			evt.lEndMillis = Long.parseLong(sEndTIme);
			evt.lDuration = Long.parseLong(sDuration);
			evt.lCalID = Long.parseLong(sCalID);
			if (evt.lCalID == 0){
				evt.lCalID = CalendarApi.GetDefaultCalendarID();
				if (evt.lCalID == 0){
					return GenRetCode(EADefine.EA_RET_CAN_NOT_CREATE_DEFAULT_CALENDAR);
				}
			}
			
			evt.lHasReminder = Long.parseLong(sHasAlarm);
			evt.lReminderMinutes = Long.parseLong(sReminderDelta);
			
			int iRet = 0;
			if (evt.lID > 0){
				iRet = CalendarApi.UpdateEvent(evt);
			}else{
				iRet = CalendarApi.InsertEvent(evt);
			}
						
			return GenRetCode(iRet);
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_MAX_CAL_EVT_ID)) {
			String sMaxID = "0";
			String sCalId = parms.getProperty(EADefine.EA_CALENDAR_ID_TAG, "");
			sMaxID = CalendarApi.GetMaxEvtID(sCalId);

			String sXml = m_sXmlHeader;
			sXml += "<MaxID>";
			sXml += sMaxID;
			sXml += "</MaxID>";

			return sXml;
		}

		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);
	}

}
