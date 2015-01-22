#pragma once
#include "PEProtocol.h"

#include "afxinet.h"

#include "OutLookHelper.h"

//#include "ContactsHelper.h"

#import "msxml6.dll"
using namespace MSXML2;

void InitDataBuf(DataBuffer &data,int iDataLen = -1);
void DeInitDataBuf(DataBuffer &data);

ULONG ParseMaxID(DataBuffer &data);


class DeviceAgent
{
public:
	DeviceAgent(void);
	~DeviceAgent(void);

	void SyncSms(int iSyncType = PE_SYNC_ALL);
	int Export(int iRestoreType,TCHAR *szFolderID = NULL);
	void SyncCallLog();
	void SyncContact(TCHAR *szAccount);
	void SyncSimContact();
	void SyncCalendar(TCHAR *szCalendarID,FILETIME ftSyncAfter);
	void SyncMMS();
	void SyncMedia(TCHAR *szFolderPath, int iMediaType);
	int  SaveContact(TCHAR *szName,TCHAR *szPhone);

	int DeleteSms(ULONG ulThreadID,ULONG ulID,TCHAR* szPhoneID);
	int DeleteContact(ULONG ulID,TCHAR* szPhoneID);
	int DeleteCall(ULONG ulID,TCHAR* szPhoneID);
	static int RestoreSMS(SMS_ITEM& sms);
	static int RestoreCall(CALL_LOG_ITEM &callMsg);
	static int RestoreContact(CONTACT_ITEM  *pCal,int &iPhoneRetCode);

	static int SendSMS(TCHAR *szPhones,TCHAR *szContent);

	int GetCalendarListFromDevice();
	int ParseCalendarListFromXML(DataBuffer &data);

	int GetAccountsFromDevice();
	int ParseAccountsFromXML(DataBuffer &data);

	list<SCHEDULE_SMS_ITEM*> GetScheduleSMSFromDevice();
	list<SCHEDULE_SMS_ITEM*> ParseScheduleSMSFromXML(DataBuffer &data);
	int DeleteScheduleSMS(ULONG ulID);
	int AddScheduleSMS(TCHAR *szTo,TCHAR *szContent,FILETIME& ftSendTime);

	static int SaveCalendar2Phone(CAL_EVT_ITEM *pCal);

	static void SetHostInfo(TCHAR *tszIP);
	static TCHAR* GetHostInfo();
	static int TestConn(TCHAR *tszURL);

	static void ReportAppState(CString sEvt);
	static int CheckUpdate();
	static int GetDeviceWifiIP(TCHAR *pDeviceIP,TCHAR *szPhoneID);

	int GetDeviceInfoFromDevice(IN OUT PE_DEV_INFO *pDevInfo);
	static int ParseDeviceInfoFromXML(IN DataBuffer &data,IN OUT PE_DEV_INFO *pDevInfo);

	int QuerySysEvtFromDevice(DEVICE_EVT *pEvt);
	int ParseSysEvtFromXML(DataBuffer &data,DEVICE_EVT *pEvt);


	int DetectDevice();
#ifdef _ENABLE_ADB_CONNECTTION_		
	HANDLE m_hAdbThread;
#endif	
	HANDLE m_hWifiThread;

	int HttpRequest(CString sRequest,IN OUT DataBuffer &data);
	int Download(CString sRequest,FILE *pSaveFile);
	static int HttpPost(TCHAR *szRequest,TCHAR *szPostData,DataBuffer &data);
protected:
	static void SetInternetSessionOption(CInternetSession *pSession);
	int DoRequest(CString sRequest,IN OUT DataBuffer &data);
	int DeviceAgent::DoDownload(CString sRequest,FILE *pSaveFile);

	static TCHAR m_szHostURL[255];

	static MSXML2::IXMLDOMDocumentPtr GetXmlDoc(IN DataBuffer &data);

	int GetSMSListFromDevice(time_t ulFromDate);
	int ParseSMSListFromXML(IN DataBuffer &data);
	ULONG GetMaxSMSIDFromDevice();

	int GetCallLogListFromDevice(time_t ulFromDate);
	int ParseCallLogListFromXML(IN DataBuffer &data);
	ULONG GetMaxCallIDFromDevice();

	int GetContactPhotoFromDevice();
	int GetContactListFromDevice(ULONG ulFromID,TCHAR *szAccount);
	int ParseContactListFromXML(IN DataBuffer &data);
	ULONG GetMaxContactIDFromDevice();


	ULONG GetMaxSimContactIDFromDevice();
	int GetSimContactListFromDevice(ULONG ulFromID);
	int ParseSimContactListFromXML(DataBuffer &data);

	int GetEvtListFromDevice(time_t syncTime,TCHAR *szCalendarID);
	int ParseEvtListFromXML(DataBuffer &data);
	ULONG GetMaxCalEvtIDFromDevice(TCHAR *szCalendarID);
	
	int GetMmsMimeDataFromDevice();
	int GetMmsListFromDevice(time_t ulFromDate);
	int ParseMmsListFromXML(DataBuffer &data);

	int GetMediaListFromDevice(int iFrom,int iTo,int iMediaType);
	int ParseMediaListFromXML(DataBuffer &data,int iMediaType);

	int DeviceAgent::GetItemCountFromDevice(TCHAR *szAction);
	int ParseItemCountXML(DataBuffer &data);

	static DWORD WINAPI WifiScannerT(LPVOID  lparam);
	static DWORD WINAPI TestConnT(LPVOID  lparam);

	static ULONG ParseMaxID(DataBuffer &data);
	static int ParseRetCodeXML(DataBuffer &data);

	

};



