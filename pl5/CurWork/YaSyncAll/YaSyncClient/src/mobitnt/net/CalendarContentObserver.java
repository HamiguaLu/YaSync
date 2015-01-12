package mobitnt.net;

import mobitnt.android.wrapper.SysApi;
import mobitnt.util.EADefine;
import mobitnt.util.MobiTNTLog;
import android.content.Context;
import android.database.ContentObserver;
import android.os.Handler;

public class CalendarContentObserver extends ContentObserver {
	public CalendarContentObserver(Context context, Handler handler) {
		super(handler);
	}

	@Override
	public void onChange(boolean selfChange) {
		MobiTNTLog.write("Calendar changed");
		super.onChange(selfChange);
			
		SysApi.PushSysEvt(EADefine.SYS_EVT_CALENDAR_CHANGED,"");
	}

}