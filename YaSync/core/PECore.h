#ifndef	__PE_PE_CORE_HEADER__
#define __PE_PE_CORE_HEADER__

#include "DeviceAgent.h"

#include "FilterHelper.h"

TCHAR *GetThreadData();

class PECore
{
public:
	static int StartSvr();
	static void PostDelayMessage(UINT Msg,WPARAM wParam, LPARAM lParam,UINT delay);
	static void PostMessage(UINT Msg,WPARAM wParam, LPARAM lParam);
	static void SendMessage(UINT Msg,WPARAM wParam);
	static int StartMiniHttpd();
	static int StopMiniHttpd();

	static void ApplyFilterNow();

	static void SendSyncState2UI(int iSyncType,int fTotalCount,	int fCurCount);
	static void SendRestoreState2UI(int iSyncType,float fTotalCount,	float fCurCount);
	
	static void SetCurPhone(PE_PHONE_SETTING* s);
	static PE_PHONE_SETTING* GetCurPhone();

	static int GetCurPhoneName(TCHAR *szPhoneName);
	
	static PE_PHONE_SETTING* GetNextPhone();

	static int TestCalendarSync();
	static int GetNameAndPhoneFromItem(TCHAR *szEntryID,TCHAR *szName,TCHAR *szPhoneNum);
	static int DeleteItem(TCHAR *szEntryID);

	static void StartUI(int iUIType,void *data);
	static int GenStatData(SMS_STAT *pSmsStat,CALL_LOG_STAT *pCallStat);
	static void SetSyncStat(int iItemType);

	static void ShowTrayDlg(TCHAR *szMsg,int iType,TCHAR *szReplyAddr);

	static UINT_PTR m_iTimerID;
	
	static int StartSync(int iSyncType = PE_SYNC_ALL);

private:
	static int StartRestoreProcess(int iExportType,TCHAR *szFolderID);
	static int StartSyncProcess(int iSyncType);
	static int SyncContact(PE_PHONE_SETTING *s);
	static int SyncCalendar(PE_PHONE_SETTING *s);

	static void QueryDeviceEvt();
	static DWORD WINAPI CoreProcessor(LPVOID  lparam);
	static DeviceAgent *m_pDeviceAgent;
	static DWORD m_dwThreadID;
	static TCHAR* ShowThread(TCHAR *szEntryID);

	static int Add2PhoneList(PE_PHONE_SETTING* setting);
	static int RemoveFromPhoneList(PE_PHONE_SETTING* setting);
	static int FreePhoneList();
	static PE_PHONE_SETTING* FindPhoneFromPhoneList(TCHAR *szPhoneName);
	static list<PE_PHONE_SETTING*> m_PhoneList;//connected phone list
	
};

#endif

