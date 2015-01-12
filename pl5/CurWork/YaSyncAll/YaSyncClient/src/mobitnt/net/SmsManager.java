package mobitnt.net;

import java.util.Properties;
import mobitnt.android.wrapper.SmsApi;
import mobitnt.util.EADefine;


public class SmsManager extends PageGen {

	
	static public String m_sSender = "";
	static public String m_sPhone = "";

	public String ProcessRequest(String request, Properties parms) {
		String sAction = parms.getProperty(EADefine.EA_ACTION_TAG, "n");
	
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_SEND_SMS)) {
			/* ·¢ËÍ¶ÌÐÅ */
			String sContent = parms.getProperty(EADefine.EA_SMS_CONTENT_TAG, "");
			String sPhones = parms.getProperty(EADefine.EA_SMS_RECEIVER_TAG, "");
			String sTimeStamp = parms.getProperty(EADefine.EA_SMS_TIMESTAMP_TAG,"");
			int iRet = SmsApi.SendSMS(sPhones, sContent, sTimeStamp);
			return GenRetCode(iRet);
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_SCHEDULE_SMS_LIST)) {
			return SmsApi.GetScheduledSmsList();
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_ADD_SCHEDULE_SMS)) {
			String sContent = parms.getProperty(EADefine.EA_SMS_CONTENT_TAG, "");
			String sPhones = parms.getProperty(EADefine.EA_SMS_RECEIVER_TAG, "");
			String sScheduleTime = parms.getProperty(EADefine.EA_SMS_SCHEDULE_TIME_TAG, "");
			SmsApi.AddScheduledSms(sPhones, sContent, sScheduleTime);
			return GenRetCode(EADefine.EA_RET_OK);
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_DELETE_SMS)) {
			String id = parms.getProperty(EADefine.EA_ID_TAG, "");
			String threadID = parms.getProperty(EADefine.EA_THREAD_ID_TAG, "");
			int iRet = SmsApi.DeleteSms(threadID, id);
			return GenRetCode(iRet);
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_DEL_SCHEDULE_SMS)) {
			String sID = parms.getProperty(EADefine.EA_ID_TAG, "");
			SmsApi.DelScheduledSms(sID);
			return GenRetCode(EADefine.EA_RET_OK);
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_SMS_COUNT)) {
			int iTotalCount = SmsApi.GetSmsCount();
			String sItemFmt = "<ItemCount>%d</ItemCount>";
			String sXml = String.format(sItemFmt, iTotalCount);
			return sXml;
		}
		
		if (sAction.equalsIgnoreCase("getnameandphone")){
			if (m_sSender.length() < 1 && m_sPhone.length() < 1){
				return GenRetCode(EADefine.EA_RET_END_OF_FILE);
			}
			
			return "<sender><name>" + m_sSender + "</name>" + "<phone>" + m_sPhone + "</phone></sender>";
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_MAX_SMS_ID)) {
			String sMaxID = SmsApi.GetMaxSmsID();
						
			String sXml = m_sXmlHeader;
			sXml += "<MaxID>";
			sXml += sMaxID;
			sXml += "</MaxID>";
			
			return sXml;
		} 
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_SMS_LIST)) {
			String startDate = parms.getProperty(EADefine.EA_START_DATE_TAG, "0");
			return SmsApi.GetSmsList(startDate);
		}
		
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_RESTORE_SMS)) {
			//String sName = parms.getProperty("name", "0");
			String sNumber = parms.getProperty("phone", "0");
			String sBody = parms.getProperty("body", "0");
			String sType = parms.getProperty("type", "0");
			String sDate = parms.getProperty("date", "0");
			SmsApi.insertsms(sNumber, sBody, "0", sType,Long.parseLong(sDate));
			return GenRetCode(EADefine.EA_RET_OK);
		}

		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);

	}

}
