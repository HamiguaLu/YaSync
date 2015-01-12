package mobitnt.net;

import java.io.IOException;

import mobitnt.android.YaSync.YaSync;
import mobitnt.android.wrapper.SmsApi;
import mobitnt.util.EADefine;
import mobitnt.util.EAUtil;
import mobitnt.util.MobiTNTLog;
import android.annotation.SuppressLint;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.net.wifi.WifiManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.provider.CalendarContract;
import android.provider.ContactsContract;
import android.util.Log;

public class YaSyncService extends Service {
	SrvSock m_SrvSock;

	static YaSyncService myService = null;

	public YaSyncService() {
		Log.i("Service", "PE service started");
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return mBinder;
	}

	@Override
	public boolean onUnbind(Intent intent) {
		super.onUnbind(intent);
		MobiTNTLog.write("service onUnbind called");
		return false;
	}

	public int onStartCommand(Intent intent, int flags, int startId) {
		MobiTNTLog.write("service create called");
		return START_STICKY;
	}

	private static BroadcastReceiver mBroadcastReceiver = new YaSyncReceiver();
	private static Handler mHandler = new Handler() {
		public void handleMessage(Message msg) {
			Log.i("PES", "---mHanlder----");
		}
	};
	
	@SuppressLint("NewApi")
	void InstallReceiver() {
		IntentFilter filter = new IntentFilter();

		filter.addAction(EADefine.INTENT_ACTION_PHONE_STATE);
		filter.addAction(Intent.ACTION_BATTERY_CHANGED);
		filter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
		filter.addAction(WifiManager.WIFI_STATE_CHANGED_ACTION);
		filter.addAction(WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION);

		registerReceiver(mBroadcastReceiver, filter);

		SmsContentObserver smsChangeObserver = new SmsContentObserver(
				EAUtil.GetEAContext(), mHandler);
		getContentResolver().registerContentObserver(
				Uri.parse("content://sms/"), true, smsChangeObserver);

		ContactContentObserver contactChangeObserver = new ContactContentObserver(
				EAUtil.GetEAContext(), mHandler);
		getContentResolver().registerContentObserver(
				ContactsContract.Contacts.CONTENT_URI, true,
				contactChangeObserver);

		if (android.os.Build.VERSION.SDK_INT > 14) {
			CalendarContentObserver calendarChangeObserver = new CalendarContentObserver(
					EAUtil.GetEAContext(), mHandler);
			getContentResolver().registerContentObserver(
					CalendarContract.Events.CONTENT_URI, true,
					calendarChangeObserver);
		}
	}

	@Override
	public void onCreate() {
		super.onCreate();

		myService = this;
		EAUtil.SetEAContext(this.getBaseContext());

		MobiTNTLog.write("service create called");

		InstallReceiver();

		SmsApi.ReScheduleSms();

		try {
			m_SrvSock = new SrvSock();
			m_SrvSock.htmlData = getResources().getAssets();
			MobiTNTLog.write("Waiting for request...");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			MobiTNTLog.write("service create call failed:" + e.toString());
		}
	}

	@Override
	public void onDestroy() {
		MobiTNTLog.write("service destroy called 1");
		super.onDestroy();
		Log.i("Service", "onDestroy");
		MobiTNTLog.write("service destroy called");
	}

	private final iPESrvCtrl.Stub mBinder = new iPESrvCtrl.Stub() {
		public int OP(int iOpCode) {
			return 0;
		}
	};

	static public void ShowInfoOnUI(String sInfo) {
		if (myService != null) {
			Intent intent = new Intent(YaSync.SHOW_SRV_INFO);
			intent.putExtra("INFO", sInfo);
			myService.sendBroadcast(intent);
		}
	}

}
