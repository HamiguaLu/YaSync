package mobitnt.util;


import android.util.Log;


public class MobiTNTLog {
	static public boolean m_bEnableLog = false;

	static public void write(String sInfo) {
		if (!m_bEnableLog) {
			return;
		}

		Log.e("MobiTNT",sInfo);
	}


}
