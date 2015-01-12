package mobitnt.util;

import java.io.UnsupportedEncodingException;
import java.net.InetAddress;
import java.net.URLEncoder;
import java.util.Random;

import mobitnt.android.wrapper.FileApi;

import android.content.ContentResolver;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.database.Cursor;

public class EAUtil {
	static Context eaContext;
	static InetAddress m_ClientIP = null;

	public static final int SRV_STATE_WAIT = 0;
	public static final int SRV_STATE_BACKUP_SMS = 1;
	public static final int SRV_STATE_BACKUP_CALL = 2;
	public static final int SRV_STATE_RESOTORE_SMS = 3;
	public static final int SRV_STATE_RESOTORE_CALL = 4;
	static int m_iSrvState = SRV_STATE_WAIT;

	public static Context GetEAContext() {
		return eaContext;
	}

	public static String GetResString(int id) {
		if (eaContext == null) {
			return null;
		}

		return eaContext.getString(id);
	}

	public static void SetEAContext(Context c) {
		eaContext = c;
	}

	public static String CHECK_STRING(String s, String DefValue) {
		if (s == null || s.length() < 1) {
			return DefValue;
		}

		return s;
	}

	static public String GetPEFolderOnSDCard() {
		String sPath = FileApi.getExternalStoragePath();
		if (sPath == null || sPath.length() < 2) {
			return "";
		}

		sPath += "/YaSync";
		FileApi.CreateFolder(sPath);

		return sPath;
	}

	static public ContentResolver GetContentResolver() {
		if (eaContext != null) {
			return eaContext.getContentResolver();
		}
		return null;
	}

	static String GenRandomNumber(int iLen) {
		String s = "";
		for (int i = 0; i < iLen; ++i) {
			int r = new Random().nextInt(10);
			s += String.valueOf(r);
		}

		return s;
	}
	
	static public boolean IsSecurityEnabled(){
		SharedPreferences sp = EAUtil.GetEAContext().getSharedPreferences(
				EADefine.EA_PREF_NAME, 0);
		if (sp == null) {
			return false;
		}

		return sp.getBoolean("EnableSecurityCode", false);
	}
	
	static public void EnableSecurityCode(boolean bEnable){
		SharedPreferences sp = EAUtil.GetEAContext().getSharedPreferences(
				EADefine.EA_PREF_NAME, 0);
		if (sp == null) {
			MobiTNTLog.write("GetSecurityCode:can not open pref");
			return;
		}

		Editor sharedata = sp.edit();
		sharedata.putBoolean("EnableSecurityCode", bEnable);
		sharedata.commit();

		return;
	}
	
	

	static public String GetSecurityCode(boolean bNew) {
		SharedPreferences sp = EAUtil.GetEAContext().getSharedPreferences(
				EADefine.EA_PREF_NAME, 0);
		if (sp == null) {
			MobiTNTLog.write("GetSecurityCode:can not open pref");
			return "";
		}

		if (!bNew) {
			String s = sp.getString("SecurityCode", "");
			if (s.length() == 5) {
				return s;
			}
		}

		String sCode = GenRandomNumber(5);

		Editor sharedata = sp.edit();
		sharedata.putString("SecurityCode", sCode);
		sharedata.commit();

		return sCode;
	}
	
	static public String GetPhoneID() {
		SharedPreferences sp = EAUtil.GetEAContext().getSharedPreferences(
				EADefine.EA_PREF_NAME, 0);
		if (sp == null) {
			MobiTNTLog.write("GetPhoneID: can not open pref");
			return "unknowid";
		}

		String s = sp.getString("PhoneID", "");
		if (s.length() > 0) {
			return s;
		}
		
		String sCode = GenRandomNumber(10);

		Editor sharedata = sp.edit();
		sharedata.putString("PhoneID", sCode);
		sharedata.commit();

		return sCode;
	}


	static public String EncodeItem(String s) {
		String sVal = EAUtil.CHECK_STRING(s, " ");

		try {
			sVal = URLEncoder.encode(sVal, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		return sVal;
	}

	static public String Cursor2Xml(Cursor cur) {
		if (cur == null) {
			return "";
		}

		StringBuilder sXml = new StringBuilder("");
		String[] colNames = cur.getColumnNames();
		for (int i = 0; i < colNames.length; ++i) {
			String sCol = colNames[i];
			sCol = sCol.trim();
			if (sCol.length() < 1){
				continue;
			}
			
			if (sCol.replaceAll("[a-z]*[A-Z]*\\d*-*_*\\s*", "")
					.length() != 0) {
				continue;
			}
			
			if (sCol.contains("sync_") || sCol.contains("_sync")) {
				continue;
			}
			
			String sVal = "";
			try{
				sVal = cur.getString(i);
			}catch (Exception e){
				sVal = "";
			}
			
			if (sVal == null || sVal.length() < 1) {
				continue;
			}

			sVal = sVal.trim();
			if (sVal.length() > 0) {
				sXml.append("<" + sCol + ">");
				sXml.append(EAUtil.EncodeItem(cur.getString(i)));
				sXml.append("</" + sCol + ">");
			}
		}

		return sXml.toString();
	}
}
