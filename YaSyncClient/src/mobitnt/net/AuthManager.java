package mobitnt.net;

import java.util.Properties;
import mobitnt.android.wrapper.SysApi;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;

public class AuthManager extends PageGen {

	public String ProcessRequest(String request, Properties parms) {
		try {
			String sAction = parms.getProperty(EADefine.EA_ACTION_TAG, "n");

			if (sAction.equalsIgnoreCase(EADefine.EA_ACT_REQ_AUTH)) {
				SysApi.m_iAuthState = EADefine.EA_RET_OK;

				if (EAUtil.IsSecurityEnabled()) {
					String sSecurityCode = parms.getProperty(
							EADefine.EA_SECURITY_CODE_TAG, "");

					if (sSecurityCode.length() < 1
							|| !sSecurityCode.equals(EAUtil
									.GetSecurityCode(false))) {
						SysApi.m_iAuthState = EADefine.EA_RET_ACCESS_DENINED;
					}
				}

				SysApi.ClearEvtList();

				String sRet = SysApi.GetSysInfo();
				if (sRet == null) {
					return GenRetCode(EADefine.EA_RET_AUTH_FAILED);
				}

				return sRet;
			}

			return GenRetCode(EADefine.EA_RET_UNKONW_REQ);

		} catch (Exception e) {
			return GenRetXml(e.toString(), 0);
		}
	}

}
