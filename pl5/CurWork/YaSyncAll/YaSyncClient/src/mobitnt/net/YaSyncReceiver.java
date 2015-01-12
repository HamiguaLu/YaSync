package mobitnt.net;

import mobitnt.android.wrapper.NetApi;
import mobitnt.android.wrapper.SysApi;
import mobitnt.util.EADefine;
import mobitnt.util.MobiTNTLog;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.net.NetworkInfo;
import android.net.wifi.WifiManager;

public class YaSyncReceiver extends BroadcastReceiver {
	//static final String ACTION = "android.intent.action.BOOT_COMPLETED";

	Context my_context;
	
	public void ShowIPOnUI(){
		String sIP = NetApi.getLocalIpAddress();
		YaSyncService.ShowInfoOnUI("Device IP:" + sIP);
	}

	@Override
	public void onReceive(Context context, Intent intent) {
		my_context = context;

		String action = intent.getAction();
		MobiTNTLog.write("Action:" + action);
		if (action.equals(Intent.ACTION_BATTERY_CHANGED)) {
			int level = intent.getIntExtra("level", 0);
			SysApi.m_iBatteryLevel = level;
			SysApi.PushSysEvt(EADefine.SYS_EVT_BATTERY_LEVEL_CHANGED, String.valueOf(level));
			return;
		}

		if (action.equals(EADefine.INTENT_ACTION_PHONE_STATE)) {
			MobiTNTLog.write("new call");
			SysApi.PushSysEvt(EADefine.SYS_EVT_CALL_LOG_CHANGED,"");
			return;
		}

		if (action.equals(WifiManager.SUPPLICANT_CONNECTION_CHANGE_ACTION)) {
			boolean bConnected = intent.getBooleanExtra(
					WifiManager.EXTRA_SUPPLICANT_CONNECTED, false);
			if (!bConnected) {
				MobiTNTLog.write("SUPPLICANT_CONNECTION_CHANGE_ACTION:wifi not connected");
				return;
			}
			
			
			MobiTNTLog.write("SUPPLICANT_CONNECTION_CHANGE_ACTION:wifi is connected");
			NetApi.StartReportTimer();
			return;
		}

		if (action.equals(WifiManager.WIFI_STATE_CHANGED_ACTION)) {
			int wifiState = intent.getIntExtra(WifiManager.EXTRA_WIFI_STATE,
					WifiManager.WIFI_STATE_UNKNOWN);
			
			ShowIPOnUI();
			
			if (wifiState != WifiManager.WIFI_STATE_ENABLED) {
				MobiTNTLog.write("WIFI_STATE_CHANGED_ACTION:wifi not enabled yet");
				return;
			}

			MobiTNTLog.write("WIFI_STATE_CHANGED_ACTION:wifi is enabled");
			NetApi.StartReportTimer();
			return;
		}
		
		if (action.equals(WifiManager.NETWORK_STATE_CHANGED_ACTION)){
		  NetworkInfo info = (NetworkInfo)intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
		  if (info == null){
			  return;
		  }
		  
		  if (info.getState().equals(NetworkInfo.State.CONNECTED))
		  {
			  MobiTNTLog.write("NETWORK_STATE_CHANGED_ACTION:wifi is enabled");
			  NetApi.StartReportTimer();
		  }
		  
		  ShowIPOnUI();
		  return;
		}
		
		/*if (action.equals("android.intent.action.BOOT_COMPLETED")) {
			final Intent srvintent = new Intent();
			srvintent.setAction("mobitnt.net.YASYNCSERVICE");
			try {
				ComponentName srvName = context.startService(srvintent);
				if (srvName == null) {
					//
					MobiTNTLog.write("Boot:Start service failed");
				} else {
					String sName = srvName.getClassName();
					MobiTNTLog.write("Boot:service started  " + sName);
				}
			} catch (Exception ex) {
				ex.printStackTrace();
				MobiTNTLog.write("Boot:Start service failed");
			}
			
			return;
		}*/

		return;
	}
}
