package mobitnt.net;

import java.util.Properties;
import mobitnt.android.wrapper.CallLogApi;
import mobitnt.util.EADefine;

public class CallLogManager extends PageGen {
	public String ProcessRequest(String request, Properties parms) {
		try {
			String sAction = parms.getProperty(EADefine.EA_ACTION_TAG, "n");

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_DELETE_CALL)) {
				String sID = parms.getProperty(EADefine.EA_ID_TAG, "0");
				CallLogApi.deleteCall(sID);
				return GenRetCode(EADefine.EA_RET_OK);
			}

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CALL_COUNT)) {
				int iTotalCount = CallLogApi.GetCallLogCount();
				String sItemFmt = "<ItemCount>%d</ItemCount>";
				String sXml = String.format(sItemFmt, iTotalCount);
				return sXml;
			}

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_MAX_CALL_ID)) {
				String sMaxID = CallLogApi.GetMaxCallLogID();

				String sXml = m_sXmlHeader;
				sXml += "<MaxID>";
				sXml += sMaxID;
				sXml += "</MaxID>";

				return sXml;
			}

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_CALL_LIST)) {
				String startDate = parms.getProperty(
						EADefine.EA_START_DATE_TAG, "0");

				StringBuilder sXml = new StringBuilder(m_sXmlHeader);
				sXml.append("<CallLogList>");

				String sCalls = CallLogApi.GetCallLogList(startDate);
				if (sCalls != null) {
					sXml.append(sCalls);
				}

				sXml.append("</CallLogList>");
				
				return sXml.toString();
			}

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_RESTORE_CALL)) {
				String sName = parms.getProperty("name", "0");
				String sNumber = parms.getProperty("phone", "0");
				String sDuration = parms.getProperty("duration", "0");
				String sType = parms.getProperty("type", "0");
				String sStartTime = parms.getProperty("starttime", "0");
				// String sEndTime = parms.getProperty("endtime", "0");
				CallLogApi.InsertCallLog(sName, sNumber, sType, sStartTime,
						sDuration);
				return GenRetCode(EADefine.EA_RET_OK);
			}

			return GenRetCode(EADefine.EA_RET_UNKONW_REQ);
		} catch (Exception e) {
			return GenRetXml(e.toString(), 0);
		}
	}
}
