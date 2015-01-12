package mobitnt.net;

import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Properties;

import mobitnt.android.data.AppPackageInfo;
import mobitnt.android.wrapper.AppApi;
import mobitnt.util.*;


public class AppManager extends PageGen {

	public String ProcessRequest(String request, Properties parms) {

		String sAction = parms.getProperty(EADefine.EA_ACTION_TAG,
				EADefine.EA_ACT_GET_APP_LIST);
		if (sAction.contains(EADefine.EA_ACT_REMOVE_APP)) {
			String sAppName = parms.getProperty(EADefine.EA_APP_NAME_TAG,
					"");
			AppApi.RemoveApp(sAppName);
			return GenRetCode(EADefine.EA_RET_NEED_OP_ON_PHONE);
		}
		
		if (sAction.equals(EADefine.EA_ACT_GET_APK_STORE_PATH)){
			String sApkStorePath = AppApi.GetApkStorePath();
			return "<" + EADefine.EA_APK_PATH_TAG + ">" + sApkStorePath + "<" + EADefine.EA_APK_PATH_TAG + "/>"; 
		}
		
		if (sAction.equals(EADefine.EA_ACT_INSTALL_APP)){
			String sApkPath = parms.getProperty(EADefine.EA_APK_PATH_TAG);
			AppApi.InstallApp(sApkPath);
			
			return GenRetCode(EADefine.EA_RET_NEED_OP_ON_PHONE);
		}

		if (sAction.contains(EADefine.EA_ACT_GET_APP_LIST)) {
			return GetAppList();
		}

		return GenRetCode(EADefine.EA_RET_UNKONW_REQ);

	}

	String GetAppList() {

		ArrayList<AppPackageInfo> appList = AppApi.GetAppList();
		if (appList == null || appList.size() < 1) {
			return GenRetCode(EADefine.EA_RET_END_OF_FILE);
		}

		StringBuilder sXml = new StringBuilder(PageGen.m_sXmlHeader);
		sXml.append("<AppList>");
		for (int i = 0; i < appList.size(); ++i) {
			AppPackageInfo appItem = appList.get(i);
			
			sXml.append("<App>");

			sXml.append("<Name>");
			sXml.append(URLEncoder.encode(appItem.appname));
			sXml.append("</Name>");

			sXml.append("<PackageName>");
			sXml.append(URLEncoder.encode(appItem.pname));
			sXml.append("</PackageName>");

			sXml.append("<VersionName>");
			sXml.append(URLEncoder.encode(appItem.versionName));
			sXml.append("</VersionName>");

			sXml.append("</App>");
		}

		sXml.append("</AppList>\r\n");
		return sXml.toString();
	}

}
