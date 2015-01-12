#include "stdafx.h"
#include "DllHelper.h"


void WriteLog(TCHAR *szLogInfo)
{
	if (!szLogInfo || _tcslen(szLogInfo) < 1)
	{
		return;
	}

	HWND hWnd = FindWindow(NULL,YASYNC_MAIN_WIN_NAME);
	if (hWnd == NULL)
	{
		return;
	}

	COPYDATASTRUCT cpd;
   	cpd.dwData = PE_CMD_LOG;
   	cpd.cbData = (DWORD)_tcslen(szLogInfo)*sizeof(TCHAR);
   	cpd.lpData = szLogInfo;
 
   	SendMessage(hWnd,WM_COPYDATA,NULL,(LPARAM)&cpd);
	
}


int PEIsRegistered()
{
	HKEY  hkey;
	DWORD dwType;
	DWORD dwLen;
	DWORD dwIsRegistered;
	
	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
	{
		return PE_RET_FAIL;
	}
	
	dwLen = sizeof(DWORD);
	RegQueryValueEx(hkey, _T("dwIsRegistered"), 0, &dwType, (BYTE*)&dwIsRegistered, &dwLen);
	RegCloseKey(hkey);

	if (dwIsRegistered)
	{
		return PE_RET_OK;
	}
	
	return PE_RET_FAIL;
}


int IsInstanceExist()
{
	HWND hWnd = FindWindow(NULL,YASYNC_MAIN_WIN_NAME);
	if (hWnd == NULL)
	{
		DWORD dwErr = GetLastError();
		StartYaSync();
		return PE_RET_FAIL;
	}
	return PE_RET_OK;
}

CRITICAL_SECTION cs4HttpConnection;
int SendCmd2YaSync(DWORD dwCmd,void *cmdData,int iDataLen)
{
	HWND hWnd = FindWindow(NULL,YASYNC_MAIN_WIN_NAME);
	if (hWnd == NULL)
	{
		StartYaSync();
		return PE_RET_FAIL;
	}

	COPYDATASTRUCT cpd;
    cpd.dwData = dwCmd;
    cpd.cbData = iDataLen;
    cpd.lpData = cmdData;
 
    SendMessage(hWnd,WM_COPYDATA,NULL,(LPARAM)&cpd);

	return PE_RET_OK;
}

int StartYaSync()
{
	HKEY  hkey;
	DWORD dwType;
	DWORD dwLen;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_COMMON_SETTING, 0, KEY_ALL_ACCESS, &hkey) != ERROR_SUCCESS)
	{
		return PE_RET_FAIL;
	}

	TCHAR szAppPath[MAX_PATH];
	dwLen = dwLen = MAX_PATH * sizeof(TCHAR);
	memset(szAppPath,0,dwLen);
	RegQueryValueEx(hkey, _T("szAppPath"), 0, &dwType, (BYTE*)(szAppPath) ,&dwLen);

	if (_tcslen(szAppPath) < 5)
	{
		return PE_RET_FAIL;
	}

	ShellExecute(NULL,_T("open"),szAppPath,_T("startByOutlook"),NULL,SW_SHOW);
		
	return PE_RET_OK;
}



