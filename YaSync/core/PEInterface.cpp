// PocketExport.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PEInterface.h"
#include "AppTools.h"
#include "DeviceAgent.h"
#include "PECore.h"

#include "SettingHelper.h"
#include "TrayHelper.h"

#include "ExceptionHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



static HWND g_hUI;

void SetMainUIHandle(HWND hWnd)
{
	g_hUI = hWnd;
}

HWND GetMainUIHandle()
{
	return g_hUI;
}

extern void SendLogFile();
long WINAPI SysExceptionHandler(EXCEPTION_POINTERS* lpExceptionInfo)
{
	PEWriteLog(_T("----------Exception Handled----------------"));
	TCHAR szVer[255];
	_stprintf(szVer,_T("YaSyncVer:%d build at %s %s"),APP_VER,_T(__DATE__),_T(__TIME__));
	PEWriteLog(szVer);

	RecordExceptionInfo(lpExceptionInfo);

	AfxMessageBox(_T("Oops,seems YaSync is crashed"),MB_OK|MB_ICONSTOP);

#ifndef _DEBUG
	SendLogFile();
#endif
	return EXCEPTION_EXECUTE_HANDLER;
}

#include <locale.h>
DWORD PE_Init()
{
	SetAppInstance(AfxGetInstanceHandle());

	AfxSocketInit();

	InitLog();

	SetUnhandledExceptionFilter(SysExceptionHandler);

	PECore::StartMiniHttpd();

	CSettingHelper::Init();

	PEWriteLog(_T("----------PE_Init----------------"));

	WORD languageID = LOWORD(GetKeyboardLayout(0));
	TCHAR szLCData[10];
	GetLocaleInfo(MAKELCID(languageID, SORT_DEFAULT), LOCALE_IDEFAULTANSICODEPAGE,szLCData, _countof(szLCData));
	
	TCHAR szLang[125];
	_itot(languageID,szLang,10);
	PEWriteLog(_T("languageID is :"));
	PEWriteLog(szLang);
	PEWriteLog(_T("CodePage is :"));
	PEWriteLog(szLCData);
	
	PrintSysInfo();

	DeviceAgent::SetHostInfo(_T("127.0.0.1"));

	return PE_RET_OK;
}


extern int g_iExitNow;
extern int g_iCanSync;

DWORD PEStartSvr()
{
	g_iExitNow = 0;
	g_iCanSync = 1;

	PECore::StartSvr();

	PECore::PostMessage(WM_START_SRV,0,0);

	PEWriteLog(_T("Start to Sync now"));
	return PE_RET_OK;
}


DWORD PEStopSvr()
{
	g_iExitNow = 1;
	g_iCanSync = 0;

	ShowTrayInfo(_T("YaSync will exit now, disconnect with device..."));
	PEWriteLog(_T("YaSync will exit now"));

	HANDLE hExitEvt = CreateEvent(0,FALSE,FALSE,0);
	PECore::PostMessage(WM_STOP_SRV,(WPARAM)hExitEvt,0);

	PEWriteLog(_T("wait sync thread..."));
	WaitForSingleObject(hExitEvt,1000 * 60 * 3);//wait 3 minutes before shutdown

	PECore::StopMiniHttpd();

	PEWriteLog(_T("wait sync thread done"));

	RemoveTray();

	PEWriteLog(_T("send WM_CLOSE"));

	PEWriteLog(_T("---------------------PEStopSvr:exit now--------------------"));

	return PE_RET_OK;
}


DWORD PEIsRegistered()
{
	PE_COMMON_SETTING *s = (PE_COMMON_SETTING*)calloc(1,sizeof(PE_COMMON_SETTING));
	CSettingHelper::GetCommonSetting(s);

	if (s->dwIsRegistered)
	{
		if (_tcslen(s->szSN) > 10)
		{
			//DeviceAgent::ReportAppState(s->szSN);

			free(s);
			return PE_RET_OK;
		}
	}

	free(s);
	return PE_RET_FAIL;
}

