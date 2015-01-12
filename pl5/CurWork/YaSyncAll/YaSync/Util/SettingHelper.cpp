
#include "stdafx.h"
#include "AppTools.h"
#include "SettingHelper.h"

#include "DeviceAgent.h"




CRITICAL_SECTION cs4Reg;
static DWORD dwShowQuickStart = 1;

void CSettingHelper::Init(void)
{
	InitializeCriticalSectionAndSpinCount(&cs4Reg,0x80000400);
}


CSettingHelper::CSettingHelper(void)
{

}

CSettingHelper::~CSettingHelper(void)
{

}

DWORD dwDevConnState = 0;

void CSettingHelper::SetDeviceConnState()
{
	if (dwDevConnState == 1)
	{
		return;
	}
	
	EnterCriticalSection(&cs4Reg);

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		RegCreateKey(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, &hkey);
	}

	DWORD dwDeviceConnected = 1;
	RegSetValueEx(hkey, _T("dwDeviceConnected"),	0,	REG_DWORD,	(BYTE *)&dwDeviceConnected,sizeof(DWORD));

	RegCloseKey(hkey);

	DeviceAgent::ReportAppState(_T("DevConn"));

	LeaveCriticalSection(&cs4Reg);
}


int CSettingHelper::GetDeviceConnState()
{
	EnterCriticalSection(&cs4Reg);

	if (dwShowQuickStart == 0)
	{
		LeaveCriticalSection(&cs4Reg);
		return PE_RET_OK;
	}

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		LeaveCriticalSection(&cs4Reg);
		return NULL;
	}

	DWORD dwType = 0;
	DWORD dwLen = 255 * sizeof(TCHAR);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwDeviceConnected"), 0, &dwType, (BYTE*)&dwDevConnState, &dwLen);

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);

	if (dwDevConnState == 1)
	{
		return PE_RET_OK;
	}

	return PE_RET_FAIL;
	
}

void CSettingHelper::SetLastPhoneName(TCHAR *szPhoneName)
{
	EnterCriticalSection(&cs4Reg);

	if (!szPhoneName || _tcslen(szPhoneName) < 1)
	{
		return;
	}

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		RegCreateKey(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, &hkey);
	}

	RegSetValueEx(hkey, _T("szLastPhoneName"),0,REG_SZ,(BYTE *)(szPhoneName),(DWORD)_tcslen(szPhoneName)*sizeof(TCHAR));

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);
}


int CSettingHelper::GetLastPhoneName(TCHAR *szPhoneName)
{
	EnterCriticalSection(&cs4Reg);

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		LeaveCriticalSection(&cs4Reg);
		return NULL;
	}

	DWORD dwType = 0;
	DWORD dwLen = 255 * sizeof(TCHAR);

	RegQueryValueEx(hkey, _T("szLastPhoneName"), 0, &dwType, (BYTE*)(szPhoneName) ,&dwLen);

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);

	return PE_RET_OK;
}


void CSettingHelper::SetAutoStartApp(int iEnable)
{
	EnterCriticalSection(&cs4Reg);

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_AUTO_START, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		return;
	}

	TCHAR szPath[255];
	memset(szPath,0,255*sizeof(TCHAR));
	if (iEnable)
	{
		GetModuleFileName(NULL, szPath, 255);
		_tcscat(szPath,_T(" autoStart"));
	}

	RegSetValueEx(hkey, _T("YaSync"),0,REG_SZ,(BYTE *)(szPath),(DWORD)_tcslen(szPath)*sizeof(TCHAR));

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);
}


void CSettingHelper::SetCommonSetting(PE_COMMON_SETTING *setting)
{
	EnterCriticalSection(&cs4Reg);

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		RegCreateKey(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, &hkey);
	}

	TCHAR szAppPath[MAX_PATH];
	GetModuleFileName(GetAppInstance(), szAppPath, MAX_PATH);

	RegSetValueEx(hkey, _T("szAppPath"),			0,	REG_SZ,	   (BYTE *)(szAppPath),							(DWORD)_tcslen(szAppPath)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwVer"),				0,	REG_DWORD, (BYTE *)&(setting->dwVer),					sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwIsRegistered"),		0,	REG_DWORD, (BYTE *)&setting->dwIsRegistered,			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwTrialStartTime"),		0,	REG_DWORD, (BYTE *)&setting->dwTrialStartTime,			sizeof(DWORD));
	RegSetValueEx(hkey, _T("szSN"),					0,	REG_SZ,	   (BYTE *)(setting->szSN),						(DWORD)_tcslen(setting->szSN)*sizeof(TCHAR));
	RegSetValueEx(hkey, _T("szPst"),				0,	REG_SZ,	   (BYTE *)(setting->szPst),					(DWORD)_tcslen(setting->szPst)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwAutoSync"),			0,	REG_DWORD,	(BYTE *)&(setting->dwAutoSync),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwAutoStart"),			0,	REG_DWORD,	(BYTE *)&(setting->dwAutoStart),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwExitAfterSync"),		0,	REG_DWORD,	(BYTE *)&(setting->dwExitAfterSync),		sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwUseDefaultPst"),		0,	REG_DWORD,	(BYTE *)&(setting->dwUseDefaultPst),		sizeof(DWORD));

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);
}


void CSettingHelper::ResetCommonSetting(PE_COMMON_SETTING *setting)
{
	setting->dwVer = APP_VER;
	setting->dwAutoStart = 1;
	setting->dwAutoSync = 1;
	setting->dwExitAfterSync = 0;
	setting->dwUseDefaultPst = 1;

	SetAutoStartApp(1);

	int iVer = 0;
	GetOutlookVersion(iVer);
	if (iVer >= 14)
	{
		RegDll(_T("AndroidAddIn.dll"),0);
	}
	else
	{
		RegDll(_T("SMSOutLookAddin.dll"),0);
	}

	SetCommonSetting(setting);
}



int CSettingHelper::GetCommonSetting(PE_COMMON_SETTING *setting)
{
	EnterCriticalSection(&cs4Reg);

	HKEY  hkey;
	DWORD dwType;
	DWORD dwLen;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
	{
		time_t ltime;
		time( &ltime );

		memset(setting,0,sizeof(PE_COMMON_SETTING));
		setting->dwVer = APP_VER;
		setting->dwTrialStartTime = (DWORD)ltime;

		ResetCommonSetting(setting);
		CString sInfo;
		sInfo.Format(_T("http://a.mobitnt.com/install.php?P=YaSync%d"),APP_VER);
		ShellExecute(NULL,_T("open"),sInfo,NULL,NULL,SW_SHOW);
		DeviceAgent::ReportAppState(_T("First Start,no reg info found"));

		dwShowQuickStart = 0;
		LeaveCriticalSection(&cs4Reg);

		return PE_RET_FAIL;
	}

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwVer"), 0, &dwType, (BYTE*)&(setting->dwVer), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwIsRegistered"), 0, &dwType, (BYTE*)&(setting->dwIsRegistered), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwTrialStartTime"), 0, &dwType, (BYTE*)&(setting->dwTrialStartTime), &dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szSN"), 0, &dwType, (BYTE*)(setting->szSN) ,&dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szPst"), 0, &dwType, (BYTE*)(setting->szPst) ,&dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwAutoSync"), 0, &dwType, (BYTE*)&(setting->dwAutoSync), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwAutoStart"), 0, &dwType, (BYTE*)&(setting->dwAutoStart), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwExitAfterSync"), 0, &dwType, (BYTE*)&(setting->dwExitAfterSync), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwUseDefaultPst"), 0, &dwType, (BYTE*)&(setting->dwUseDefaultPst), &dwLen);

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);

	if (setting->dwVer != APP_VER)
	{
		setting->dwVer = APP_VER;
		SetCommonSetting(setting);

		if (setting->dwAutoStart)
		{
			SetAutoStartApp(1);
		}

		int iVer = 0;
		GetOutlookVersion(iVer);
		if (iVer >= 14)
		{
			RegDll(_T("AndroidAddIn.dll"),0);
			//RegDll(_T("SMSOutLookAddin.dll"),1);
		}
		else
		{
			RegDll(_T("SMSOutLookAddin.dll"),0);
		}

		CString sInfo;
		sInfo.Format(_T("http://a.mobitnt.com/install.php?P=YaSync%d"),APP_VER);
		ShellExecute(NULL,_T("open"),sInfo,NULL,NULL,SW_SHOW);

		dwShowQuickStart = 0;
		DeviceAgent::ReportAppState(_T("First Start:old version found"));
	}

	return PE_RET_OK;
}

void CSettingHelper::SetPhoneSetting(PE_PHONE_SETTING *setting)
{
	EnterCriticalSection(&cs4Reg);

	TCHAR szkey[255];
	_stprintf(szkey,HKEY_PE_PHONE_SETTING,setting->szPhoneID);

	if (setting->dwSyncImage || setting->dwSyncAudio || setting->dwSyncVideo)
	{
		if ( !_tcslen(setting->szAudioFolderName) || !_tcslen(setting->szImageFolderName) || !_tcslen(setting->szVideoFolderName))
		{
			ResetMediaSyncSetting(setting);
		}
	}

	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, szkey, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		RegCreateKey(HKEY_CURRENT_USER, szkey, &hkey);
	}

	RegSetValueEx(hkey, _T("dwSyncContact"),				0,	REG_DWORD,	(BYTE *)&(setting->dwSyncContact),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwContactSyncType"),				0,	REG_DWORD,	(BYTE *)&(setting->dwContactSyncType),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwEnableContactAccount"),				0,	REG_DWORD,	(BYTE *)&(setting->dwEnableContactAccount),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("szContactAccounts"),		0,	REG_SZ,	   (BYTE *)(setting->szContactAccounts),		(DWORD)_tcslen(setting->szContactAccounts)*sizeof(TCHAR));
	RegSetValueEx(hkey, _T("szContactFolderNames"),		0,	REG_SZ,	   (BYTE *)(setting->szContactFolderNames),		(DWORD)_tcslen(setting->szContactFolderNames)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwSyncCalendar"),				0,	REG_DWORD,	(BYTE *)&(setting->dwSyncCalendar),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwCalSyncType"),				0,	REG_DWORD,	(BYTE *)&(setting->dwCalSyncType),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("ftCalendarSyncAfterH"),				0,	REG_DWORD,	(BYTE *)&(setting->ftCalendarSyncAfter.dwHighDateTime),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("ftCalendarSyncAfterL"),				0,	REG_DWORD,	(BYTE *)&(setting->ftCalendarSyncAfter.dwLowDateTime),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwOnlySyncSelectedCal"),				0,	REG_DWORD,	(BYTE *)&(setting->dwOnlySyncSelectedCal),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwOnlySyncCalAfterTime"),				0,	REG_DWORD,	(BYTE *)&(setting->dwOnlySyncCalAfterTime),			sizeof(DWORD));
	RegSetValueEx(hkey, _T("szCalendarIDs"),				0,	REG_SZ,	   (BYTE *)(setting->szCalendarIDs),			(DWORD)_tcslen(setting->szCalendarIDs)*sizeof(TCHAR));
	RegSetValueEx(hkey, _T("szCalendarFolderNames"),		0,	REG_SZ,    (BYTE *)(setting->szCalendarFolderNames),	(DWORD)_tcslen(setting->szCalendarFolderNames)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwSyncSMS"),					0,	REG_DWORD,	(BYTE *)&(setting->dwSyncSMS),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwSetReadFlag"),				0,	REG_DWORD,	(BYTE *)&(setting->dwSetReadFlag),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwShowSMSReprot"),				0,	REG_DWORD,	(BYTE *)&(setting->dwShowSMSReprot),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwSyncMMS"),					0,	REG_DWORD,	(BYTE *)&(setting->dwSyncMMS),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("dwSyncCall"),					0,	REG_DWORD,	(BYTE *)&(setting->dwSyncCall),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("szSMSFolderName"),			0,	REG_SZ,    (BYTE *)(setting->szSMSFolderName),			(DWORD)_tcslen(setting->szSMSFolderName)*sizeof(TCHAR));
	RegSetValueEx(hkey, _T("szCallFolderName"), 		0,	REG_SZ,    (BYTE *)(setting->szCallFolderName), 		(DWORD)_tcslen(setting->szCallFolderName)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwSyncImage"),					0,	REG_DWORD,	(BYTE *)&(setting->dwSyncImage),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("szImageFolderName"),			0,	REG_SZ,    (BYTE *)(setting->szImageFolderName),			(DWORD)_tcslen(setting->szImageFolderName)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwSyncAudio"),					0,	REG_DWORD,	(BYTE *)&(setting->dwSyncAudio),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("szAudioFolderName"),			0,	REG_SZ,    (BYTE *)(setting->szAudioFolderName),			(DWORD)_tcslen(setting->szAudioFolderName)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("dwSyncVideo"),					0,	REG_DWORD,	(BYTE *)&(setting->dwSyncVideo),				sizeof(DWORD));
	RegSetValueEx(hkey, _T("szVideoFolderName"),			0,	REG_SZ,    (BYTE *)(setting->szVideoFolderName),			(DWORD)_tcslen(setting->szVideoFolderName)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("szDevIP"),					0,	REG_SZ,		(BYTE *)(setting->szDevIP),					(DWORD)_tcslen(setting->szDevIP)*sizeof(TCHAR));

	RegSetValueEx(hkey, _T("szPhoneName"),				0,	REG_SZ,	   (BYTE *)(setting->szPhoneName),				(DWORD)_tcslen(setting->szPhoneName)*sizeof(TCHAR));
	RegSetValueEx(hkey, _T("szPhoneID"),					0,	REG_SZ,	   (BYTE *)(setting->szPhoneID),				(DWORD)_tcslen(setting->szPhoneID)*sizeof(TCHAR));
	RegSetValueEx(hkey, _T("szSecurityCode"),				0,	REG_SZ,	   (BYTE *)(setting->szSecurityCode),			(DWORD)_tcslen(setting->szSecurityCode)*sizeof(TCHAR));

	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);
}


int CSettingHelper::ResetMediaSyncSetting(PE_PHONE_SETTING *setting)
{
	setting->dwSyncImage = 1;
	setting->dwSyncVideo = 1;
	setting->dwSyncAudio = 1;

	TCHAR szPath[255];
	memset(szPath,0,255 * sizeof(TCHAR));
	GetDocPath(szPath);
	_tcscat(szPath,_T("\\"));
	_tcscat(szPath,setting->szPhoneName);
	
	_tcscpy(setting->szImageFolderName,szPath);
	_tcscat(setting->szImageFolderName,_T("Photo\\"));

	_tcscpy(setting->szVideoFolderName,szPath);
	_tcscat(setting->szVideoFolderName,_T("Video\\"));

	_tcscpy(setting->szAudioFolderName,szPath);
	_tcscat(setting->szAudioFolderName,_T("Audio\\"));

	return PE_RET_OK;
}



int CSettingHelper::ResetPhoneSetting(PE_PHONE_SETTING *setting)
{
	setting->dwSyncSMS = 1;
	setting->dwSyncMMS = 1;
	setting->dwSetReadFlag = 1;
	setting->dwSyncCall = 1;
	setting->dwSyncContact = 1;
	setting->dwSyncMMS = 1;
	setting->dwSyncCalendar = 1;
	setting->dwShowSMSReprot = 1;
	setting->dwOnlySyncSelectedCal = 0;
	
	_tcscpy(setting->szContactFolderNames,_T(""));
	_tcscpy(setting->szCalendarFolderNames,_T(""));

	ResetMediaSyncSetting(setting);

	SetPhoneSetting(setting);

	return PE_RET_OK;
}

int CSettingHelper::GetPhoneSetting(PE_PHONE_SETTING *setting)
{
	EnterCriticalSection(&cs4Reg);

	HKEY hkey;
	DWORD dwType;
	DWORD dwLen;
	int iRet = 0;

	int iNeedFree = 0;
	TCHAR szPhoneKey[255];
	_stprintf(szPhoneKey,HKEY_PE_PHONE_SETTING,setting->szPhoneID);
	iRet = RegOpenKeyEx(HKEY_CURRENT_USER, szPhoneKey, 0, KEY_ALL_ACCESS, &hkey);
	if (iRet != ERROR_SUCCESS)
	{
		ResetPhoneSetting(setting);
		LeaveCriticalSection(&cs4Reg);

		return PE_RET_OK;
	}

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncContact"), 0, &dwType, (BYTE*)&(setting->dwSyncContact), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwContactSyncType"), 0, &dwType, (BYTE*)&(setting->dwContactSyncType), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwEnableContactAccount"), 0, &dwType, (BYTE*)&(setting->dwEnableContactAccount), &dwLen);

	dwLen = 1024 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szContactAccounts"), 0, &dwType, (BYTE*)(setting->szContactAccounts) ,&dwLen);

	dwLen = 1024 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szContactFolderNames"), 0, &dwType, (BYTE*)(setting->szContactFolderNames) ,&dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncCalendar"), 0, &dwType, (BYTE*)&(setting->dwSyncCalendar), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwCalSyncType"), 0, &dwType, (BYTE*)&(setting->dwCalSyncType), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwOnlySyncSelectedCal"), 0, &dwType, (BYTE*)&(setting->dwOnlySyncSelectedCal), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwOnlySyncCalAfterTime"), 0, &dwType, (BYTE*)&(setting->dwOnlySyncCalAfterTime), &dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szCalendarIDs"), 0, &dwType, (BYTE*)(setting->szCalendarIDs) ,&dwLen);

	dwLen = 1024 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szCalendarFolderNames"), 0, &dwType, (BYTE*)(setting->szCalendarFolderNames) ,&dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("ftCalendarSyncAfterH"), 0, &dwType, (BYTE*)&(setting->ftCalendarSyncAfter.dwHighDateTime), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("ftCalendarSyncAfterL"), 0, &dwType, (BYTE*)&(setting->ftCalendarSyncAfter.dwLowDateTime), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncSMS"), 0, &dwType, (BYTE*)&(setting->dwSyncSMS), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSetReadFlag"), 0, &dwType, (BYTE*)&(setting->dwSetReadFlag), &dwLen);

	dwLen = sizeof(DWORD);
	setting->dwShowSMSReprot = 1;
	RegQueryValueEx(hkey, _T("dwShowSMSReprot"), 0, &dwType, (BYTE*)&(setting->dwShowSMSReprot), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncMMS"), 0, &dwType, (BYTE*)&(setting->dwSyncMMS), &dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncCall"), 0, &dwType, (BYTE*)&(setting->dwSyncCall), &dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szDevIP"), 0, &dwType, (BYTE*)(setting->szDevIP) ,&dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szPhoneName"), 0, &dwType, (BYTE*)(setting->szPhoneName) ,&dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szPhoneID"), 0, &dwType, (BYTE*)(setting->szPhoneID) ,&dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szSecurityCode"), 0, &dwType, (BYTE*)(setting->szSecurityCode) ,&dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szSMSFolderName"), 0, &dwType, (BYTE*)(setting->szSMSFolderName) ,&dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szCallFolderName"), 0, &dwType, (BYTE*)(setting->szCallFolderName) ,&dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncImage"), 0, &dwType, (BYTE*)&(setting->dwSyncImage), &dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szImageFolderName"), 0, &dwType, (BYTE*)(setting->szImageFolderName) ,&dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncVideo"), 0, &dwType, (BYTE*)&(setting->dwSyncVideo), &dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szVideoFolderName"), 0, &dwType, (BYTE*)(setting->szVideoFolderName) ,&dwLen);

	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwSyncAudio"), 0, &dwType, (BYTE*)&(setting->dwSyncAudio), &dwLen);

	dwLen = 255 * sizeof(TCHAR);
	RegQueryValueEx(hkey, _T("szAudioFolderName"), 0, &dwType, (BYTE*)(setting->szAudioFolderName) ,&dwLen);


	RegCloseKey(hkey);

	LeaveCriticalSection(&cs4Reg);

	return PE_RET_OK;
}

int CSettingHelper::RemovePhoneInfo(TCHAR *szPhoneID)
{
	EnterCriticalSection(&cs4Reg);

	TCHAR szPhoneKey[255];
	_stprintf(szPhoneKey,HKEY_PE_PHONE_SETTING,szPhoneID);
	SHDeleteKey(HKEY_CURRENT_USER,szPhoneKey);

	LeaveCriticalSection(&cs4Reg);

	return PE_RET_OK;
}

list<PE_PHONE_SETTING*> CSettingHelper::GetPhoneInfoList()
{ 

	EnterCriticalSection(&cs4Reg);

	HKEY	 hKey;
	TCHAR    achKey[MAX_KEY_LENGTH];   // buffer for subkey name
	DWORD    cbName;                   // size of name string 
	TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
	DWORD    cchClassName = MAX_PATH;  // size of class string 
	DWORD    cSubKeys=0;               // number of subkeys 
	DWORD    cbMaxSubKey;              // longest subkey size 
	DWORD    cchMaxClass;              // longest class string 
	DWORD    cValues;              // number of values for key 
	DWORD    cchMaxValue;          // longest value name 
	DWORD    cbMaxValueData;       // longest value data 
	DWORD    cbSecurityDescriptor; // size of security descriptor 
	FILETIME ftLastWriteTime;      // last write time 

	DWORD i, retCode;

	DWORD cchValue = MAX_VALUE_NAME;
	list<PE_PHONE_SETTING*> phoneList;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_ROOT,0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
	{
		LeaveCriticalSection(&cs4Reg);
		return phoneList;
	}

	// Get the class name and the value count. 
	retCode = RegQueryInfoKey(
		hKey,                    // key handle 
		achClass,                // buffer for class name 
		&cchClassName,           // size of class string 
		NULL,                    // reserved 
		&cSubKeys,               // number of subkeys 
		&cbMaxSubKey,            // longest subkey size 
		&cchMaxClass,            // longest class string 
		&cValues,                // number of values for this key 
		&cchMaxValue,            // longest value name 
		&cbMaxValueData,         // longest value data 
		&cbSecurityDescriptor,   // security descriptor 
		&ftLastWriteTime);       // last write time 

	if (retCode != ERROR_SUCCESS || cSubKeys <= 0)
	{
		LeaveCriticalSection(&cs4Reg);
		return phoneList;
	}

	// Enumerate the subkeys, until RegEnumKeyEx fails.

	for (i=0; i<cSubKeys; i++) 
	{ 
		cbName = MAX_KEY_LENGTH;
		retCode = RegEnumKeyEx(hKey,i,achKey,&cbName, NULL,NULL,NULL,&ftLastWriteTime); 
		if (retCode != ERROR_SUCCESS) 
		{
			continue;
		}

		//must begin with "DEVICE"
		TCHAR *szBegin = _tcsstr(achKey,_T("DEVICE"));
		if (szBegin != achKey)
		{
			continue;
		}

		szBegin += _tcslen(_T("DEVICE"));

		PE_PHONE_SETTING *pSetting = (PE_PHONE_SETTING*)calloc(1,sizeof(PE_PHONE_SETTING));
		_tcscpy(pSetting->szPhoneID,szBegin);
		if (PE_RET_OK != GetPhoneSetting(pSetting))
		{
			free(pSetting);
			continue;
		}

		phoneList.push_back(pSetting);
	}

	LeaveCriticalSection(&cs4Reg);

	return phoneList;

}



