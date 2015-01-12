package mobitnt.net;

import mobitnt.android.wrapper.SysApi;
import mobitnt.util.EADefine;
import mobitnt.util.MobiTNTLog;
import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;

public class ContactContentObserver extends ContentObserver {
	public ContactContentObserver(Context context, Handler handler) {
		super(handler);
	}
	
	static public int m_iChanged = 0;

	@Override
	public void onChange(boolean selfChange) {
		MobiTNTLog.write("Contact changed");
		super.onChange(selfChange);
			
		SysApi.PushSysEvt(EADefine.SYS_EVT_CONTACT_CHANGED,"");
		
		m_iChanged = 2;
	}

}