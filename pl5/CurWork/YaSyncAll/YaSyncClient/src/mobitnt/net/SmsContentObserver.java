package mobitnt.net;

import mobitnt.android.wrapper.SysApi;
import mobitnt.util.EADefine;
import mobitnt.util.MobiTNTLog;
import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;

public class SmsContentObserver extends ContentObserver {
	public SmsContentObserver(Context context, Handler handler) {
		super(handler);
	}

	@Override
	public void onChange(boolean selfChange) {
		MobiTNTLog.write("SmsReceived");
		super.onChange(selfChange);
			
		SysApi.PushSysEvt(EADefine.SYS_EVT_SMS_CHANGED,"");
	}

}