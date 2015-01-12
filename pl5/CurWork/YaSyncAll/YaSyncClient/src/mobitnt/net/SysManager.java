package mobitnt.net;

import java.net.URLEncoder;
import java.util.Properties;

import mobitnt.android.data.SysEvt;
import mobitnt.android.wrapper.SysApi;
import mobitnt.util.*;

public class SysManager extends PageGen {
	public int m_iReqType;

	public String ProcessRequest(String sReq, Properties parms) {
		String sAction = parms.getProperty(EADefine.EA_ACTION_TAG,
				EADefine.EA_ACT_GET_APP_LIST);
		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_SYS_INFO)) {
			String sRet = SysApi.GetSysInfo();
			if (sRet == null) {
				return GenRetCode(EADefine.EA_RET_QUERY_STATE_LATER);
			}

			return sRet;
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_QUERY_SYS_EVT)) {
			SysEvt evt = SysApi.PopSysEvt();

			StringBuilder sXml = new StringBuilder();

			sXml.append("<Events>");
			
			sXml.append("<EventCode>");
			sXml.append(evt.iEventCode);
			sXml.append("</EventCode>");
			
			sXml.append("<Parameter>");
			sXml.append(URLEncoder.encode(evt.sParameter));
			sXml.append("</Parameter>");
			
			sXml.append("<HasMore>");
			sXml.append(evt.iHasMore);
			sXml.append("</HasMore>");
			
			sXml.append("</Events>");

			return sXml.toString();
		}

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_ACCOUNT_LIST)) {
			return SysApi.GetAccountList();
		}

		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);
	}

}
