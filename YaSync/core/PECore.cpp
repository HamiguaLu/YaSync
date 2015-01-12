#include "stdafx.h"
#include "PEProtocol.h"
#include "AppTools.h"
#include "DeviceAgent.h"
#include "PEInterface.h"

#include "PECore.h"
#include "UrlEscape.h"
#include "SettingHelper.h"
#include "SmsHelper.h"
#include "CallLogHelper.h"
#include "TrayHelper.h"
#include "AdbHelper.h"
#include "ContactsHelper.h"
#include "UIContainer.h"
#include "DeviceBlockHelper.h"
#include "CalendarHelper.h"

int g_iExitNow = 0;
int g_iCanSync = 1;
int g_iInSync = 0;
int g_iInEvtLoop = 0;

extern CRITICAL_SECTION cs4TrayDlg;


DeviceAgent* PECore::m_pDeviceAgent;

DWORD PECore::m_dwThreadID;
UINT_PTR PECore::m_iTimerID;

extern HWND g_hNewSmsWnd;
extern HWND g_hRestoreDlg;
extern HWND g_hHomeUIWnd;

SYNC_STAT g_SyncStat;

BOOL g_bOutlookInstalled = FALSE;

#define BREAK_IF_NO_OUTLOOK   if (g_bOutlookInstalled == FALSE)break;

list<PE_PHONE_SETTING*> PECore::m_PhoneList;
int PECore::Add2PhoneList(PE_PHONE_SETTING* setting)
{
	list<PE_PHONE_SETTING*>::iterator it;
	for (it = m_PhoneList.begin(); it != m_PhoneList.end(); ++it)
	{ 
		PE_PHONE_SETTING *s = (PE_PHONE_SETTING*)*it;
		if (_tcscmp(s->szPhoneID,setting->szPhoneID) == 0)
		{
			//already exist
			return PE_RET_ALREADY_EXIST;
		}
	}

	if (setting->dwSDKVer < 14)
	{
		setting->dwSyncCalendar = 0;
		CSettingHelper::SetPhoneSetting(setting);
	}

	m_PhoneList.push_back(setting);
	return PE_RET_OK;
}


int PECore::GetCurPhoneName(TCHAR *szPhoneName)
{
	PE_PHONE_SETTING *s = PECore::GetCurPhone();
	if (s)
	{
		_tcscpy(szPhoneName,s->szPhoneName);
		return PE_RET_OK;
	}

	PEWriteLog(_T("CurPhone is NULL!use last phone name"));
	return CSettingHelper::GetLastPhoneName(szPhoneName);
}


PE_PHONE_SETTING* PECore::GetNextPhone()
{
	if (m_PhoneList.size() < 1)
	{
		return NULL;
	}

	return (PE_PHONE_SETTING*)(*m_PhoneList.begin());
}

PE_PHONE_SETTING* PECore::FindPhoneFromPhoneList(TCHAR *szPhoneName)
{
	list<PE_PHONE_SETTING*>::iterator it;

	for (it = m_PhoneList.begin(); it != m_PhoneList.end();++it)
	{ 
		PE_PHONE_SETTING *s = (PE_PHONE_SETTING*)*it;
		if (_tcscmp(s->szPhoneName,szPhoneName) == 0)
		{
			return s;
		}
	}

	return NULL;
}



int PECore::RemoveFromPhoneList(PE_PHONE_SETTING* setting)
{
	if (!setting)
	{
		return PE_RET_OK;
	}

	list<PE_PHONE_SETTING*>::iterator it;

	for (it = m_PhoneList.begin(); it != m_PhoneList.end();)
	{ 
		PE_PHONE_SETTING *s = (PE_PHONE_SETTING*)*it;
		if (_tcscmp(s->szPhoneID,setting->szPhoneID) == 0)
		{
			free(s);
			it = m_PhoneList.erase(it);
		}
		else
		{
			++it;
		}
	}

	return PE_RET_OK;
}



int PECore::FreePhoneList()
{
	list<PE_PHONE_SETTING*>::iterator it;

	for (it = m_PhoneList.begin(); it != m_PhoneList.end();)
	{ 
		PE_PHONE_SETTING *s = (PE_PHONE_SETTING*)*it;
		free(s);
		it = m_PhoneList.erase(it);
	}

	return PE_RET_OK;
}


PE_PHONE_SETTING* g_curPhoneInfo = NULL;
void PECore::SetCurPhone(PE_PHONE_SETTING* s)
{
	g_curPhoneInfo = s;
	if (s)
	{
		m_pDeviceAgent->SetHostInfo(s->szDevIP);
		CSettingHelper::SetLastPhoneName(s->szPhoneName);
	}
	else
	{
		m_pDeviceAgent->SetHostInfo(_T(""));
	}
}

PE_PHONE_SETTING* PECore::GetCurPhone()
{
	return g_curPhoneInfo;
}

int PECore::StartRestoreProcess(int iExportType,TCHAR *szFolderID)
{
	g_iCanSync = 1;
	m_pDeviceAgent->Export(iExportType,szFolderID);

	return PE_RET_OK;
}

int g_iStartPercet = 0;
int g_iEndPercet = 0;

int PECore::SyncContact(PE_PHONE_SETTING *s)
{
	PEWriteLog(_T("sync contact from Phone To PC Start"));
	g_bOutlookInstalled = COutLookHelper::LogIn();
	if (g_bOutlookInstalled == FALSE)
	{
		return PE_RET_FAIL;
	}

	if (s->dwContactSyncType == PE_SYNC_BOTH_WAY 
		|| s->dwContactSyncType == PE_SYNC_FROM_PHONE_2_PC)
	{
		if (!s->dwEnableContactAccount)
		{
			m_pDeviceAgent->SyncContact(_T(""));
		}
		else
		{
			list<TCHAR*> selAccounts = splitString(s->szContactAccounts,_T(';'));
			list<TCHAR*>::iterator it1;
			for (it1 = selAccounts.begin(); it1 != selAccounts.end() && g_iCanSync; ++it1)
			{ 
				TCHAR *s = (TCHAR*)*it1;
				if (s == NULL || _tcslen(s) < 1)
				{
					continue;
				}

				TCHAR *szAccount = URLEncodeW( s,(int)_tcslen(s));
				m_pDeviceAgent->SyncContact(szAccount);
				free(szAccount);
			}

			FreeStringList(selAccounts);
		}

#if 0
		PEWriteLog(_T("Start sync sim contact"));
		m_pDeviceAgent->SyncSimContact();
		PEWriteLog(_T("End sync sim contact"));
#endif
	}

	PEWriteLog(_T("sync contact from Phone To PC Done"));

	if (s->dwContactSyncType == PE_SYNC_BOTH_WAY 
		|| s->dwContactSyncType == PE_SYNC_FROM_PC_2_PHONE)
	{
		PEWriteLog(_T("start sync contact from PC to Phone"));
		PECore::SendSyncState2UI(PE_SYNC_CONTACT_2_PHONE,100,0);

		list<TCHAR*> selFolders = splitString(s->szContactFolderNames,_T(';'));

		float tmp = (float)selFolders.size() + 1;
		float tmp1 = 100;
		float fStep = tmp1/tmp;
		g_iEndPercet += (int)fStep;

		//sync device folder first
		ContactsHelper *pHelper = new ContactsHelper();
		pHelper->SyncContact2Phone(NULL);
		delete pHelper;

		if (selFolders.size() > 0)
		{
			g_iStartPercet = 0;
			g_iEndPercet = 0;

			list<TCHAR*>::iterator it1;

			int iFolderCount = 1;
			for (it1 = selFolders.begin(); it1 != selFolders.end() && g_iCanSync; ++it1)
			{ 
				++iFolderCount;
				g_iStartPercet = g_iEndPercet;
				g_iEndPercet += (int)(fStep * iFolderCount);
				if (g_iEndPercet > 100)
				{
					g_iEndPercet = 100;
				}

				TCHAR *szFolder = (TCHAR*)*it1;
				if (szFolder)
				{
					TCHAR szRealName[1024];
					URLDecoder(szFolder,szRealName,1024);

					szFolder = szRealName;

					TCHAR *szPst = szFolder;
					++szFolder;

					szFolder = _tcsstr(szFolder,_T("//"));
					if (szFolder)
					{
						*szFolder = 0;
						szFolder += 2;
					}

					if (szFolder)
					{
						g_bOutlookInstalled = COutLookHelper::LogInEx(szPst);
						BREAK_IF_NO_OUTLOOK;
						ContactsHelper *pHelper = new ContactsHelper();
						pHelper->SyncContact2Phone(szFolder);
						COutLookHelper::LogOut();
						delete pHelper;
					}
				}

				PECore::SendSyncState2UI(PE_SYNC_CONTACT_2_PHONE,100,g_iEndPercet);
			}

			FreeStringList(selFolders);
		}

		PEWriteLog(_T("start sync contact from PC to Phone Done"));

		PECore::SendSyncState2UI(PE_SYNC_CONTACT_2_PHONE,100,100);
	}

	return PE_RET_OK;

}


int PECore::TestCalendarSync()
{
	//COutLookHelper::SyncCalendar2Phone(CALENDAR_ROOT_FOLDER_NAME);

	return PE_RET_OK;
}


int PECore::SyncCalendar(PE_PHONE_SETTING *s)
{
	PEWriteLog(_T("SyncCalendar from Phone To PC Start"));
	g_bOutlookInstalled  = COutLookHelper::LogIn();
	if (g_bOutlookInstalled == FALSE)
	{
		return PE_RET_FAIL;
	}

	FILETIME ftSyncTime = {0};
	if (s->dwOnlySyncCalAfterTime)
	{
		ftSyncTime = s->ftCalendarSyncAfter;
	}

	if (s->dwCalSyncType == PE_SYNC_BOTH_WAY 
		|| s->dwCalSyncType == PE_SYNC_FROM_PHONE_2_PC)
	{
		if (s->dwOnlySyncSelectedCal)
		{
			list<TCHAR*> selIDs = splitString(s->szCalendarIDs,_T(';'));
			list<TCHAR*>::iterator it1;
			for (it1 = selIDs.begin(); it1 != selIDs.end() && g_iCanSync; ++it1)
			{ 
				TCHAR *s = (TCHAR*)*it1;
				m_pDeviceAgent->SyncCalendar(s,ftSyncTime);
			}

			FreeStringList(selIDs);
		}
		else
		{
			if (PE_RET_OK == m_pDeviceAgent->GetCalendarListFromDevice())
			{
				list<CAL_ITEM*>::iterator it;
				extern list<CAL_ITEM*> 		g_calendarList;
				for (it = g_calendarList.begin(); it != g_calendarList.end(); ++it)
				{ 
					CAL_ITEM *pCalendar = (CAL_ITEM*)*it;
					if (pCalendar->ulCalendarID < 1 )
					{
						PEWriteLog(_T("ulContactID < 0"));
						free(pCalendar);
						continue;
					}

					CString sID;
					sID.Format(_T("%d"),pCalendar->ulCalendarID);
					m_pDeviceAgent->SyncCalendar(sID.GetBuffer(),ftSyncTime);
					sID.ReleaseBuffer();

					free(pCalendar);
				}

				g_calendarList.clear();
			}
			else
			{
				PEWriteLog(_T("SyncCalendar: can not get calendar list from device"));
			}
		}
	}

	PEWriteLog(_T("SyncCalendar from Phone To PC Done"));

	if (s->dwCalSyncType != PE_SYNC_BOTH_WAY 
		&& s->dwCalSyncType != PE_SYNC_FROM_PC_2_PHONE)
	{
		return PE_RET_OK;
	}

	PEWriteLog(_T("start sync Calendar from PC to Phone"));
	PECore::SendSyncState2UI(PE_SYNC_CAL_2_PHONE,100,0);

	list<TCHAR*> selFolders = splitString(s->szCalendarFolderNames,_T(';'));

	PEWriteLog(_T("start sync Calendar from PC to Phone 1"));

	float tmp = (float)selFolders.size() + 1;
	float tmp1 = 100;
	float fStep = tmp1/tmp;
	g_iEndPercet += (int)fStep;

	//sync device folder first
	CalendarHelper *pHelper = new CalendarHelper();
	pHelper->SyncCalendar2Phone(NULL,ftSyncTime);
	delete pHelper;
	PEWriteLog(_T("start sync Calendar from PC to Phone2"));

	if (selFolders.size() <= 0)
	{
		PEWriteLog(_T("no calendar folder name found!"));
		PECore::SendSyncState2UI(PE_SYNC_CAL_2_PHONE,100,100);

		return PE_RET_OK;
	}

	g_iStartPercet = 0;
	g_iEndPercet = 0;

	list<TCHAR*>::iterator it1;

	int iFolderCount = 1;
	for (it1 = selFolders.begin(); it1 != selFolders.end() && g_iCanSync; ++it1)
	{ 
		++iFolderCount;
		g_iStartPercet = g_iEndPercet;
		g_iEndPercet += (int)(fStep * iFolderCount);
		if (g_iEndPercet > 100)
		{
			g_iEndPercet = 100;
		}

		PEWriteLog(_T("start sync Calendar from PC to Phone,folder name:"));
		TCHAR *szFolder = (TCHAR*)*it1;
		if (szFolder)
		{
			TCHAR szRealName[1024];
			URLDecoder(szFolder,szRealName,1024);

			szFolder = szRealName;

			TCHAR *szPst = szFolder;
			++szFolder;

			szFolder = _tcsstr(szFolder,_T("//"));
			if (szFolder)
			{
				*szFolder = 0;
				szFolder += 2;
			}

			if (szFolder)
			{
				g_bOutlookInstalled = COutLookHelper::LogInEx(szPst);
				BREAK_IF_NO_OUTLOOK;
				CalendarHelper *pHelper = new CalendarHelper();
				pHelper->SyncCalendar2Phone(szFolder,ftSyncTime);
				COutLookHelper::LogOut();
				delete pHelper;
			}
		}

		PEWriteLog(_T("start sync Calendar from PC to Phone 4"));
		PECore::SendSyncState2UI(PE_SYNC_CAL_2_PHONE,100,g_iEndPercet);
	}

	FreeStringList(selFolders);

	PEWriteLog(_T("start sync contact from PC to Phone Done"));

	PECore::SendSyncState2UI(PE_SYNC_CAL_2_PHONE,100,100);

	return PE_RET_OK;
}

DWORD g_dwShowSMSReprot = 1;

int PECore::StartSync(int iSyncType)
{
	HWND hWnd = GetMainUIHandle();

	if (PEIsRegistered() == PE_RET_FAIL)
	{
		if (!VerifyAPP())
		{
			DeviceAgent::ReportAppState(_T("Expired"));
			::PostMessage(hWnd,WM_ASK_FOR_REG,0,0);
			::PostMessage(hWnd,WM_SYNC_DONE,0,0);			
			return PE_RET_FAIL;
		}
	}

	::PostMessage(hWnd,WM_SYNC_STARTED,0,0);	

	g_iCanSync = 1;
	g_iInSync = 1;

	//SMS/MMS/Call transfer
	StartSyncProcess(iSyncType);

	PE_PHONE_SETTING *s = GetCurPhone();
	if (s == NULL)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_CONNNETION_STATE_OFFLINE;
	}

	//contacts sync
	if (iSyncType == PE_SYNC_CONTACT || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncContact)
		{
			SyncContact(s);
		}
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}


	//calendar sync
	if (iSyncType == PE_SYNC_CALENDAR || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncCalendar)
		{
			SyncCalendar(s);
		}
	}

	::PostMessage(hWnd,WM_SYNC_DONE,0,0);
	g_iInSync = 0;


	return PE_RET_OK;
}


int PECore::StartSyncProcess(int iSyncType)
{
	HWND hWnd = GetMainUIHandle();
	PE_PHONE_SETTING *s = GetCurPhone();
	if (s == NULL)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_CONNNETION_STATE_OFFLINE;
	}

	g_dwShowSMSReprot = s->dwShowSMSReprot;

	DeviceAgent::SetHostInfo(s->szDevIP);

	g_bOutlookInstalled = COutLookHelper::LogIn();
	if (g_bOutlookInstalled == FALSE)
	{
		return PE_RET_FAIL;
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	//SMS Sync
	if (iSyncType == PE_SYNC_SMS || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncSMS)
		{
			PEWriteLog(_T("sync sms start"));
			m_pDeviceAgent->SyncSms(iSyncType);
			PEWriteLog(_T("sync sms end"));
		}
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	//Image Sync
	if (iSyncType == PE_SYNC_IMAGE || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncImage)
		{
			PEWriteLog(_T("sync image start"));
			m_pDeviceAgent->SyncMedia(s->szImageFolderName,MEDIA_TYPE_IMAGE);
			PEWriteLog(_T("sync image end"));
		}
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	//Video Sync
	if (iSyncType == PE_SYNC_VIDEO || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncVideo)
		{
			PEWriteLog(_T("sync video start"));
			m_pDeviceAgent->SyncMedia(s->szVideoFolderName,MEDIA_TYPE_VIDEO);
			PEWriteLog(_T("sync video end"));
		}
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	//Audio Sync
	if (iSyncType == PE_SYNC_AUDIO || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncAudio)
		{
			PEWriteLog(_T("sync audio start"));
			m_pDeviceAgent->SyncMedia(s->szAudioFolderName,MEDIA_TYPE_AUDIO);
			PEWriteLog(_T("sync audio end"));
		}
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	//MMS Sync
	if (iSyncType == PE_SYNC_MMS || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncMMS)
		{
			PEWriteLog(_T("sync mms start"));
			m_pDeviceAgent->SyncMMS();
			PEWriteLog(_T("sync mms end"));
		}
	}
	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	//Call Log Sync
	if (iSyncType == PE_SYNC_CALL || iSyncType == PE_SYNC_ALL )
	{
		if (s->dwSyncCall)
		{
			PEWriteLog(_T("sync call start"));
			m_pDeviceAgent->SyncCallLog();
			PEWriteLog(_T("sync call end"));
		}
	}

	if (!g_iCanSync)
	{
		::PostMessage(hWnd,WM_SYNC_DONE,0,0);
		g_iInSync = 0;
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}


DWORD WINAPI TestConnT(LPVOID  lparam)
{
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	TCHAR *s = (TCHAR*)lparam;

	DeviceAgent::TestConn(s);

	free(s);

	return 0;
}


void CALLBACK TimerProc(HWND hWnd,UINT nMsg,UINT nTimerid,DWORD dwTime)
{
	if (g_iInEvtLoop)
	{
		//now busy,don't need to query device
		return;
	}

	static int iCount = 0;
	if (++iCount > 10)
	{
		iCount = 0;
		//update device info on UI
		PECore::PostMessage(WM_GET_DEV_INFO,0,0);
		return;
	}

	PECore::PostMessage(WM_QUERY_DEV_EVT,0,0);
}

void ShowSendState(TCHAR *szPara,int iSent)
{
	list<TCHAR*> status = splitString(szPara,_T(':'));
	if (status.size() != 3)
	{
		FreeStringList(status);
		PEWriteLog(_T("invalid sms status data"));
		ShowTrayInfo(_T("SMS sent failed"));
		return;
	}

	CString sTxt;

	if (iSent)
	{
		sTxt += _T("SMS sent to ");
	}
	else
	{
		sTxt += _T("SMS delivered to ");
	}

	list<TCHAR*>::iterator it = status.begin();
	TCHAR *szPhone = (TCHAR *)(*it);
	++it;
	sTxt += szPhone;

	TCHAR *szCode = (TCHAR *)(*it);

	int iCode = _ttoi(szCode);

	if (iCode == 0)
	{
		sTxt += _T(" successfully");
	}
	else
	{
		sTxt += _T(" failed");
	}

	ShowTrayInfo( (LPWSTR)(LPCWSTR)sTxt);

	FreeStringList(status);

}

void PECore::QueryDeviceEvt()
{
	static int iReported = 0;
	if (!iReported)
	{
		DeviceAgent::CheckUpdate();
		iReported = 1;
	}

	PE_PHONE_SETTING* s = GetCurPhone();
	if (s == NULL)
	{
		m_pDeviceAgent->DetectDevice();
		return;
	}

	DEVICE_EVT *pEvent = (DEVICE_EVT*)calloc(1,sizeof(DEVICE_EVT));
	int iRet = m_pDeviceAgent->QuerySysEvtFromDevice(pEvent);
	if (iRet != PE_RET_OK)
	{
		PostMessage(WM_DEVICE_DISCONNECTED,0,0);
		PEWriteLog(_T("QUERY_DEV_EVT failed,seems device is disconnected"));

		::PostMessage(g_hNewSmsWnd,WM_SEND_SMS_STATUS,PE_SEND_STATUS_CONN_DOWN,0);

		free(pEvent);
		return;
	}

	switch (pEvent->iEvtCode)
	{
	case SYS_EVT_NONE:
		{
			break;
		}
	case SYS_EVT_SMS_CHANGED:
		{
			PostMessage(WM_SMS_COMING,0,0);
			break;
		}
	case SYS_EVT_CALL_LOG_CHANGED:
		{
			PostMessage(WM_CALL_COMING,0,0);
			break;
		}
	case SYS_EVT_CONTACT_CHANGED:
		{
			PostMessage(WM_CONTACT_CHANGED,0,0);
			break;
		}
	case SYS_EVT_CALENDAR_CHANGED:
		{
			break;
		}
	case SYS_EVT_SMS_SENT_STATUS:
		{
			ShowSendState(pEvent->szPara,1);
			break;
		}
	case SYS_EVT_SMS_DELIVER_STATUS:
		{
			ShowSendState(pEvent->szPara,0);
			break;
		}
	case SYS_EVT_BATTERY_LEVEL_CHANGED:
		{
			::SendMessage(g_hHomeUIWnd,WM_BATTERY_LEVEL_CHANGED,(WPARAM)pEvent->szPara,0);
			break;
		}
	}

	if (pEvent->iHasMore)
	{
		PECore::PostMessage(WM_QUERY_DEV_EVT,0,0);
	}

	free(pEvent);
}


DWORD WINAPI PECore::CoreProcessor(LPVOID  lparam)
{
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);

	AfxSocketInit();

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	while(true)
	{
		if(GetMessage(&msg,0,0,0)) //get msg from message queue
		{
			if (msg.message <= WM_PE_MIN_MSG || msg.message >= WM_PE_MAX_MSG)
			{
				DispatchMessage(&msg);
				continue;
			}

			g_iInEvtLoop = 1;

			g_bOutlookInstalled = COutLookHelper::LogIn();

			switch(msg.message)
			{
			case WM_START_SRV:
				{
					PEWriteLog(_T("WM_START_SRV"));
					killProcessByName(_T("adb.exe"));

					g_iExitNow = 0;

					if (PE_RET_OK != CSettingHelper::GetDeviceConnState())
					{
						::PostMessage(GetMainUIHandle(),WM_ASK_INSTALL_CLIENT,0,0);
					}

					m_pDeviceAgent = new DeviceAgent();

					m_pDeviceAgent->DetectDevice();

#ifndef _DEBUG
					m_iTimerID = (int)SetTimer(0,0,1000*30,TimerProc);
#else
					m_iTimerID = (int)SetTimer(0,0,1000*3,TimerProc);
#endif

					break;
				}
			case WM_STOP_SRV:
				{
					PEWriteLog(_T("WM_STOP_SRV"));

					KillTimer(0,m_iTimerID);
					m_iTimerID = 0;

					PEWriteLog(_T("stop srv now"));
					delete m_pDeviceAgent;

					FilterHelper::DeInit();

					PECore::FreePhoneList();

					killProcessByName(_T("adb.exe"));
					PEWriteLog(_T("Exit CoreProcessor Now"));

					HANDLE hEvt = (HANDLE)msg.wParam;
					SetEvent(hEvt);
					g_iInEvtLoop = 0;

					COutLookHelper::LogOut();
					return 0;
				}
			case WM_GET_MSG_STORE:
				{
					BREAK_IF_NO_OUTLOOK;
					PEWriteLog(_T("WM_GET_MSG_STORE"));
					HWND hWnd = (HWND)msg.wParam;

					list<TCHAR*> profileList = COutLookHelper::GetStoreList();
					::SendMessage(hWnd,WM_MSG_STORE_COMING,(WPARAM)&profileList,0);
					FreeStringList(profileList);

					break;
				}
			case WM_QUERY_DEV_EVT:
				{
					QueryDeviceEvt();
					break;
				}
			case WM_SYNC_NOW:
				{
					PEWriteLog(_T("WM_SYNC_NOW"));
					BREAK_IF_NO_OUTLOOK;

					memset(&g_SyncStat,0,sizeof(g_SyncStat));

					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						PECore::PostMessage(WM_SCAN_DEVICE,0,0);
						break;
					}

					StartSync();

					if (msg.wParam == 1)
					{
						//auto started sync 
						PE_COMMON_SETTING *setting = (PE_COMMON_SETTING*)calloc(1,sizeof(PE_COMMON_SETTING));
						CSettingHelper::GetCommonSetting(setting);
						if (setting->dwExitAfterSync)
						{	
							PEWriteLog(_T("ExitAfterSync"));
							HWND hWnd = GetMainUIHandle();
							::PostMessage(hWnd,WM_STOP_SRV2,0,0);
						}

						free(setting);
					}
					break;
				}
			case WM_SCAN_DEVICE:
				{
					PEWriteLog(_T("WM_SCAN_DEVICE"));
					if (m_pDeviceAgent)
					{
						m_pDeviceAgent->DetectDevice();
					}
					break;
				}
			case WM_RESTORE_ITEM:
				{
					BREAK_IF_NO_OUTLOOK;
					PEWriteLog(_T("WM_RESTORE_ITEM"));
					MSG_Data_Truck *p = (MSG_Data_Truck*)msg.wParam;
					HWND hWnd = (HWND)p->p1;
					int iRestoreType = (int)p->p2;

					TCHAR szFolderID[500];
					_tcscpy(szFolderID,(TCHAR *)p->p3);
					HANDLE hEvt = (HANDLE)msg.lParam;
					SetEvent(hEvt);

					BREAK_IF_NO_OUTLOOK;
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					StartRestoreProcess(iRestoreType,szFolderID);

					break;
				}
			case WM_INSTALL_CLIENT:
				{
					PEWriteLog(_T("WM_INSTALL_CLIENT"));
					CAdbHelper *pHelper = new CAdbHelper();
					int iRet = pHelper->InstallApp2Device();
					delete pHelper;

					HWND hWnd = (HWND)msg.wParam;
					::PostMessage(hWnd,WM_INSTALL_RESULT,iRet,0);
					break;
				}
			case WM_UPDATE_DEVICE_LIST_ON_UI:
				{
					PEWriteLog(_T("WM_UPDATE_DEVICE_LIST_ON_UI"));
					PE_PHONE_SETTING* s = GetCurPhone();
					if (!s)
					{
						::PostMessage(g_hHomeUIWnd,WM_DEVICE_LIST_COMING,0,0);
						break;
					}

					::SendMessage(g_hHomeUIWnd,WM_DEVICE_LIST_COMING,(WPARAM)&m_PhoneList,(LPARAM)s->szPhoneID);
					break;
				}
			case WM_SWITCH_2_DEVICE:
				{
					PEWriteLog(_T("WM_SWITCH_2_DEVICE"));
					TCHAR *szPhoneName = (TCHAR*)msg.wParam;
					PE_PHONE_SETTING* s = FindPhoneFromPhoneList(szPhoneName);
					if (s)
					{
						SetCurPhone(s);
						PECore::PostMessage(WM_UPDATE_DEVICE_LIST_ON_UI,0,0);
					}

					HANDLE hEvt = (HANDLE)msg.lParam;
					SetEvent(hEvt);

					PECore::PostMessage(WM_GET_DEV_INFO,0,0);

					break;
				}
			case WM_DEVICE_CONNECTED:
				{
					PEWriteLog(_T("WM_DEVICE_CONNECTED"));
					MSG_Data_Truck *pTruck = (MSG_Data_Truck*)msg.wParam;
					PE_PHONE_SETTING* s = (PE_PHONE_SETTING*)pTruck->p1;
					PE_DEV_INFO *pDevInfo  = (PE_DEV_INFO*)pTruck->p2;

					free(pTruck);

					if (!s || !pDevInfo)
					{
						free(s);
						free(pDevInfo);
						PEWriteLog(_T("WM_DEVICE_CONNECTED"));
						break;
					}

					CSettingHelper::SetDeviceConnState();

					s->dwConnState = PE_CONNNETION_STATE_ONLINE;

					if (Add2PhoneList(s) == PE_RET_ALREADY_EXIST)
					{
						//already connected,do nothing
						if (GetCurPhone() != NULL)
						{
							free(s);
							free(pDevInfo);
							break;
						}
					}

					UpdateTrayIcon(TRAY_ICON_ONLINE);

					SetCurPhone(s);

					PECore::PostMessage(WM_UPDATE_DEVICE_LIST_ON_UI,0,0);

					::PostMessage(g_hHomeUIWnd,WM_DEVICE_CONNECTED,(WPARAM)pDevInfo,0);

					PE_COMMON_SETTING *setting = (PE_COMMON_SETTING*)calloc(1,sizeof(PE_COMMON_SETTING));
					CSettingHelper::GetCommonSetting(setting);

					extern int g_iAutoSync;
					if (setting->dwAutoSync && g_iAutoSync)
					{
#ifndef _DEBUG
						PostDelayMessage(WM_SYNC_NOW,1,0,30);
#else
						PostDelayMessage(WM_SYNC_NOW,1,0,6);
#endif
					}

					free(setting);

					break;
				}
			case WM_DEVICE_DISCONNECTED:
				{
					PEWriteLog(_T("WM_DEVICE_DISCONNECTED"));
					PE_PHONE_SETTING* s = GetCurPhone();
					RemoveFromPhoneList(s);

					s = GetNextPhone();
					SetCurPhone(s);
					if (s)
					{
						UpdateTrayIcon(TRAY_ICON_ONLINE);
					}
					else
					{
						UpdateTrayIcon(TRAY_ICON_OFFLINE);

						::PostMessage(g_hHomeUIWnd,WM_DEVICE_DISCONNECTED,0,0);
					}

					PECore::PostMessage(WM_UPDATE_DEVICE_LIST_ON_UI,0,0);

					m_pDeviceAgent->DetectDevice();
					break;
				}
			case WM_SMS_COMING:
				{
					BREAK_IF_NO_OUTLOOK;
					PEWriteLog(_T("WM_SMS_COMING"));
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					StartSync(PE_SYNC_SMS);
					StartSync(PE_SYNC_MMS);

					break;
				}
			case WM_CALL_COMING:
				{
					BREAK_IF_NO_OUTLOOK;
					PEWriteLog(_T("WM_CALL_COMING"));
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					StartSync(PE_SYNC_CALL);
					break;
				}
			case WM_GET_CAL_LIST:
				{
					PEWriteLog(_T("WM_GET_CAL_LIST"));
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					HWND hWnd = (HWND)msg.wParam;
					if (PE_RET_OK == m_pDeviceAgent->GetCalendarListFromDevice())
					{
						::SendMessage(hWnd,WM_CAL_LIST_COMING,1,0);
					}
					else
					{
						::PostMessage(hWnd,WM_CAL_LIST_COMING,0,0);
					}

					break;
				}
			case WM_GET_ACCOUNTS:
				{
					PEWriteLog(_T("WM_GET_ACCOUNTS"));
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					HWND hWnd = (HWND)msg.wParam;
					if (PE_RET_OK == m_pDeviceAgent->GetAccountsFromDevice())
					{
						::SendMessage(hWnd,WM_ACCOUNTS_COMING,1,0);
					}
					else
					{
						::PostMessage(hWnd,WM_ACCOUNTS_COMING,0,0);
					}		

					break;
				}
			case WM_APPLY_FILTER_NOW:
				{
					BREAK_IF_NO_OUTLOOK;
					PEWriteLog(_T("WM_APPLY_FILTER_NOW"));
					SmsHelper *pSms = new SmsHelper();
					if (pSms->openFolder())
					{
						pSms->ApplyFilter();
					}
					delete pSms;

					CallLogHelper *pCall = new CallLogHelper();
					if (pCall->openFolder())
					{
						pCall->ApplyFilter();
					}
					delete pCall;

					break;
				}
			case WM_DISABLE_CONN_BTN:
				{
					PEWriteLog(_T("WM_DISABLE_CONN_BTN"));
					::PostMessage(g_hHomeUIWnd,WM_DISABLE_CONN_BTN,msg.wParam,msg.lParam);
					break;
				}
			case WM_GET_DEV_INFO:
				{
					//PEWriteLog(_T("WM_GET_DEV_INFO"));
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					PE_DEV_INFO *pInfo = (PE_DEV_INFO *)calloc(1,sizeof(PE_DEV_INFO));

					if (PE_RET_OK != m_pDeviceAgent->GetDeviceInfoFromDevice(pInfo))
					{
						free(pInfo);
						break;
					}

					::PostMessage(g_hHomeUIWnd,WM_DEVICE_CONNECTED,(WPARAM)pInfo,0);
					break;
				}
			case WM_CONNECT_2_DEVICE:
				{
					PEWriteLog(_T("WM_CONNECT_2_DEVICE"));
					CreateThread(NULL,0,TestConnT,(TCHAR *)msg.wParam,0,0);
					break;
				}
			case WM_GET_SUB_FOLDER:
				{
					BREAK_IF_NO_OUTLOOK;
					PEWriteLog(_T("WM_GET_SUB_FOLDER"));
					list<TCHAR*> subFolderList = COutLookHelper::GetSubFolderList((int)msg.wParam);
					PEWriteLog(_T("WM_GET_SUB_FOLDER get list done"));
					HWND hWnd = (HWND)msg.lParam;
					::SendMessage(hWnd,WM_SUB_FOLDER_COMING,(WPARAM)&subFolderList,msg.wParam);
					PEWriteLog(_T("WM_GET_SUB_FOLDER done"));
					FreeStringList(subFolderList);
					break;
				}
			case WM_GET_CONTACTS_FROM_OUTLOOK:
				{
					BREAK_IF_NO_OUTLOOK;

					PEWriteLog(_T("WM_GET_CONTACTS_FROM_OUTLOOK"));
					list<CONTACT_ITEM*> contactList = COutLookHelper::GetContactList();
					HWND hWnd = (HWND)msg.wParam;
					//will free list from UI
					::SendMessage(hWnd,WM_CONTACTS_COMING,(WPARAM)&contactList,msg.wParam);

					break;
				}
			case WM_GET_SCHEDULE_SMS_LIST:
				{
					PEWriteLog(_T("WM_GET_SCHEDULE_SMS_LIST"));
					list<SCHEDULE_SMS_ITEM*> list = m_pDeviceAgent->GetScheduleSMSFromDevice();
					HWND hWnd = (HWND)msg.wParam;
					::SendMessage(hWnd,WM_SCHEDULE_SMS_LIST_COMING,(WPARAM)&list,0);
					break;
				}
			case WM_DEL_SCHEDULE_SMS:
				{
					PEWriteLog(_T("WM_DEL_SCHEDULE_SMS"));
					ULONG ulID = (ULONG)msg.wParam;
					m_pDeviceAgent->DeleteScheduleSMS(ulID);
					break;
				}
			case WM_SEND_SCHEDULE_SMS:
				{
					PEWriteLog(_T("WM_SEND_SCHEDULE_SMS"));
					SCHEDULE_SMS_ITEM *pSMS = (SCHEDULE_SMS_ITEM*)msg.wParam;
					m_pDeviceAgent->AddScheduleSMS(pSMS->szTo,pSMS->szContent,pSMS->ftSendTime);
					free(pSMS);
					break;
				}
			case WM_DELETE_ITEM:
				{
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					TCHAR *szEntryID = (TCHAR*)msg.wParam;
					PECore::DeleteItem(szEntryID);
					HANDLE hEvt = (HANDLE)msg.lParam;
					SetEvent(hEvt);
					break;
				}
			case WM_DELETE_SMS:
				{
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						break;
					}

					MSG_Data_Truck *tmpTruct= (MSG_Data_Truck*)msg.wParam;
					ULONG ulThreadID = _ttoi((TCHAR*)tmpTruct->p1);
					ULONG ulID = _ttoi((TCHAR*)tmpTruct->p2);

					free((void*)tmpTruct->p1);
					free((void*)tmpTruct->p2);
					free(tmpTruct);

					m_pDeviceAgent->DeleteSms(ulThreadID,ulID,s->szPhoneID);

					break;
				}
			case WM_SHOW_THREAD:
				{
					ShowThread((TCHAR*)msg.wParam);

					HANDLE hEvt = (HANDLE)msg.lParam;
					SetEvent(hEvt);
					break;
				}
			case WM_SAVE_CONTACT:
				{
					BREAK_IF_NO_OUTLOOK;
					TCHAR *szName = (TCHAR*)msg.wParam;
					TCHAR *szPhone = (TCHAR*)msg.lParam;
					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						free(szName);
						free(szPhone);
						break;
					}

					m_pDeviceAgent->SaveContact(szName,szPhone);

					free(szName);
					free(szPhone);
					break;
				}
			case WM_SEND_SMS:
				{
					TCHAR *szPhones = (TCHAR*)msg.wParam;
					TCHAR *szContent = (TCHAR*)msg.lParam;

					PE_PHONE_SETTING* s = GetCurPhone();
					if (s == NULL)
					{
						ShowTrayInfo( _T("Can not send SMS when device is connected"));
						free(szPhones);
						free(szContent);
						break;
					}

					PEWriteLog(_T("WM_SEND_SMS"));

					ShowTrayInfo( _T("Sending SMS  now"));

					m_pDeviceAgent->SendSMS(szPhones,szContent);
					free(szPhones);
					free(szContent);

					break;
				}
			case WM_DISABLE_SEND_BTN:
				{
					PEWriteLog(_T("WM_DISABLE_SEND_BTN"));
					::PostMessage((HWND)msg.wParam,WM_DISABLE_SEND_BTN,0,0);
					break;
				}
			case WM_STAT:
				{
					BREAK_IF_NO_OUTLOOK;
					TCHAR szName[255];
					TCHAR szPhone[255];
					memset(szName,0,sizeof(TCHAR)*255);
					memset(szPhone,0,sizeof(TCHAR)*255);
					if (PECore::GetNameAndPhoneFromItem((TCHAR*)msg.wParam,szName,szPhone) != PE_RET_OK)
					{
						break;
					}

					HANDLE hEvt = (HANDLE)msg.lParam;
					SetEvent(hEvt);

					extern int GenChartData(TCHAR *szName,TCHAR *szPhone);
					GenChartData(szName,szPhone);
					break;
				}
			case WM_GET_NAME_PHONE:
				{
					BREAK_IF_NO_OUTLOOK;
					MSG_Data_Truck *pData = (MSG_Data_Truck*)msg.wParam;

					PECore::GetNameAndPhoneFromItem((TCHAR*)pData->p1,(TCHAR*)pData->p2,(TCHAR*)pData->p3);

					HANDLE hEvt = (HANDLE)msg.lParam;
					SetEvent(hEvt);
					break;
				}
			default:
				{
					TCHAR szInfo[255];
					_stprintf(szInfo,_T("UI EVT:%d,transfer to UI"),msg.message);
					PEWriteLog(szInfo);

					HWND hWnd = GetMainUIHandle();
					::PostMessage(hWnd,msg.message,msg.wParam,msg.lParam);
					break;

				}
			}

			g_iInEvtLoop = 0;

			COutLookHelper::LogOut();
		}
	};

	AfxSocketTerm();

	return 0;
}


int PECore::StartSvr()
{
	m_iTimerID = 0;

	CreateThread(NULL,0,CoreProcessor,NULL,0,&m_dwThreadID);
	return 0;
}



DWORD WINAPI DelayPostMsgT(LPVOID  lparam)
{
	DelayData *pDelayData = (DelayData*)lparam;
	Sleep(1000 * pDelayData->delay);

	PECore::PostMessage(pDelayData->Msg,pDelayData->wParam,pDelayData->lParam);

	free(pDelayData);

	return PE_RET_OK;
}


void PECore::PostDelayMessage(UINT Msg,WPARAM wParam, LPARAM lParam,UINT delay)
{
	DelayData *delayArg = (DelayData *)calloc(1,sizeof(DelayData));
	delayArg->delay = delay;
	delayArg->Msg = Msg;
	delayArg->wParam = wParam;
	delayArg->lParam = lParam;

	CreateThread(NULL,0,DelayPostMsgT,(LPVOID)delayArg,0,0);
}


void PECore::PostMessage(UINT Msg,WPARAM wParam, LPARAM lParam)
{
	while(true)
	{
		if(PostThreadMessage(m_dwThreadID,Msg,wParam,lParam))//post thread msg
		{
			break;
		}
		Sleep(100);
	}
}


void PECore::SendMessage(UINT Msg,WPARAM wParam)
{
	if (g_iExitNow)
	{
		PEWriteLog(_T("Exit now,no more messages"));
		return;	
	}

	if (g_iInEvtLoop)
	{
		CString sInfo;
		sInfo.Format(_T("Caution:In evt loop,msg %d"),Msg);
		PEWriteLog(sInfo);
	}

	HANDLE hEvt = CreateEvent(0,FALSE,FALSE,0);
	PostMessage(Msg,wParam,(LPARAM)hEvt);
	WaitForSingleObject(hEvt,INFINITE);
	CloseHandle(hEvt);
}



void PECore::SendSyncState2UI(int iSyncType,int fTotalCount,int fCurCount)
{
	HWND hWnd = GetMainUIHandle();
	static PE_SYNC_STATUS info;
	memset(&info,0,sizeof(PE_SYNC_STATUS));

	info.iSyncType = iSyncType;
	info.fTotalCount = (float)fTotalCount;
	info.fCurCount = (float)fCurCount;

	::PostMessage(hWnd,WM_SYNC_INFO,(WPARAM)&info,0);
}

void PECore::SendRestoreState2UI(int iRestoreType,float fTotalCount,float fCurCount)
{
	PE_RESTORE_STATUS *pInfo = (PE_RESTORE_STATUS *)calloc(1,sizeof(PE_RESTORE_STATUS));

	pInfo->iRestoreType = iRestoreType;
	pInfo->fTotalCount = fTotalCount;
	pInfo->fCurCount = fCurCount;

	::PostMessage(g_hRestoreDlg,WM_RESTORE_INFO,(WPARAM)pInfo,0);
}

void PECore::SetSyncStat(int iItemType)
{
	switch (iItemType)
	{
	case PE_SYNC_SMS:
		{
			g_SyncStat.iSmsCount += 1;
			break;
		}

	case PE_SYNC_MMS:
		{
			g_SyncStat.iMmsCount += 1;
			break;
		}

	case PE_SYNC_CALL:
		{
			g_SyncStat.iCallCount += 1;
			break;
		}

	case PE_SYNC_CONTACT:
		{
			g_SyncStat.iContactsCount += 1;
			break;
		}

	case PE_SYNC_CALENDAR:
		{
			g_SyncStat.iCalendarCount += 1;
			break;
		}
	case PE_SYNC_CONTACT_2_PHONE:
		{
			break;
		}
	case PE_SYNC_CAL_2_PHONE:
		{
			break;
		}
	case PE_SYNC_IMAGE:
		{
			g_SyncStat.iImageCount += 1;
			break;
		}
	case PE_SYNC_VIDEO:
		{
			g_SyncStat.iVideoCount += 1;
			break;
		}
	case PE_SYNC_AUDIO:
		{
			g_SyncStat.iAudioCount += 1;
			break;
		}

	}

}



struct sortALG{
	bool operator()(const SMS_ITEM* s1,const SMS_ITEM* s2){
		return s1->ulMsgID < s2->ulMsgID;
	}
};



/*
<div class="MsgItem f_left">Gerry:how are y</div>
<div class="clear"></div>
<div class="MsgItem f_left">I am just fine</div>
<div class="clear"></div>
*/

void FmtSms(SMS_ITEM &sms,TCHAR *szSMS)
{
	TCHAR *tszFmtIn = _T("<div class=\"MsgItem f_left\"><span class=\"contactName\"><img src=\"img/contact.png\"/>%s <%s> :</span><p class=\"smsBody\">%s</p><span class=\"recvTime\"><img class=\"timeicon\" src=\"img/time.png\"/>%s %s</span></div><div class=\"clear\"></div>");
	TCHAR *tszFmtSent = _T("<div class=\"MsgItem f_right sentbk\"><span class=\"contactName\"><img src=\"img/contactsent.png\"/>%s :</span><p class=\"smsBody\">%s</p><span class=\"recvTime\"><img class=\"timeicon\" src=\"img/timesent.png\"/>%s %s</span></div><div class=\"clear\"></div>");

	FILETIME lftRecvTime;
	FileTimeToLocalFileTime(&sms.ftRecvTime,&lftRecvTime);

	SYSTEMTIME stRecvTime;
	FileTimeToSystemTime(&lftRecvTime,&stRecvTime);

	TCHAR szRecvDate[255];
	GetDateFormat(LOCALE_USER_DEFAULT, 0, &stRecvTime, NULL,szRecvDate, 255);

	TCHAR szRecvTime[255];
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stRecvTime, NULL,szRecvTime, 255);

	if (sms.ulMsgType == SMS_TYPE_RECEIVE)
	{
		_stprintf(szSMS,tszFmtIn,sms.tszName,sms.tszPhone,sms.pszBody,szRecvDate,szRecvTime);
	}
	else
	{
		_stprintf(szSMS,tszFmtSent,sms.tszName,sms.pszBody,szRecvDate,szRecvTime);
	}
}



int PECore::GetNameAndPhoneFromItem(TCHAR *szEntryID,TCHAR *szName,TCHAR *szPhoneNum)
{
	CALL_LOG_ITEM call;
	SMS_ITEM sms;
	//CONTACT_ITEM  contact;

	if (!szEntryID || _tcslen(szEntryID) < 1)
	{
		return PE_RET_FAIL;
	}

	TCHAR *szFolderID = _tcsstr(szEntryID,_T(";"));
	if (szFolderID)
	{
		*szFolderID = 0;
		szFolderID += 1;
	}

	CallLogHelper *pCall = new CallLogHelper();
	if (pCall->openFolderByID(szFolderID))
	{
		if (pCall->FindCallItemByEntryID(szEntryID,call) == PE_RET_OK)
		{
			_tcscpy(szName,call.szNameW);
			_tcscpy(szPhoneNum,call.szNumberW);
			delete pCall;
			return PE_RET_OK;
		}
	}
	delete pCall;

#if 0
	ContactsHelper *pContacts = new ContactsHelper();
	if (pContacts->openFolderByID(szFolderID))
	{
		if (pContacts->FindContactItemByEntryID(szEntryID,&contact) == PE_RET_OK)
		{
			//delete contact on phone	
			//m_pDeviceAgent->DeleteContact(contact.ulContactID,NULL);
			ContactsHelper::FreeContactItem(&contact);
			delete pContacts;
			return PE_RET_OK;
		}
	}
	delete pContacts;
#endif

	SmsHelper *pSms = new SmsHelper();
	if (pSms->openFolderByID(szFolderID))
	{
		if (pSms->FindSmsIemByEntryID(szEntryID,sms) == PE_RET_OK)
		{
			_tcscpy(szName,sms.tszName);
			if (sms.ulMsgType == SMS_TYPE_SENT)
			{
				if (_tcsstr(szName,_T("To ")) == szName)
				{
					_tcscpy(szName,sms.tszName + 3);
				}
			}
			_tcscpy(szPhoneNum,sms.tszPhone);
			delete pSms;
			return PE_RET_OK;
		}
	}
	delete pSms;

	return PE_RET_FAIL;
}

int PECore::DeleteItem(TCHAR *szEntryID)
{
	CALL_LOG_ITEM call;
	CONTACT_ITEM contact;
	SMS_ITEM sms;

	if (g_bOutlookInstalled == FALSE)
	{
		return PE_RET_FAIL;
	}

	memset(&call,0,sizeof(call));
	memset(&contact,0,sizeof(contact));
	memset(&sms,0,sizeof(sms));

	TCHAR *szFolderID = _tcsstr(szEntryID,_T(";"));
	if (szFolderID)
	{
		*szFolderID = 0;
		szFolderID += 1;
	}

	if (!szFolderID || _tcslen(szFolderID) < 1)
	{
		return PE_RET_FAIL;
	}

	CallLogHelper *pCall = new CallLogHelper();
	if (pCall->openFolderByID(szFolderID))
	{
		if (pCall->FindCallItemByEntryID(szEntryID,call) == PE_RET_OK)
		{
			//delete call on phone	
			SBinary bin;
			COutLookHelper::TCHAR2SBinary(szEntryID,bin);
			pCall->DeleteItemByEntryID(bin);
			m_pDeviceAgent->DeleteCall(call.ulCallID,NULL);
			delete pCall;
			return PE_RET_OK;
		}
	}
	delete pCall;

	ContactsHelper *pContacts = new ContactsHelper();
	if (pContacts->openFolderByID(szFolderID))
	{
		if (pContacts->FindContactItemByEntryID(szEntryID,&contact) == PE_RET_OK)
		{
			//delete contact on phone
			SBinary bin;
			COutLookHelper::TCHAR2SBinary(szEntryID,bin);
			pContacts->DeleteItemByEntryID(bin);
			m_pDeviceAgent->DeleteContact(contact.ulContactID,NULL);
			ContactsHelper::FreeContactItem(&contact);
			delete pContacts;
			return PE_RET_OK;
		}
	}
	delete pContacts;

	SmsHelper *pSms = new SmsHelper();
	if (pSms->openFolderByID(szFolderID))
	{
		if (pSms->FindSmsIemByEntryID(szEntryID,sms) == PE_RET_OK)
		{
			//delete sms on phone
			SBinary bin;
			COutLookHelper::TCHAR2SBinary(szEntryID,bin);
			pSms->DeleteItemByEntryID(bin);
			m_pDeviceAgent->DeleteSms(sms.ulThreadID,sms.ulMsgID,NULL);
			free(sms.pszBody);
			delete pSms;
			return PE_RET_OK;
		}
	}
	delete pSms;

	return PE_RET_FAIL;
}




TCHAR *g_tszThreadData;
TCHAR *GetThreadData()
{
	return g_tszThreadData;
}

TCHAR* PECore::ShowThread(TCHAR *szEntryID)
{
	SMS_ITEM sms;
	TCHAR *szFolderID = _tcsstr(szEntryID,_T(";"));
	if (szFolderID)
	{
		*szFolderID = 0;
		szFolderID += 1;
	}

	SmsHelper *pSmsHelper = new SmsHelper();
	if (!pSmsHelper->openFolder())
	{
		delete pSmsHelper;
		return NULL;
	}

	if (PE_RET_OK != pSmsHelper->FindSmsIemByEntryID(szEntryID,sms))
	{
		g_tszThreadData = NULL;
		delete pSmsHelper;
		return NULL;
	}

	list<SMS_ITEM*> smsList = pSmsHelper->GetSmsListByThreadID(szFolderID,sms.ulThreadID,NULL);
	if (smsList.size() < 1)
	{
		g_tszThreadData = NULL;
		delete pSmsHelper;
		return NULL;
	}

	smsList.sort(sortALG());

	//assume each sms take 1024 bytes
	TCHAR *pContent = new TCHAR[MAX_THREAD_SHOW_COUNT * 1024];
	TCHAR *pHeader = pContent;
	list<SMS_ITEM*>::iterator it;

	for (it = smsList.begin(); it != smsList.end(); ++it)
	{ 
		SMS_ITEM *pSms = (SMS_ITEM*)*it;
		TCHAR smsContent[2048];
		memset(smsContent,0,2048*sizeof(TCHAR));
		FmtSms(*pSms,smsContent);
		_tcscpy(pHeader,smsContent);
		pHeader += _tcslen(smsContent);

		free(pSms->pszBody);
		free(pSms);
	}

	_tcscpy(pHeader,_T("<br><br><br><br><br><br>"));

	g_tszThreadData = pContent;
	smsList.clear();
	delete pSmsHelper;

	return pContent;
}

#include "MiniHttpd.h"

CMiniHttpd *m_miniHttpd;


void PECore::StartUI(int iUIType,void *data)
{
	TCHAR szHost[255];
	int w = 450;
	int h = 300;
	int iPort = m_miniHttpd->m_nPort;
	TCHAR *szFileName = _T("");

	if (!m_miniHttpd)
	{
		return;
	}

	static int iStamp = 0;
	++iStamp;

	switch (iUIType)
	{
	case PE_CMD_THREAD:
		{
			_stprintf(szHost,_T("http://127.0.0.1:%d/ThreadList.html?timestamp=%d"),iPort,iStamp);
			CUIContainer *pUIContainer = new CUIContainer();
			pUIContainer->m_width = 750;
			pUIContainer->m_height = 500;
			pUIContainer->SetURL(szHost);
			pUIContainer->DoModal();

			delete pUIContainer;
			break;
		}
	default:
		{
			_stprintf(szHost,_T("http://127.0.0.1:%d/%s"),iPort,szFileName);
			break;
		}
	}

}


int PECore::GenStatData(SMS_STAT *pSmsStat,CALL_LOG_STAT *pCallStat)
{
	FILETIME ftStart,ftEnd;

	SmsHelper *pSmsHelper = new SmsHelper();
	if (!pSmsHelper->openFolder())
	{
		delete pSmsHelper;
		return PE_RET_FAIL;
	}

	pSmsHelper->GenStat(ftStart,ftEnd,pSmsStat);
	delete pSmsHelper;

	CallLogHelper *pCallHelper = new CallLogHelper();
	if (!pCallHelper->openFolder())
	{
		delete pCallHelper;
		return PE_RET_FAIL;
	}

	pCallHelper->GenStat(ftStart,ftEnd,pCallStat);
	delete pCallHelper;

	return PE_RET_OK;
}


int PECore::StopMiniHttpd()
{
	/*m_miniHttpd->Detach();
	if(m_miniHttpd)
	{
	m_miniHttpd->Close();
	}*/

	//delete m_miniHttpd;

	return 0;
}


int PECore::StartMiniHttpd()
{
	m_miniHttpd = new CMiniHttpd();
	m_miniHttpd->m_nPort = 20000;

	while( m_miniHttpd->Create(m_miniHttpd->m_nPort) == FALSE )
	{
		m_miniHttpd->m_nPort++;
		if (m_miniHttpd->m_nPort > 65535 )
		{
			break;
		}
	}

	if(m_miniHttpd->Listen()==FALSE)
	{
		return 0;
	}

	return 0;
}


void PECore::ShowTrayDlg(TCHAR *szMsg,int iType,TCHAR *szReplyAddr)
{
	EnterCriticalSection(&cs4TrayDlg);
	TRAY_MSG *pMsg = (TRAY_MSG *)calloc(1,sizeof(TRAY_MSG));
	pMsg->iType = iType;
	pMsg->szMsg = (TCHAR*)calloc(_tcslen(szMsg) + 1,sizeof(TCHAR));
	pMsg->szReplyAddr = (TCHAR*)calloc(_tcslen(szReplyAddr) + 1,sizeof(TCHAR));
	_tcscpy(pMsg->szMsg,szMsg);
	_tcscpy(pMsg->szReplyAddr,szReplyAddr);

	::SendMessage(GetMainUIHandle(),WM_SHOW_TRAY_INFO,(WPARAM)pMsg,0);

	LeaveCriticalSection(&cs4TrayDlg);

}

