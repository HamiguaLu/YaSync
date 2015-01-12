package mobitnt.android.wrapper;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

import mobitnt.android.data.*;
import mobitnt.util.EAUtil;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;

public class AppApi {
	static ArrayList<AppPackageInfo> m_appList = null;
	static public ArrayList<AppPackageInfo> GetAppList() {
		m_appList = getInstalledApps(true); 
		return m_appList;
	}

	static public ByteArrayOutputStream GetAppIcon(String sAppName) {
		if (m_appList == null || m_appList.size() < 1) {
			return null;
		}

		for (int i = 0; i < m_appList.size(); ++i) {
			AppPackageInfo app = m_appList.get(i);
			if (app != null && app.appname.equals(sAppName)) {
				return app.iconStream;
			}
		}

		return null;
	}

	
	static ArrayList<AppPackageInfo> getInstalledApps(boolean bOnlyOne) {
		ArrayList<AppPackageInfo> res = new ArrayList<AppPackageInfo>();
		List<PackageInfo> packs = EAUtil.GetEAContext().getPackageManager()
				.getInstalledPackages(PackageManager.GET_UNINSTALLED_PACKAGES);

		int iCount = packs.size();
		if (iCount == 0) {
			return null;
		}

		for (int i = 0; i < iCount; i++) {
			PackageInfo p = packs.get(i);

			if ((p.applicationInfo.flags & ApplicationInfo.FLAG_UPDATED_SYSTEM_APP) != 0
					|| (p.applicationInfo.flags & ApplicationInfo.FLAG_SYSTEM) == 0) {
				AppPackageInfo newInfo = new AppPackageInfo();

				newInfo.appname = p.applicationInfo.loadLabel(
						EAUtil.GetEAContext().getPackageManager()).toString();
				newInfo.pname = p.packageName;
				newInfo.versionName = p.versionName;
				newInfo.versionCode = p.versionCode;

				Drawable icon = p.applicationInfo.loadIcon(EAUtil
						.GetEAContext().getPackageManager());

				BitmapDrawable bd = (BitmapDrawable) icon;
				Bitmap bm = bd.getBitmap();

				newInfo.iconStream = new ByteArrayOutputStream();
				bm.compress(Bitmap.CompressFormat.PNG, 100, newInfo.iconStream);

				newInfo.appname = EAUtil
						.CHECK_STRING(newInfo.appname, "noname");
				newInfo.pname = EAUtil.CHECK_STRING(newInfo.pname, "noname");
				newInfo.versionName = EAUtil.CHECK_STRING(newInfo.versionName,
						"0");

				res.add(newInfo);
			}
		}

		return res;
	}
	
	static public String GetApkStorePath(){
		return EAUtil.GetEAContext().getExternalFilesDir(null).getAbsolutePath();
	}
	
	static public void InstallApp(String sApkPath){
		Intent intent = new Intent(Intent.ACTION_VIEW);
		intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		intent.setDataAndType(Uri.parse("file://" + sApkPath),"application/vnd.android.package-archive");
		EAUtil.GetEAContext().startActivity(intent);

	}

	static public void RemoveApp(String sPackageName) {
		Uri packageURI = Uri.fromParts("package", sPackageName, null);
		Intent uninstallIntent = new Intent(Intent.ACTION_DELETE, packageURI);
		uninstallIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
		EAUtil.GetEAContext().startActivity(uninstallIntent);
		return;
	}
}


