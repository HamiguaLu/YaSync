package mobitnt.android.wrapper;


import java.io.File;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import mobitnt.android.data.SysEvt;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;
import android.accounts.Account;
import android.accounts.AccountManager;
import android.os.Environment;
import android.os.StatFs;

public class SysApi {
	static public int m_iAuthState;
	
	static public int m_iBatteryLevel;

	static public String GetSysInfo() {
		String sEntryFormat = "<%s>%s</%s>";

		StringBuilder sXml = new StringBuilder();
		sXml.append("<SysInfoList>");

		List<String> sSysInfo = getSysInfo();
		if (sSysInfo == null || sSysInfo.size() < 1) {
			return null;
		}

		for (int i = 0; i < sSysInfo.size(); ++i) {
			String sItmes[] = sSysInfo.get(i).split(":");
			if (sItmes.length == 2) {
				sXml.append(String.format(sEntryFormat, sItmes[0],
						URLEncoder.encode(sItmes[1]), sItmes[0]));
			}
		}

		sXml.append("<PhoneID>");
		sXml.append(URLEncoder.encode(SysApi.getImei()));
		sXml.append("</PhoneID>");
		
		sXml.append("<BatteryLevel>");
		sXml.append(m_iBatteryLevel);
		sXml.append("</BatteryLevel>");
		
		sXml.append("<AppVer>");
		sXml.append(EADefine.PE_APP_VER);
		sXml.append("</AppVer>");

		sXml.append("<AuthState>");
		sXml.append(m_iAuthState);
		sXml.append("</AuthState>");

		sXml.append("<OS_VER>");
		String sVer = android.os.Build.VERSION.SDK + "," + android.os.Build.VERSION.RELEASE;
		sXml.append(URLEncoder.encode(sVer));
		sXml.append("</OS_VER>");

		sXml.append("<SDK_VER>");
		sXml.append(android.os.Build.VERSION.SDK_INT);
		sXml.append("</SDK_VER>");

		sXml.append("</SysInfoList>");

		return sXml.toString();
	}

	static List<String> getSysInfo() {
		String[] sStorageInfo = getStorageInfo();

		List<String> sysInfo = new ArrayList<String>();

		if (sStorageInfo != null) {
			for (int i = 0; i < sStorageInfo.length; ++i) {
				sysInfo.add(sStorageInfo[i]);
			}
		}

		sysInfo.add(EADefine.EA_SYS_PRODUCT_MANUFACTURER_TAG + ":"
				+ android.os.Build.MANUFACTURER);
		sysInfo.add(EADefine.EA_SYS_PRODUCT_MODEL_TAG + ":"
				+ android.os.Build.MODEL);

		return sysInfo;
	}

	static public String getImei() {
		return EAUtil.GetPhoneID();
	}

	static String[] getStorageInfo() {
		String sdcard = Environment.getExternalStorageDirectory().getPath();
		File file = new File(sdcard);
		StatFs statFs = new StatFs(file.getPath());
		
		long bs = statFs.getBlockSize();
		long ab = statFs.getAvailableBlocks();
		long bc = statFs.getBlockCount();
		
		long lAvailableSpace = bs * (ab - 4);
		long lTotalSpace = bc * bs;

		// String sASize = formatSize(lAvailableSpace);
		// String sTotalSize = formatSize(lTotalSpace);

		String[] storageInfo = {
				EADefine.EA_SYS_EXT_AVAILABLE_SPACE + ":"
						+ String.valueOf(lAvailableSpace),
				EADefine.EA_SYS_EXT_TOTAL_SPACE + ":"
						+ String.valueOf(lTotalSpace) };

		return storageInfo;
	}

	/*
	 * String getSysUpTime() { long ut = SystemClock.elapsedRealtime() / 1000;
	 * if (ut == 0) { ut = 1; } int m = (int) ((ut / 60) % 60); int h = (int)
	 * ((ut / 3600)); return String.valueOf(h); }
	 */

	static  List<SysEvt> m_SysEvtlist = new Vector<SysEvt>();

	static public synchronized void ClearEvtList() {
		m_SysEvtlist.clear();
		PushSysEvt(EADefine.SYS_EVT_BATTERY_LEVEL_CHANGED, String.valueOf(m_iBatteryLevel));
	}

	static public synchronized SysEvt PopSysEvt() {
		if (m_SysEvtlist.size() < 1) {
			SysEvt evt = new SysEvt();
			evt.iEventCode = EADefine.SYS_EVT_NONE;
			return evt;
		}

		SysEvt evt = m_SysEvtlist.get(0);
		m_SysEvtlist.remove(0);

		evt.iHasMore = 0;
		if (m_SysEvtlist.size() > 0) {
			evt.iHasMore = 1;
		}

		return evt;
	}

	static public synchronized void PushSysEvt(int iEvt, String sParameter) {
		SysEvt evt = new SysEvt();
		evt.iEventCode = iEvt;
		evt.sParameter = sParameter;
		if (m_SysEvtlist.size() < 1) {
			m_SysEvtlist.add(evt);
			return;
		}

		if (m_SysEvtlist.size() > 1000) {
			ClearEvtList();
			return;
		}

		for (int i = 0; i < m_SysEvtlist.size(); ++i) {
			if (m_SysEvtlist.get(i).iEventCode != iEvt) {
				continue;
			}
			if (m_SysEvtlist.get(i).sParameter.length() < 1) {
				if (sParameter.length() < 1) {
					// already here
					return;
				}
			}

		}

		m_SysEvtlist.add(evt);
	}

	static public String GetAccountList() {
		AccountManager accountManager = AccountManager.get(EAUtil
				.GetEAContext());
		Account[] accounts = accountManager.getAccounts();

		StringBuilder sXml = new StringBuilder();
		sXml.append("<Accounts>");

		for (Account account : accounts) {
			sXml.append("<Account>");
			sXml.append("<Name>");
			sXml.append(URLEncoder.encode(account.name));
			sXml.append("</Name>");

			sXml.append("<Type>");
			sXml.append(URLEncoder.encode(account.type));
			sXml.append("</Type>");

			sXml.append("</Account>");
		}
		sXml.append("</Accounts>");

		return sXml.toString();
	}
	
}
