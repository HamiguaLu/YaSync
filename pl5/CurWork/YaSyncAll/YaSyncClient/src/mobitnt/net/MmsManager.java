package mobitnt.net;

import java.util.Properties;

import mobitnt.android.wrapper.MmsApi;
import mobitnt.util.EADefine;

public class MmsManager extends PageGen {

	public String ProcessRequest(String request, Properties parms) {

		String sAction = parms.getProperty(EADefine.EA_ACTION_TAG,
				EADefine.EA_ACT_GET_APP_LIST);

		if (sAction.equalsIgnoreCase(EADefine.EA_ACT_GET_MMS_LIST)) {
			String sFrom = parms.getProperty(EADefine.EA_START_DATE_TAG, "0");
			return MmsApi.GetMmsList(sFrom);

		} else if (sAction.equalsIgnoreCase(EADefine.EA_ACT_DEL_MMS)) {

		} 
		
		return sAction;
	}

}
