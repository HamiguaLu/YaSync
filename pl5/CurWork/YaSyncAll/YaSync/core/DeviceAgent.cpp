#include "stdafx.h"
#include "DeviceAgent.h"
#include "AppTools.h"
#include "PEInterface.h"

#include "PECore.h"
#include "UrlEscape.h" 

#include "AdbHelper.h"
#include "SettingHelper.h"

#include "CalendarHelper.h"
#include "ContactsHelper.h"
#include "SmsHelper.h"
#include "MmsHelper.h"
#include "CallLogHelper.h"
#include "DeviceBlockHelper.h"
#include "TrayHelper.h"

TCHAR DeviceAgent::m_szHostURL[255];


list<SMS_ITEM*> 					g_smsList;
list<CALL_LOG_ITEM*>				g_callList;
list<CONTACT_ITEM*> 				g_contactList;
list<CAL_EVT_ITEM*> 				g_calEvtList;
list<MMS_ITEM*> 					g_mmsList;

list<CAL_ITEM*> 					g_calendarList;
list<ACCOUNT_ITEM*> 				g_accountList;

list<MEDIA_ITEM*> 					g_mediaList;


CRITICAL_SECTION cs4HttpConnection;
CRITICAL_SECTION cs4TestConn;

extern int g_iExitNow;
extern int g_iCanSync;
extern HWND g_hHomeUIWnd;


void InitDataBuf(DataBuffer &data,int iDataLen)
{
	if (iDataLen < 0)
	{
		iDataLen = PE_MAX_HTTP_DATA_LEN;
	}

	memset(&data,0,sizeof(data));

	data.iRawBufferLen = iDataLen;
	data.pRawBufffer = (char*)calloc(iDataLen,sizeof(char));
}



void ConvertDataBuf(DataBuffer &data)
{
	data.pRawBufffer[data.iRawBufferLen] = 0;
	data.iBufferLen = 1 + MultiByteToWideChar(CP_UTF8,0,data.pRawBufffer,-1,NULL,NULL);
	data.pBuffer = (TCHAR*)calloc(data.iBufferLen + 1,sizeof(TCHAR));
	int iLen = MultiByteToWideChar(CP_UTF8,0,data.pRawBufffer,-1,data.pBuffer,data.iBufferLen);
	data.iBufferLen = iLen;
	data.pBuffer[iLen] = 0;

}

void DeInitDataBuf(DataBuffer &data)
{
	free(data.pBuffer);
	free(data.pRawBufffer);

	memset(&data,0,sizeof(data));
}


void FreeContactList();

void FreeMmsList()
{
	list<MMS_ITEM*>::iterator it;
	for (it = g_mmsList.begin(); it != g_mmsList.end(); ++it)
	{ 
		MMS_ITEM *pMms = (MMS_ITEM*)*it;

		MmsHelper::FreeMimeItem(pMms);
		free(pMms);
	}

	g_mmsList.clear();
}


CString BSTR2StrringThenFree(BSTR &bstrValue)
{
	TCHAR *tszValue = NULL;
	char* pNodeName = _com_util::ConvertBSTRToString(bstrValue);
	SysFreeString(bstrValue);
	if (pNodeName == NULL)
	{
		return _T("");
	}

	int iSize = MultiByteToWideChar(CP_UTF8,0,pNodeName,-1,tszValue,0);
	if (!iSize)
	{
		//FAILED
		delete[] pNodeName;
		return _T("");
	}

	tszValue = new TCHAR[iSize];
	//memset(tszValue,0,iSize * sizeof(TCHAR));
	MultiByteToWideChar(CP_UTF8,0,pNodeName,-1,tszValue,iSize);
	delete[] pNodeName;
	CString sValue(tszValue);
	delete[] tszValue;

	return sValue;
}


/*检查参数有效性*/
void CheckSmsItem(SMS_ITEM &sms)
{
	CheckXmlString(sms.pszBody);
	CheckXmlString(sms.tszPhone);
	CheckXmlString(sms.tszName);
}

void CheckCallItem(CALL_LOG_ITEM &call)
{
	if (_tcslen(call.szNumberW) < 1)
	{
		_tcscpy(call.szNumberW , _T("NULL"));
	}

	if (_tcslen(call.szNameW) < 1)
	{
		_tcscpy(call.szNameW , _T(" "));
	}

	CheckXmlString(call.szNameW);
}


void PrintException(CInternetException * pException,TCHAR *szFunc)
{
	TCHAR   szCause[255];
	DWORD dwErrCode = pException->m_dwError;
	pException->GetErrorMessage(szCause, 255);
	CString sInfo;
	sInfo.Format(_T("In %s:Connection error!exception code is %d : %s"),szFunc,dwErrCode,szCause);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();
}


DeviceAgent::DeviceAgent()
{
#ifdef _ENABLE_ADB_CONNECTTION_
	m_hAdbThread = 0;
#endif	
	m_hWifiThread = 0;

	InitializeCriticalSectionAndSpinCount(&cs4HttpConnection,0x80000400);
	InitializeCriticalSectionAndSpinCount(&cs4TestConn,0x80000400);

	CAdbHelper::m_hExitEvt = CreateEvent(0,FALSE,FALSE,0);

	g_smsList.clear();
	g_callList.clear();
	g_contactList.clear();
	g_calEvtList.clear();
	g_mmsList.clear();
	g_calendarList.clear();
	g_accountList.clear();
}

DeviceAgent::~DeviceAgent()
{
	SetEvent(CAdbHelper::m_hExitEvt);
#ifdef _ENABLE_ADB_CONNECTTION_	
	WaitForSingleObject(m_hAdbThread,1000 * 60);
#endif	
	WaitForSingleObject(m_hWifiThread,1000 * 60);

	PEWriteLog(_T("Dev scan thread exited"));
}


DWORD WINAPI DeviceAgent::WifiScannerT(LPVOID  lparam)
{
	TCHAR szInfo[255];
	TCHAR szWifiIP[100] = {0};

	PEWriteLog(_T("WifiScannerT started,connect from reg first"));	
	list<PE_PHONE_SETTING*> iplist = CSettingHelper::GetPhoneInfoList();
	::PostMessage(g_hHomeUIWnd,WM_SHOW_CONN_STAT,(WPARAM)SCAN_STAT_CONNECT_FROM_HISTORY,0);

	list<PE_PHONE_SETTING*>::iterator it;
	for (it = iplist.begin(); it != iplist.end() && !g_iExitNow; ++it)
	{ 
		PE_PHONE_SETTING *s = (PE_PHONE_SETTING*)*it;
		if (_tcslen(s->szDevIP) < 4 || _tcscmp(s->szDevIP,_T("127.0.0.1")) == 0)
		{
			free(s);
			continue;
		}

		_stprintf(szInfo,_T("wifiscanner:Test conn by Setting:reg,ip is %s"),s->szDevIP);
		PEWriteLog(szInfo);
#if 0
		DeviceAgent::TestConn(s->szDevIP);
#else
		TCHAR *szIP = (TCHAR*)malloc(255);
		_tcscpy(szIP,s->szDevIP);
		CreateThread(NULL,0,TestConnT,(LPVOID)szIP,0,0);
#endif

		free(s);
	}

	iplist.clear();

	if (g_iExitNow)
	{
		return 0;
	}

	PEWriteLog(_T("WifiScannerT get device ip from server"));
	
	::PostMessage(g_hHomeUIWnd,WM_SHOW_CONN_STAT,(WPARAM)SCAN_STAT_QUERY_FROM_SRV,0);
	if (PE_RET_OK == DeviceAgent::GetDeviceWifiIP(szWifiIP,_T("")) && !g_iExitNow)
	{
		_stprintf(szInfo,_T("WIFI ip from server is %s,connect now"),szWifiIP);
		PEWriteLog(szInfo);

		::PostMessage(g_hHomeUIWnd,WM_SHOW_CONN_STAT,(WPARAM)SCAN_STAT_CONNECT_2_CLIENT,0);
		DeviceAgent::TestConn(szWifiIP);
	}

	::PostMessage(g_hHomeUIWnd,WM_SHOW_CONN_STAT,(WPARAM)SCAN_STAT_STOP,0);

	PEWriteLog(_T("WifiScannerT ended"));

	return 0;
}


int DeviceAgent::DetectDevice()
{
#ifdef _ENABLE_ADB_CONNECTTION_
	if (m_hAdbThread)
	{
		//wait for last running thread
		if (WaitForSingleObject(m_hAdbThread,0) == WAIT_OBJECT_0)
		{
			CloseHandle(m_hAdbThread);
			m_hAdbThread = 0;
		}
	}

	if (!m_hAdbThread)
	{
		//start new thread
		killProcessByName(_T("adb.exe"));
		m_hAdbThread = CreateThread(NULL,0,CAdbHelper::AdbScannerT,0,0,0);
	}
#endif

	if (m_hWifiThread)
	{
		//wait for last running thread
		if (WaitForSingleObject(m_hWifiThread,0) == WAIT_OBJECT_0)
		{
			CloseHandle(m_hWifiThread);
			m_hWifiThread = 0;
		}
	}

	if (!m_hWifiThread)
	{
		m_hWifiThread = CreateThread(NULL,0,WifiScannerT,0,0,0);
	}

	return 0;
}

void DeviceAgent::SetHostInfo(TCHAR *tszIP)
{
	_tcscpy(m_szHostURL,tszIP);
}

TCHAR* DeviceAgent::GetHostInfo()
{
	return m_szHostURL;
}


void DeviceAgent::SetInternetSessionOption(CInternetSession *pSession)
{
	pSession->SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,		HTTP_CONNECTION_TIME_OUT);
	pSession->SetOption(INTERNET_OPTION_SEND_TIMEOUT, 			HTTP_SEND_TIME_OUT);
	pSession->SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT, 		HTTP_RECV_TIME_OUT);
	pSession->SetOption(INTERNET_OPTION_DATA_SEND_TIMEOUT, 		HTTP_SEND_TIME_OUT);
	pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT, 	HTTP_RECV_TIME_OUT);
	pSession->SetOption(INTERNET_OPTION_CONNECT_RETRIES, 		1);
}


void DeviceAgent::ReportAppState(CString sEvt)
{
	CInternetSession *pSession = new CInternetSession(_T("PETestSession"));
	SetInternetSessionOption(pSession);
	int iNeedUpdate = 0;

	CHttpFile *file = NULL;
	CString sURL;
	sURL.Format(_T("http://a.mobitnt.com/ReportAppState.php?P=YaSync%d&evt=%s"),APP_VER,sEvt);

	//发起http请求
	try
	{
		file = (CHttpFile*)pSession->OpenURL(sURL.GetBuffer());
	}
	catch(CInternetException * pException)
	{
		PrintException(pException,_T("ReportAppState"));
		pException->Delete();
	}

	sURL.ReleaseBuffer();

	if (file)
	{
		file->Close();
		delete file;
	}

	if (pSession)
	{
		pSession->Close();
		delete pSession;
	}

	return;
}


int DeviceAgent::CheckUpdate()
{
	EnterCriticalSection(&cs4HttpConnection);

	CInternetSession *pSession = new CInternetSession(_T("PETestSession"));
	SetInternetSessionOption(pSession);

	CHttpFile *file = NULL;
	int iRet = PE_RET_FAIL;
	char *pBuffer = NULL;

	try
	{
		file = (CHttpFile*)pSession->OpenURL(_T("http://a.mobitnt.com/YaSyncVer.php"));
		if (file == NULL)
		{
			goto ExitConn;
		}
		//先将http数据全部读出来，然后整体进行转换
		pBuffer = new char[1024 * 5];
		memset(pBuffer,0,1024 * 5);

		int iXmlLen = file->Read(pBuffer,1024 * 5);
		if (iXmlLen <= 0)
		{
			goto ExitConn;
		}

		pBuffer[iXmlLen] = 0;

		int iVer = atoi(pBuffer);
		if (iVer > APP_VER)
		{
			HWND hWnd = GetMainUIHandle();
			::PostMessage(hWnd,WM_NEW_VER_FOUND,0,0);

			iRet = PE_RET_OK;
		}
	}
	catch(CInternetException * pException)
	{
		PrintException(pException,_T("CheckUpdate"));
		pException->Delete();
	}

ExitConn:
	if (pBuffer != NULL)
	{
		delete[] pBuffer;
		pBuffer = NULL;
	}

	if (file)
	{
		file->Close();
		delete file;
	}

	if (pSession)
	{
		pSession->Close();
		delete pSession;
	}

	LeaveCriticalSection(&cs4HttpConnection);

	return iRet;
}


int DeviceAgent::GetDeviceWifiIP(TCHAR *pDeviceIP,TCHAR *szPhoneID)
{
	EnterCriticalSection(&cs4HttpConnection);
	CInternetSession *pSession = new CInternetSession(_T("PETestSession"));
	SetInternetSessionOption(pSession);
	int iNeedUpdate = 0;

	CHttpFile *file = NULL;
	int iRet = PE_RET_FAIL;
	char *pBuffer = NULL;

	try
	{
		CString sURL;
		TCHAR *szIMEI = URLEncodeW(szPhoneID,(int)_tcslen(szPhoneID));
		sURL.Format(_T("http://a.mobitnt.com/GetIp4PE.php?action=getdeviceip&imei=%s"),szIMEI);
		free(szIMEI);
		file = (CHttpFile*)pSession->OpenURL(sURL);

		if (file == NULL)
		{
			PEWriteLog(_T("No Data from server"));
			goto ExitConn;
		}

		//先将http数据全部读出来，然后整体进行转换
		pBuffer = (char *)calloc(1,1024 * 6);

		int iXmlLen = file->Read(pBuffer,1024 * 5);
		if (iXmlLen <= 0)
		{
			PEWriteLog(_T("No Data from server"));
			goto ExitConn;
		}

		char *p = strstr(pBuffer,";");
		if (p)
		{
			*p = 0;
		}

		MultiByteToWideChar(CP_ACP,0,pBuffer,-1,pDeviceIP,64);

		struct sockaddr_in sock;
		int addrSize = sizeof( struct sockaddr_in );
		memset( &sock, 0, addrSize );
		sock.sin_family = AF_INET;
		if (0 == WSAStringToAddress( pDeviceIP, AF_INET, NULL, (LPSOCKADDR) &sock, &addrSize ))
		{
			iRet = PE_RET_OK;
		}
		else
		{
			iRet = PE_RET_FAIL;
			PEWriteLog(_T("IP from server is invalid"));
		}
	}
	catch(CInternetException * pException)
	{
		PrintException(pException,_T("GetDeviceWifiIP"));
		pException->Delete();
		iRet = PE_RET_FAIL;
	}

ExitConn:
	if (pBuffer != NULL)
	{
		free(pBuffer);
		pBuffer = NULL;
	}

	if (file)
	{
		file->Close();
		delete file;
	}

	if (pSession)
	{
		pSession->Close();
		delete pSession;
	}

	LeaveCriticalSection(&cs4HttpConnection);

	return iRet;
}


DWORD WINAPI DeviceAgent::TestConnT(LPVOID  lparam)
{
	TCHAR *szIP = (TCHAR*)lparam;
	DeviceAgent::TestConn(szIP);
	free(szIP);
	
	return 0;
}


int DeviceAgent::TestConn(TCHAR *szDeviceIP)
{
	::CoInitialize(NULL);
	EnterCriticalSection(&cs4TestConn);
	int iTryCount = 3;
	CString sURL;
	CString sSecurityCode = _T("");
	DataBuffer data;
	memset(&data,0,sizeof(data));
TRYAGAIN:
	sURL.Format(_T("http://%s:%d/AuthConn.xml?action=RequestAuth&securitycode=%s"),szDeviceIP,PE_ANDROID_SVR_PORT,sSecurityCode);
	CInternetSession *pSession = new CInternetSession(_T("PETestSession"));
	SetInternetSessionOption(pSession);
	int iRet = PE_RET_FAIL;

	CHttpFile *file = NULL;
	memset(&data,0,sizeof(data));

	TCHAR szInfoabc[255];
	_stprintf(szInfoabc,_T("Test conn start,ip is %s,count: %d"),szDeviceIP,iTryCount);
	PEWriteLog(szInfoabc);

	//发起http请求
	try
	{
		file = (CHttpFile*)pSession->OpenURL(sURL.GetBuffer());
		//先将http数据全部读出来，然后整体进行转换
		int iBufLen = (int)file->GetLength();
		InitDataBuf(data,iBufLen + 1);

		//int iRet = PE_RET_FAIL;

		data.iRawBufferLen = file->Read(data.pRawBufffer,iBufLen);
		if (data.iRawBufferLen <= 0)
		{
			goto ExitConn;
		}

		ConvertDataBuf(data);

		PE_DEV_INFO *pDevInfo = (PE_DEV_INFO *)calloc(1,sizeof(PE_DEV_INFO));
		if (PE_RET_OK != ParseDeviceInfoFromXML(data,pDevInfo))
		{
			free(pDevInfo);
			iTryCount = -1;
			goto ExitConn;
		}

		PE_PHONE_SETTING *pSetting = (PE_PHONE_SETTING *)calloc(1,sizeof(PE_PHONE_SETTING));
		if (DeviceBlockHelper::IsItemInFilterList(pDevInfo->szPhoneID) == PE_RET_OK)
		{
			PEWriteLog(_T("Blocked device"));

			free(pSetting);
			free(pDevInfo);
			iTryCount = -1;
			goto ExitConn;
		}

		_tcscpy(pSetting->szPhoneName,pDevInfo->szPhoneModel);
		_tcscpy(pSetting->szPhoneID,pDevInfo->szPhoneID);
		pSetting->dwSDKVer = pDevInfo->dwSDKVer;
		if (pSetting->dwSDKVer < 14)
		{
			pSetting->dwSyncCalendar = 0;
			PEWriteLog(_T("SDK Ver below 14,disable calendar sync"));
		}

		PEWriteLog(_T("Phone Name:"));
		PEWriteLog(pSetting->szPhoneName);

		CSettingHelper::GetPhoneSetting(pSetting);
		_tcscpy(pSetting->szDevIP,szDeviceIP);
		CSettingHelper::SetPhoneSetting(pSetting);

		if (pDevInfo->iAuthState != 0)
		{
			if (sSecurityCode.GetLength() < 1 && _tcslen(pSetting->szSecurityCode) == 5)
			{
				sSecurityCode = pSetting->szSecurityCode;
				++iTryCount;
				free(pSetting);
				free(pDevInfo);
				goto ExitConn;
			}

			//something wrong,need enter security code
			PEWriteLog(_T("Invalid Security code"));
			PECore::SendMessage(WM_WRONG_SECURITY_CODE,(WPARAM)pSetting);
			iTryCount = -1;
			free(pSetting);
			free(pDevInfo);
			iRet = PE_RET_WRONG_PWD;
			goto ExitConn;
		}


		int iClientVer = _ttoi(pDevInfo->szAppVer);
		if (iClientVer > APP_CLIENT_VER)
		{
			PEWriteLog(_T("PC version is low!"));
			PECore::PostMessage(WM_WRONG_PC_VERSION,0,0);
		}
		else if (iClientVer < APP_CLIENT_VER )
		{
			PEWriteLog(_T("Client version is low!"));
			PECore::PostMessage(WM_WRONG_CLIENT_VERSION,0,0);
		}

		MSG_Data_Truck *tmpTruct = (MSG_Data_Truck *)calloc(1,sizeof(MSG_Data_Truck));
		tmpTruct->p1 = (WPARAM)pSetting;
		tmpTruct->p2 = (WPARAM)pDevInfo;

		iRet = PE_RET_OK;

		PEWriteLog(_T("send connect evt now"));
		PECore::PostMessage(WM_DEVICE_CONNECTED,(WPARAM)tmpTruct,0);
		PEWriteLog(_T("Connected to device"));

		//will free when phone is disconnected
		//free(pSetting);
	}
	catch(CInternetException * pException)
	{
		PrintException(pException,_T("TestConn"));
		pException->Delete();
		iRet = PE_RET_FAIL;
	}

ExitConn:
	sURL.ReleaseBuffer();

	DeInitDataBuf(data);

	if (file)
	{
		file->Close();
		delete file;
	}

	if (pSession)
	{
		pSession->Close();
		delete pSession;
	}

	if (iRet == PE_RET_OK || iTryCount <= 0 )
	{
		PEWriteLog(_T("Test conn end"));
		LeaveCriticalSection(&cs4TestConn);
		return iRet;
	}

	if (g_iExitNow)
	{
		PEWriteLog(_T("Test conn exit during app exit"));
		return PE_RET_NEED_EXIT;
	}

	--iTryCount;
	goto TRYAGAIN;
}




int DeviceAgent::RestoreSMS(SMS_ITEM& smsMsg)
{
	if (smsMsg.pszBody)
	{
		return PE_RET_FAIL;
	}

	CString sReq;
	int iLen = 255;
	TCHAR *szName = URLEncodeW(smsMsg.tszName,255);
	TCHAR *szPhone = URLEncodeW(smsMsg.tszPhone,255);
	TCHAR *szBody = URLEncodeW(smsMsg.pszBody,(int)_tcslen(smsMsg.pszBody));

	if (!szBody)
	{
		free(szName);
		free(szPhone);
		return PE_RET_FAIL;
	}

	time_t recvTime = FileTime2JavaTime(&smsMsg.ftRecvTime);
	sReq.Format(_T("action=RestoreSMS(&read=%d&name=%s&phone=%s&body=%s&type=%d&date=%lld"),
		smsMsg.ulReadFlag,szName,szPhone,szBody,smsMsg.ulMsgType,recvTime);

	free(szName);
	free(szPhone);
	free(szBody);

	DataBuffer data;

	if (PE_RET_OK != HttpPost(_T("SmsList.xml"),sReq.GetBuffer(),data))
	{
		sReq.ReleaseBuffer();
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	DeInitDataBuf(data);
	sReq.ReleaseBuffer();

	return PE_RET_OK;

}

int DeviceAgent::RestoreCall(CALL_LOG_ITEM &call)
{
	CString sReq;
	TCHAR *szName = URLEncodeW(call.szNameW,255);
	TCHAR *szPhone = URLEncodeW(call.szNumberW,255);

	time_t starttime = FileTime2JavaTime(&call.ftStartTime);
	time_t endtime = FileTime2JavaTime(&call.ftEndTime);

	sReq.Format(_T("action=restorecall&type=%d&name=%s&phone=%s&starttime=%lld&duration=%llu"),
		call.ulCallType,szName,szPhone,starttime,(endtime - starttime)/1000);

	free(szName);
	free(szPhone);

	PEWriteLog(sReq.GetBuffer());

	DataBuffer data;
	if (PE_RET_OK != HttpPost(_T("CallLog.xml"),sReq.GetBuffer(),data))
	{
		sReq.ReleaseBuffer();
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	DeInitDataBuf(data);
	sReq.ReleaseBuffer();

	return PE_RET_OK;


	return 0;
}


int DeviceAgent::Download(CString sRequest,FILE *pSaveFile)
{
	int iTry = 5;
	int iRet = PE_RET_FAIL;

	while (iTry-- > 0)
	{
		if (g_iExitNow == 1)
		{
			return -1;
		}

		fseek(pSaveFile,0,SEEK_SET);
		iRet = DoDownload(sRequest,pSaveFile);
		if (iRet == PE_RET_OK)
		{
			fflush(pSaveFile);
			return iRet;
		}
	}

	return iRet;
}


int DeviceAgent::HttpRequest(CString sRequest,IN OUT DataBuffer &data)
{
	int iTry = 5;
	int iRet = PE_RET_FAIL;

	memset(&data,0,sizeof(data));

	while (iTry-- > 0)
	{
		if (g_iExitNow == 1)
		{
			return -1;
		}

		iRet = DoRequest(sRequest,data);
		if (iRet == PE_RET_OK)
		{
			return iRet;
		}
	}

	return iRet;
}

int DeviceAgent::HttpPost(TCHAR *szRequest,TCHAR *szPostData,DataBuffer &data)
{
	EnterCriticalSection(&cs4HttpConnection);

	CInternetSession m_InetSession(_T("PESessionPost"),0,INTERNET_OPEN_TYPE_PRECONFIG,NULL,NULL,INTERNET_FLAG_DONT_CACHE);
	SetInternetSessionOption(&m_InetSession);
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	int iRet = PE_RET_FAIL;
	CString sContentLen;
	INTERNET_PORT nPort = PE_ANDROID_SVR_PORT;

	memset(&data,0,sizeof(data));

	try{
		CString strHeaders = _T("Content-Type: application/x-www-form-urlencoded\r\n)");
		pServer = m_InetSession.GetHttpConnection(m_szHostURL,nPort );
		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST,szRequest);
		pFile->AddRequestHeaders(strHeaders);

		int iDataLen = WideCharToMultiByte(CP_UTF8,0,szPostData,(int)_tcslen(szPostData),0,0,0,0);
		char *szData = (char *)calloc(iDataLen + 1,1);
		WideCharToMultiByte(CP_UTF8,0,szPostData,(int)_tcslen(szPostData),szData,iDataLen,0,0);

		pFile->SendRequestEx((DWORD)iDataLen);
		pFile->Write(szData,iDataLen);
		pFile->EndRequest();

		free(szData);

		DWORD dwRet;
		pFile->QueryInfoStatusCode(dwRet);
		if (dwRet != HTTP_STATUS_OK)
		{
			goto ONEXIT;
		} 

		pFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, sContentLen);
		int iBufLen = _ttoi(sContentLen.GetBuffer());
		sContentLen.ReleaseBuffer();
		if (iBufLen <= 0)
		{
			goto ONEXIT;
		}

		InitDataBuf(data,iBufLen + 1);
		data.iRawBufferLen = pFile->Read(data.pRawBufffer,data.iRawBufferLen);
		ConvertDataBuf(data);
		iRet = PE_RET_OK;
	}
	catch (CInternetException* e)
	{
		PrintException(e,_T("HttpPost"));
		e->Delete();
	}

ONEXIT:
	if (pFile)
	{
		pFile->Close();
		delete pFile;
	}

	if (pServer)
	{
		pServer->Close();
		delete pServer;
	}

	m_InetSession.Close();

	LeaveCriticalSection(&cs4HttpConnection);

	return iRet;	
}


int DeviceAgent::DoDownload(CString sRequest,FILE *pSaveFile)
{
	EnterCriticalSection(&cs4HttpConnection);

	CInternetSession *pHttpSession = NULL;
	CHttpConnection* pHttpConnection = NULL;
	CHttpFile *pHttpFile = NULL;
	int iRet = PE_RET_FAIL;
	INTERNET_PORT nPort = PE_ANDROID_SVR_PORT;
	CString sContentLen;

	//发起http请求
	try
	{
		pHttpSession = new CInternetSession(_T("PESessionGet"));
		SetInternetSessionOption(pHttpSession);
		pHttpConnection = pHttpSession->GetHttpConnection(m_szHostURL,nPort);
		DWORD HttpRequestFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;

		pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,sRequest, NULL, 1, NULL, (LPCTSTR)"1.0", HttpRequestFlags);
		if (!pHttpFile)
		{
			PEWriteLog(_T("pHttpFile is NULL"));
			goto ExitConn;
		}

		// Use direct write to posting field!
		CString strHeaders = _T("Accept: text/*\r\n");
		strHeaders += _T("User-Agent: YaSync\r\n");
		strHeaders += _T("Accept-Language: en-us\r\n");

		pHttpFile->AddRequestHeaders((LPCTSTR)strHeaders);
		pHttpFile->SendRequest();

		DWORD dwRet = 0;
		pHttpFile->QueryInfoStatusCode(dwRet);
		if (dwRet != HTTP_STATUS_OK)
		{
			goto ExitConn;
		}

		if (!pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, sContentLen))
		{
			PEWriteLog("http QueryInfo failed");
			goto ExitConn;
		}

		int iContentLen = _ttoi(sContentLen.GetBuffer());
		sContentLen.ReleaseBuffer();

		//先将http数据全部读出来，然后整体进行转换
		if(iContentLen <= 0)
		{
			PEWriteLog("CONTENT_LENGTH is 0");
			goto ExitConn;
		}

		char szBuf[1024];

		do
		{
			int iRead = pHttpFile->Read(szBuf,1024);
			if (iRead < 1)
			{
				break;
			}

			fwrite(szBuf,iRead,1,pSaveFile);
			iContentLen -= iRead;
		}while (iContentLen);

		if (iContentLen > 0)
		{
			PEWriteLog(_T("Read failed========"));
			iRet = PE_RET_FAIL;
		}
		else
		{
			iRet = PE_RET_OK;
		}
	}
	catch(CInternetException * pException)
	{
		PEWriteLog(_T("Do Request exception========"));
		PrintException(pException,sRequest.GetBuffer());
		sRequest.ReleaseBuffer();
		pException->Delete();
	}

ExitConn:
	if (pHttpFile)
	{
		pHttpFile->Close();
		delete pHttpFile;
	}

	if (pHttpConnection)
	{
		pHttpConnection->Close();
		delete pHttpConnection;
	}

	if (pHttpSession)
	{
		pHttpSession->Close();
		delete pHttpSession;
	}

	LeaveCriticalSection(&cs4HttpConnection);

	return iRet;
}



int DeviceAgent::DoRequest(CString sRequest,IN OUT DataBuffer &data )
{
	EnterCriticalSection(&cs4HttpConnection);

	CInternetSession *pHttpSession = NULL;
	CHttpConnection* pHttpConnection = NULL;
	CHttpFile *pHttpFile = NULL;
	int iRet = PE_RET_FAIL;
	INTERNET_PORT nPort = PE_ANDROID_SVR_PORT;
	CString sContentLen;

	memset(&data,0,sizeof(data));

	//发起http请求
	try
	{
		pHttpSession = new CInternetSession(_T("PESessionGet"));
		SetInternetSessionOption(pHttpSession);
		pHttpConnection = pHttpSession->GetHttpConnection(m_szHostURL,nPort);
		DWORD HttpRequestFlags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;

		pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,sRequest, NULL, 1, NULL, (LPCTSTR)"1.0", HttpRequestFlags);
		if (!pHttpFile)
		{
			PEWriteLog(_T("pHttpFile is NULL"));
			goto ExitConn;
		}

		// Use direct write to posting field!
		CString strHeaders = _T("Accept: text/*\r\n");
		strHeaders += _T("User-Agent: YaSync\r\n");
		strHeaders += _T("Accept-Language: en-us\r\n");

		pHttpFile->AddRequestHeaders((LPCTSTR)strHeaders);
		pHttpFile->SendRequest();

		DWORD dwRet = 0;
		pHttpFile->QueryInfoStatusCode(dwRet);
		if (dwRet != HTTP_STATUS_OK)
		{
			goto ExitConn;
		}

		if (!pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, sContentLen))
		{
			PEWriteLog("http QueryInfo failed");
			goto ExitConn;
		}

		int iBufLen = _ttoi(sContentLen.GetBuffer());
		sContentLen.ReleaseBuffer();

		//先将http数据全部读出来，然后整体进行转换
		if(iBufLen <= 0)
		{
			PEWriteLog("CONTENT_LENGTH is 0");
			goto ExitConn;
		}

		InitDataBuf(data,iBufLen + 1);

		data.iRawBufferLen = pHttpFile->Read(data.pRawBufffer,iBufLen);
		if (data.iRawBufferLen < 1)
		{
			PEWriteLog(_T("Read failed========"));
			iRet = PE_RET_FAIL;
		}
		else
		{
			ConvertDataBuf(data);
			iRet = PE_RET_OK;
		}
	}
	catch(CInternetException * pException)
	{
		PEWriteLog(_T("Do Request exception========"));
		PrintException(pException,sRequest.GetBuffer());
		sRequest.ReleaseBuffer();
		pException->Delete();
	}

ExitConn:
	if (pHttpFile)
	{
		pHttpFile->Close();
		delete pHttpFile;
	}

	if (pHttpConnection)
	{
		pHttpConnection->Close();
		delete pHttpConnection;
	}

	if (pHttpSession)
	{
		pHttpSession->Close();
		delete pHttpSession;
	}

	LeaveCriticalSection(&cs4HttpConnection);

	return iRet;
}

MSXML2::IXMLDOMDocumentPtr DeviceAgent::GetXmlDoc(DataBuffer &data)
{
	HRESULT hr;
	MSXML2::IXMLDOMDocumentPtr pDoc;
	VARIANT_BOOL fSuccess;

	hr = pDoc.CreateInstance( __uuidof(MSXML2::DOMDocument));
	if ( !SUCCEEDED(hr) ) 
	{
		DWORD dwErr = GetLastError();
		PEWriteLog(_T("GetXmlDoc CreateInstance wrong"));

		return NULL;
	}

	BSTR bstrHttpData = ::SysAllocStringLen(data.pBuffer,data.iBufferLen);

	fSuccess = pDoc->loadXML(bstrHttpData);

	SysFreeString(bstrHttpData);
	if (fSuccess != VARIANT_TRUE)
	{
		PEWriteLog(_T("GetXmlDoc loadXML wrong"));
		return NULL;
	}

	return pDoc;
}


int DeviceAgent::GetDeviceInfoFromDevice(PE_DEV_INFO *pDevInfo)
{
	CString sReq = _T("SysInfo.xml?action=getsysinfo");

	DataBuffer data;

	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseDeviceInfoFromXML(data,pDevInfo);

	DeInitDataBuf(data);

	return iRet;

}

int DeviceAgent::ParseDeviceInfoFromXML(IN DataBuffer &data,IN OUT PE_DEV_INFO *pDevInfo)
{
	MSXML2::IXMLDOMNodeListPtr pSysSubNodes;

	MSXML2::IXMLDOMNodePtr	   pSysNode;
	MSXML2::IXMLDOMNodePtr	   pSysItem;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return PE_RET_FAIL;
	}

	pSysNode = pDoc->selectSingleNode("/SysInfoList");
	if (pSysNode == NULL)
	{
		return PE_RET_FAIL;
	}

	pSysSubNodes = pSysNode->GetchildNodes();
	if (pSysSubNodes == NULL)
	{
		return PE_RET_FAIL;
	}

	int m = 0;
	while (pSysItem = pSysSubNodes->Getitem(m++))
	{
		/*解析单个SmsThreadInfo节点*/
		BSTR NodeText;
		BSTR NodeName;

		NodeName = pSysItem->GetnodeName();
		CString sNodeName = BSTR2StrringThenFree(NodeName);

		pSysItem->get_text(&NodeText);
		CString sNodeValue = BSTR2StrringThenFree(NodeText);

		SysFreeString(NodeName);
		SysFreeString(NodeText);


		if (sNodeName == _T("ProductModel") )
		{
			URLDecoder(sNodeValue.GetBuffer(),pDevInfo->szPhoneModel,255);
		}
		else if (sNodeName == _T("ProductManufacturer") )
		{
			URLDecoder(sNodeValue.GetBuffer(),pDevInfo->szPhoneManufacturer,255);
		}
		else if (sNodeName == _T("BatteryLevel") )
		{
			pDevInfo->dwBatteryLevel = _tstoi64(sNodeValue.GetBuffer());
		}
		else if (sNodeName == _T("SDCardAvailableSpace") )
		{
			pDevInfo->dwSDCardAvailableSpace = _tstoi64(sNodeValue.GetBuffer());
		}
		else if (sNodeName == _T("SDCardTotalSpace") )
		{
			pDevInfo->dwSDCardTotalSpace= _tstoi64(sNodeValue.GetBuffer());
		}
		else if (sNodeName == _T("PhoneID") )
		{
			URLDecoder(sNodeValue.GetBuffer(),pDevInfo->szPhoneID,255);
		}
		else if (sNodeName == _T("AppVer") )
		{
			_tcscpy(pDevInfo->szAppVer,sNodeValue.GetBuffer());
		}
		else if (sNodeName == _T("AuthState") )
		{
			pDevInfo->iAuthState = _ttoi(sNodeValue.GetBuffer());
		}
		else if (sNodeName == _T("OS_VER"))
		{
			URLDecoder(sNodeValue.GetBuffer(),pDevInfo->szOSVer,255);
		}
		else if (sNodeName == _T("SDK_VER"))
		{
			pDevInfo->dwSDKVer = _ttoi(sNodeValue.GetBuffer());
		}

		sNodeValue.ReleaseBuffer();
	}

	return PE_RET_OK;
}

time_t GetDurationTime(TCHAR *tszDuration);
int DeviceAgent::ParseEvtListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pEvtNodes;
	MSXML2::IXMLDOMNodeListPtr pEvtSubNodes,pRmdNodes;
	MSXML2::IXMLDOMNodePtr	   pCalRootNode;
	MSXML2::IXMLDOMNodePtr	   pEvtNode,pRmdNode;
	MSXML2::IXMLDOMNodePtr	   pEvtItem;
	//MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	pCalRootNode = pDoc->selectSingleNode("/CalEvents");
	if (pCalRootNode == NULL)
	{
		return -1;
	}

	pEvtNodes = pCalRootNode->GetchildNodes();
	if (pEvtNodes == NULL)
	{
		return -1;
	}

	g_calEvtList.clear();

	int iMsgCount = 0;
	while (pEvtItem = pEvtNodes->Getitem(iMsgCount))
	{
		int n = 0;

		pEvtSubNodes = pEvtItem->GetchildNodes();

		CAL_EVT_ITEM *pCalendar = (CAL_EVT_ITEM *)calloc(1,sizeof(CAL_EVT_ITEM));

		while (pEvtNode = pEvtSubNodes->Getitem(n++))
		{
			/*解析单个SmsThreadInfo节点*/
			BSTR NodeText;
			BSTR NodeName;

			NodeName = pEvtNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pEvtNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);
			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("eventTimezone") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szTimeZone,255);
			}
			else if (sNodeName == _T("dtstart") )
			{
				if (sNodeValue != _T("+")){
					time_t ulTime = _tstoi64(sNodeValue.GetBuffer());
					JavaTime2Filetime(ulTime,&pCalendar->ftDTSTART);
				}
			}
			else if (sNodeName == _T("dtend") )
			{
				if (sNodeValue != _T("+")){
					time_t ulTime = _tstoi64(sNodeValue.GetBuffer());
					JavaTime2Filetime(ulTime,&pCalendar->ftDTEND);
				}
			}
			else if (sNodeName == _T("_id") )
			{
				pCalendar->ulEvtID = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("calendar_id") )
			{
				pCalendar->ulCalendarID = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("eventLocation") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szLocation,255);
			}
			else if (sNodeName == _T("title") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szTitle,255);
			}
			else if (sNodeName == _T("rrule") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szRRULE,1024);
				if (_tcslen(pCalendar->szRRULE) > 2 )
				{
					pCalendar->dwIsRecur = 1;
				}
			}
			else if (sNodeName == _T("allDay") )
			{
				pCalendar->dwAllDay = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("hasAlarm") )
			{
				pCalendar->dwHasAlarm = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("Reminder") )
			{
				pRmdNodes = pEvtNode->GetchildNodes();

				int idx = 0;
				while ( pRmdNode = pRmdNodes->Getitem(idx++))
				{
					BSTR NodeText;
					BSTR NodeName;
					NodeName = pRmdNode->GetnodeName();
					CString sNodeName = BSTR2StrringThenFree(NodeName);

					pRmdNode->get_text(&NodeText);
					CString sNodeValue = BSTR2StrringThenFree(NodeText);

					SysFreeString(NodeName);
					SysFreeString(NodeText);

					if (sNodeName == _T("minutes"))
					{
						pCalendar->dwReminderDelta = _ttoi(sNodeValue.GetBuffer());
						break;
					}

					sNodeValue.ReleaseBuffer();
				}
			}
			else if (sNodeName == _T("duration") )
			{
				TCHAR szDuration[255];
				URLDecoder(sNodeValue.GetBuffer(),szDuration,255);
				if (_tcslen(szDuration) > 1)
				{
					pCalendar->tDuration = GetDurationTime(szDuration);
					if (pCalendar->tDuration == 0)
					{
						PEWriteLog("Duration parse wrong");
						PEWriteLog(szDuration);
					}
				}
			}

			sNodeValue.ReleaseBuffer();
		}

		if (pCalendar->ftDTEND.dwHighDateTime == 0 && pCalendar->ftDTEND.dwLowDateTime == 0)
		{
			memcpy(&pCalendar->ftDTEND,&pCalendar->ftDTSTART,sizeof(FILETIME));
		}

#if 0
		if (_tcscmp(pCalendar->szTimeZone,_T("UTC")) == 0)
		{
			FILETIME ft;
			ft = pCalendar->ftDTSTART;
			FileTimeToLocalFileTime(&ft,&pCalendar->ftDTSTART);
			ft = pCalendar->ftDTEND;
			FileTimeToLocalFileTime(&ft,&pCalendar->ftDTEND);

			SYSTEMTIME st;
			FileTimeToSystemTime(&pCalendar->ftDTEND,&st);
		}
#endif

		if (pCalendar->dwAllDay)
		{
			GetMidnightTime(pCalendar->ftDTSTART);
			GetNextDay(pCalendar->ftDTEND);
		}

		if (pCalendar->tDuration != 0)
		{
			AddFileTimeSpan(pCalendar->ftDTEND,pCalendar->tDuration);
		}


		g_calEvtList.push_back(pCalendar);
		++iMsgCount;
	}

	return (int)g_calEvtList.size();
}

int DeviceAgent::GetEvtListFromDevice(time_t syncTime,TCHAR *szCalendarID)
{
	CString sReq;
	sReq.Format(_T("CalendarList.xml?action=getcalendarevent&startdate=%lld&calendarid=%s"),syncTime,szCalendarID);

	DataBuffer data;

	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseEvtListFromXML(data);

	DeInitDataBuf(data);

	return iRet;	
}


int DeviceAgent::GetCalendarListFromDevice()
{
	CString sReq = _T("CalendarList.xml?action=getcalendarlist");

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseCalendarListFromXML(data);

	DeInitDataBuf(data);

	if (iRet)
	{
		return PE_RET_OK;
	}

	return PE_RET_FAIL;
}

int DeviceAgent::ParseCalendarListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pEvtNodes;
	MSXML2::IXMLDOMNodeListPtr pEvtSubNodes,pRmdNodes;
	MSXML2::IXMLDOMNodePtr	   pCalRootNode;
	MSXML2::IXMLDOMNodePtr	   pEvtNode,pRmdNode;
	MSXML2::IXMLDOMNodePtr	   pEvtItem;
	//MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	pCalRootNode = pDoc->selectSingleNode("/Calendars");
	if (pCalRootNode == NULL)
	{
		return -1;
	}

	pEvtNodes = pCalRootNode->GetchildNodes();
	if (pEvtNodes == NULL)
	{
		return -1;
	}

	g_calendarList.clear();

	int iMsgCount = 0;
	while (pEvtItem = pEvtNodes->Getitem(iMsgCount))
	{
		int n = 0;

		pEvtSubNodes = pEvtItem->GetchildNodes();

		CAL_ITEM *pCalendar = (CAL_ITEM *)calloc(1,sizeof(CAL_ITEM));

		while (pEvtNode = pEvtSubNodes->Getitem(n++))
		{
			/*解析单个SmsThreadInfo节点*/
			BSTR NodeText;
			BSTR NodeName;

			NodeName = pEvtNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pEvtNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);
			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("account_name") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szAccountName,255);
			}
			else if (sNodeName == _T("account_type") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szAccountType,255);
			}
			else if (sNodeName == _T("_id") )
			{
				pCalendar->ulCalendarID = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("visible") )
			{
				pCalendar->visible = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("calendar_displayName") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szDisplayName,255);
			}
			else if (sNodeName == _T("name") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pCalendar->szName,255);
			}

			sNodeValue.ReleaseBuffer();
		}

		if (pCalendar->visible)
		{
			g_calendarList.push_back(pCalendar);
		}
		else
		{
			free(pCalendar);
		}

		++iMsgCount;
	}

	return (int)g_calendarList.size();
}


int DeviceAgent::GetMmsMimeDataFromDevice()
{
	list<MMS_ITEM*>::iterator it;
	for (it = g_mmsList.begin(); it != g_mmsList.end(); ++it)
	{ 
		MMS_ITEM *pMms = (MMS_ITEM*)*it;

		if (!pMms->pMimeData)
		{
			continue;
		}

		list<MIME_ITEM*>::iterator it1;
		for (it1 = pMms->pMimeData->begin(); it1 != pMms->pMimeData->end(); ++it1)
		{ 
			MIME_ITEM *pMime = (MIME_ITEM*)(*it1);
			if (_tcsstr(pMime->szMimeType,_T("image")) == NULL)
			{
				continue;
			}

			CString sReq;
			sReq.Format(_T("MmsList?action=getmmsmimedata&mimetype=%s&partid=%d"),pMime->szMimeType,pMime->ulPartID);

			DataBuffer data;
			if (PE_RET_OK != HttpRequest(sReq,data) || data.iRawBufferLen < 1)
			{
				DeInitDataBuf(data);
				continue;
			}

			pMime->iMimeDataLen = data.iRawBufferLen;
			pMime->MimeData = (byte*)malloc(data.iRawBufferLen);
			memcpy(pMime->MimeData,data.pRawBufffer,data.iRawBufferLen);
			DeInitDataBuf(data);
		}
	}

	return PE_RET_OK;
}



int DeviceAgent::GetContactPhotoFromDevice()
{
	list<CONTACT_ITEM*>::iterator it;
	for (it = g_contactList.begin(); it != g_contactList.end(); ++it)
	{ 
		if (!g_iCanSync)
		{
			break;
		}

		CONTACT_ITEM *p = (CONTACT_ITEM*)*it;

		if (p->ulPhotoID == 0)
		{
			continue;
		}

		CString sReq;
		sReq.Format(_T("ContactList?action=getcontactphoto&photoid=%u&"),p->ulPhotoID);

		DataBuffer data;
		if (PE_RET_OK != HttpRequest(sReq,data) || data.iRawBufferLen < 1)
		{
			DeInitDataBuf(data);
			continue;
		}

		p->iPhotoDataLen = data.iRawBufferLen;
		p->PhotoData = (byte*)malloc(data.iRawBufferLen);
		memcpy(p->PhotoData,data.pRawBufffer,data.iRawBufferLen);
		DeInitDataBuf(data);

	}

	return PE_RET_OK;
}


int DeviceAgent::ParseSysEvtFromXML(DataBuffer &data,DEVICE_EVT *pEvent)
{
	MSXML2::IXMLDOMNodeListPtr pEventNodes;
	//MSXML2::IXMLDOMNodeListPtr pEventSubNodes;
	MSXML2::IXMLDOMNodePtr	   pEventListNode;
	MSXML2::IXMLDOMNodePtr	   pEventItem;
	//MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return PE_RET_FAIL;
	}

	char *sTag = "/Events";
	pEventListNode = pDoc->selectSingleNode(sTag);
	if (pEventListNode == NULL)
	{
		return PE_RET_FAIL;
	}

	pEventNodes = pEventListNode->GetchildNodes();
	if (pEventNodes == NULL)
	{
		return PE_RET_FAIL;
	}

	int m = 0;

	while (pEventItem = pEventNodes->Getitem(m))
	{
		/*解析单个SmsThreadInfo节点*/
		BSTR NodeText,NodeName;
		NodeName = pEventItem->GetnodeName();
		CString sNodeName = BSTR2StrringThenFree(NodeName);

		pEventItem->get_text(&NodeText);
		CString sNodeValue = BSTR2StrringThenFree(NodeText);

		SysFreeString(NodeName);
		SysFreeString(NodeText);

		if (sNodeName == _T("EventCode"))
		{
			pEvent->iEvtCode = _ttoi(sNodeValue.GetBuffer());
		}
		else if (sNodeName == _T("Parameter") )
		{
			URLDecoder(sNodeValue.GetBuffer(),pEvent->szPara,255);
		}
		else if (sNodeName == _T("HasMore"))
		{
			pEvent->iHasMore = _ttoi(sNodeValue.GetBuffer());
		}

		sNodeValue.ReleaseBuffer();
		++m;
	}

	if (m != 3)
	{
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}



int DeviceAgent::QuerySysEvtFromDevice(DEVICE_EVT *pEvt)
{
	CString sReq = _T("SysInfo.xml?action=queryevt");

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	int iRet = ParseSysEvtFromXML(data,pEvt);
	DeInitDataBuf(data);

	return iRet;
}



int DeviceAgent::GetMmsListFromDevice(time_t syncTime)
{
	CString sReq;
	sReq.Format(_T("MmsList.xml?action=getmmslist&startdate=%lld"),syncTime);

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseMmsListFromXML(data);
	DeInitDataBuf(data);
	return iRet;
}

int DeviceAgent::ParseMmsListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pMmsNodes;
	MSXML2::IXMLDOMNodeListPtr pMmsSubNodes;
	MSXML2::IXMLDOMNodePtr	   pMmsListNode;
	MSXML2::IXMLDOMNodePtr	   pMmsItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;
	MSXML2::IXMLDOMNodeListPtr pPartNodes;
	MSXML2::IXMLDOMNodePtr	   pPartNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	char *sTag = "/MMSList";
	pMmsListNode = pDoc->selectSingleNode(sTag);
	if (pMmsListNode == NULL)
	{
		return -1;
	}

	pMmsNodes = pMmsListNode->GetchildNodes();
	if (pMmsNodes == NULL)
	{
		return -1;
	}

	int m = 0;

	while (pMmsItem = pMmsNodes->Getitem(m))
	{
		int n = 0;
		pMmsSubNodes = pMmsItem->GetchildNodes();

		MMS_ITEM *pMMS = (MMS_ITEM*)calloc(1,sizeof(MMS_ITEM));
		while (pSubNode = pMmsSubNodes->Getitem(n++))
		{
			BSTR NodeText;
			BSTR NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("_id") )
			{
				pMMS->ulMsgID= _ttoi(sNodeValue.GetBuffer());
			}
			if (sNodeName == _T("thread_id") )
			{
				pMMS->ulThreadID = _ttoi(sNodeValue.GetBuffer());
			}
			if (sNodeName == _T("date") )
			{
				time_t ulTime = _tstoi64(sNodeValue.GetBuffer());
				UnixTimeToFileTime(ulTime,&pMMS->ftRecvTime);
			}
			if (sNodeName == _T("msg_box") )
			{
				pMMS->ulMsgType= _ttoi(sNodeValue.GetBuffer());
			}
			if (sNodeName == _T("read") )
			{
				pMMS->ulReadFlag= _ttoi(sNodeValue.GetBuffer());
			}
			if (sNodeName == _T("sub") )
			{
				if (sNodeValue.GetLength() > 0)
				{
					pMMS->pszBody = (TCHAR*)calloc(sNodeValue.GetLength() * 2,sizeof(TCHAR));
					URLDecoder(sNodeValue.GetBuffer(),pMMS->pszBody,sNodeValue.GetLength() * 2);
				}
			}
			if (sNodeName == _T("PhoneNo") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pMMS->tszPhone,255);
			}
			if (sNodeName == _T("Name") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pMMS->tszName,255);
			}
			else if (sNodeName == _T("Parts") )
			{
				pPartNodes = pSubNode->GetchildNodes();
				int iPartsCount = 0;

				MIME_ITEM *p = (MIME_ITEM*)calloc(1,sizeof(MIME_ITEM));
				while ( pPartNode = pPartNodes->Getitem(iPartsCount++))
				{
					BSTR NodeText;
					BSTR NodeName;
					NodeName = pPartNode->GetnodeName();
					CString sNodeName = BSTR2StrringThenFree(NodeName);

					pPartNode->get_text(&NodeText);
					CString sNodeValue = BSTR2StrringThenFree(NodeText);

					SysFreeString(NodeName);
					SysFreeString(NodeText);

					if (sNodeName == _T("_id"))
					{
						p->ulPartID = _ttoi(sNodeValue.GetBuffer());
					}
					else if (sNodeName == _T("ct"))
					{
						URLDecoder(sNodeValue.GetBuffer(),p->szMimeType,255);
					}
					else if (sNodeName == _T("text"))
					{
						int iLen = sNodeValue.GetLength() * 4;
						p->szText = (TCHAR*)calloc(iLen,2);
						URLDecoder(sNodeValue.GetBuffer(),p->szText,iLen);
					}
					else if (sNodeName == _T("name"))
					{
						URLDecoder(sNodeValue.GetBuffer(),p->szMimeName,255);
					}

					sNodeValue.ReleaseBuffer();
				}

				if (pMMS->pMimeData == NULL)
				{
					pMMS->pMimeData = new list<MIME_ITEM*>;
					pMMS->pMimeData->clear();
				}

				pMMS->pMimeData->push_back(p);

			}

			sNodeValue.ReleaseBuffer();
		}

		g_mmsList.push_back(pMMS);
		++m;
	}

	return m;

}


void DeviceAgent::SyncMedia(TCHAR *szFolderPath, int iMediaType)
{
	PEWriteLog(_T("Enter"));

	_tmkdir(szFolderPath);

	TCHAR *szAction = _T("Media.xml?action=getimagecount");
	int iSyncType = PE_SYNC_IMAGE;

	switch (iMediaType)
	{
	case MEDIA_TYPE_VIDEO:
		{
			szAction = _T("Media.xml?action=getvideocount");
			iSyncType = PE_SYNC_VIDEO;
			break;
		}
	case MEDIA_TYPE_AUDIO:
		{
			szAction = _T("Media.xml?action=getaudiocount");
			iSyncType = PE_SYNC_AUDIO;
			break;
		}
	case MEDIA_TYPE_IMAGE:
		{
			szAction = _T("Media.xml?action=getimagecount");
			iSyncType = PE_SYNC_IMAGE;
			break;
		}
	}


	int iItemCount = GetItemCountFromDevice(szAction);
	if (iItemCount <= 0)
	{
		PEWriteLog(_T("No Image found on device"));
		return;
	}

	int iFrom = 0;
	int iTo = EA_RESPONSE_LIST_SIZE;

	int iListCount = 0;

	PECore::SendSyncState2UI(iSyncType,100,0);

	do
	{
		iListCount = GetMediaListFromDevice(iFrom,iTo,iMediaType);
		if (iListCount < 1)
		{
			break;
		}

		list<MEDIA_ITEM*>::iterator it;
		for (it = g_mediaList.begin(); it != g_mediaList.end(); ++it)
		{ 
			MEDIA_ITEM *pMedia = (MEDIA_ITEM*)*it;

			CString sFileName;
			sFileName.Format(_T("%s\\%s"),szFolderPath,pMedia->szName);

			FILE *pFile = _tfopen(sFileName,_T("r"));
			if (pFile)
			{
				//already exist
				free(pMedia->szURL);
				free(pMedia);
				fclose(pFile);
				continue;
			}

			//get image file from device
			CString sReq;
			sReq.Format(_T("Downloadfile?action=getfile&file=%s"),pMedia->szURL);

			free(pMedia->szURL);

			if (!g_iCanSync)
			{
				free(pMedia);
				break;
			}

			//save to disk
			pFile = _tfopen(sFileName,_T("a+b"));
			if (!pFile)
			{
				PEWriteLog(_T("Can not create file to save image"));
				free(pMedia);
				break;
			}

			//download from device and save to file
			int iRet = Download(sReq,pFile);
			fclose(pFile);

			if (iRet != PE_RET_OK)
			{
				//delete file
				DeleteFile(sFileName);
			}

			free(pMedia);
			PECore::SetSyncStat(iSyncType);
			PECore::SendSyncState2UI(iSyncType,iItemCount,++iFrom);
		}

		iFrom = iTo;
		iTo += EA_RESPONSE_LIST_SIZE;

	}while (1);

	g_mediaList.clear();

	PECore::SendSyncState2UI(iSyncType,100,100);
	return;

}

int DeviceAgent::GetMediaListFromDevice(int iFrom,int iTo,int iMediaType)
{
	CString sReq;

	if (iMediaType == MEDIA_TYPE_IMAGE)
	{
		sReq.Format(_T("Media.xml?action=getimagelist&from=%d&to=%d"),iFrom,iTo);
	}
	else if (iMediaType == MEDIA_TYPE_AUDIO)
	{
		sReq.Format(_T("Media.xml?action=getaudiolist&from=%d&to=%d"),iFrom,iTo);
	}
	else if (iMediaType == MEDIA_TYPE_VIDEO)
	{
		sReq.Format(_T("Media.xml?action=getvideolist&from=%d&to=%d"),iFrom,iTo);
	}

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseMediaListFromXML(data, iMediaType);
	DeInitDataBuf(data);
	return iRet;
}

int DeviceAgent::ParseMediaListFromXML(DataBuffer &data,int iMediaType)
{
	MSXML2::IXMLDOMNodeListPtr pMediaNodes;
	MSXML2::IXMLDOMNodeListPtr pMediaSubNodes;
	MSXML2::IXMLDOMNodePtr	   pMediaListNode;
	MSXML2::IXMLDOMNodePtr	   pMediaItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	char *sTag = "/MediaList";
	pMediaListNode = pDoc->selectSingleNode(sTag);
	if (pMediaListNode == NULL)
	{
		return -1;
	}

	pMediaNodes = pMediaListNode->GetchildNodes();
	if (pMediaNodes == NULL)
	{
		return -1;
	}

	int m = 0;
	g_mediaList.clear();

	while (pMediaItem = pMediaNodes->Getitem(m))
	{
		int n = 0;
		pMediaSubNodes = pMediaItem->GetchildNodes();
		ULONG64 ulDuration = 0;

		MEDIA_ITEM *pMedia = (MEDIA_ITEM*)calloc(1,sizeof(MEDIA_ITEM));
		//pMedia->dwType = iMediaType;

		while (pSubNode = pMediaSubNodes->Getitem(n++))
		{
			/*解析单个Media节点*/
			BSTR NodeText,NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("Path"))
			{
				pMedia->szURL = (TCHAR*)calloc(sizeof(TCHAR),sNodeValue.GetLength() + 1);
				_tcscpy(pMedia->szURL,sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("DisplayName"))
			{
				_tcscpy(pMedia->szName,sNodeValue.GetBuffer());
			}

			sNodeValue.ReleaseBuffer();
		}


		g_mediaList.push_back(pMedia);
		++m;
	}

	return m;

}



int DeviceAgent::SendSMS(TCHAR *szPhones,TCHAR *szContent)
{
	CString sReq = _T("action=sendsms&receiver=");
	sReq += szPhones;
	sReq += _T("&content=");
	sReq += szContent;

	PEWriteLog(_T("Post SMS Data start"));

	DataBuffer data;
	if (PE_RET_OK != DeviceAgent::HttpPost(_T("SmsList.xml"),sReq.GetBuffer(),data))
	{
		sReq.ReleaseBuffer();
		DeInitDataBuf(data);
		PEWriteLog(_T("Post SMS Data failed"));
		return PE_RET_FAIL;
	}

	PEWriteLog(_T("Post SMS Data ok"));
	DeInitDataBuf(data);
	sReq.ReleaseBuffer();

	return PE_RET_OK;
}



int DeviceAgent::GetSMSListFromDevice(time_t fromDate)
{
	CString sReq;
	sReq.Format(_T("SmsList.xml?action=getsmslist&startdate=%lld"),fromDate);

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseSMSListFromXML(data);
	DeInitDataBuf(data);
	return iRet;
}


void DeviceAgent::SyncCallLog()
{
	PEWriteLog(_T("Enter"));

	CallLogHelper *pHelper = new CallLogHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("Call Log sync failed:failed to open folder for CallLog"),0);
		delete pHelper;
		return;
	}

	FILETIME ftSyncStartFrom = {0};

	ULONG ulMaxID = GetMaxCallIDFromDevice();
	if (ulMaxID == 0)
	{
		PEWriteLog(_T("SyncCallLog ulMaxID is 0"));
		delete pHelper;
		return;
	}

	ULONG ulStartID = 0;

	PECore::SendSyncState2UI(PE_SYNC_CALL,100,0);

	time_t syncTime = 0;
	if (PE_RET_OK == pHelper->GetLastSyncTime(ftSyncStartFrom))
	{
		syncTime = FileTime2JavaTime(&ftSyncStartFrom);

		//skip last item already synced
		++syncTime;
	}

	do
	{
		int iCount = GetCallLogListFromDevice(syncTime);
		if (iCount < 1 )
		{
			//暂时处理为遇到错误直接返回
			break;
		}

		list<CALL_LOG_ITEM*>::iterator it;

		for (it = g_callList.begin(); it != g_callList.end(); ++it)
		{ 
			CALL_LOG_ITEM *pCall = (CALL_LOG_ITEM*)*it;
			if (PECompareFileTime(pCall->ftStartTime,ftSyncStartFrom) == PE_RET_CMP_GREATER)
			{
				ftSyncStartFrom = pCall->ftStartTime;
			}

			if (pCall->ulCallID < 1)
			{
				free(pCall);
				continue;
			}

			if (ulStartID == 0)
			{
				ulStartID = pCall->ulCallID;
			}

			PECore::SendSyncState2UI(PE_SYNC_CALL,ulMaxID - ulStartID,pCall->ulCallID - ulStartID);

			if (FilterHelper::IsItemInFilterList(pCall->szNumberW) == PE_RET_OK)
			{
				free(pCall);
				continue;
			}

			pHelper->SaveCallLog2OutLook(*pCall);
			PECore::SetSyncStat(PE_SYNC_CALL);

			if (pCall->ulCallType != CALL_TYPE_MISSED_CALL)
			{
				free(pCall);
				continue;
			}

			CString sInfo;

			SYSTEMTIME stTime,stLocal;
			FileTimeToSystemTime(&pCall->ftStartTime, &stTime);
			SystemTimeToTzSpecificLocalTime(NULL, &stTime, &stLocal);

			TCHAR szTime[255];
			_stprintf(szTime,_T("at  %0d-%02d  %02d:%02d:%02d"),stLocal.wMonth,stLocal.wDay,stLocal.wHour,stLocal.wMinute,stLocal.wSecond);
			if (_tcslen(pCall->szNameW))
			{
				sInfo.Format(_T("Missed call from %s %s"),pCall->szNameW,szTime);
			}
			else if (_tcslen(pCall->szNumberW))
			{
				sInfo.Format(_T("Missed call from %s %s"),pCall->szNumberW,szTime);
			}
			else
			{
				sInfo.Format(_T("Missed Call found %s"),szTime);
			}

			ShowTrayInfo( (LPWSTR)(LPCWSTR)sInfo);

			free(pCall);
		}

		g_callList.clear();

		if (iCount < EA_RESPONSE_LIST_SIZE)
		{
			//last list
			break;
		}

		syncTime = FileTime2JavaTime(&ftSyncStartFrom);
		++syncTime;
	}
	while (g_iCanSync);

	delete pHelper;

	PECore::SendSyncState2UI(PE_SYNC_CALL,100,100);

	return;
}



int DeviceAgent::SaveCalendar2Phone(CAL_EVT_ITEM *pCal)
{
	CString sReq;
	sReq.Format(_T("action=addcalendarevent&calendarid=%d"),pCal->ulCalendarID);

	CString sName;
	TCHAR *szTitle = URLEncodeW( pCal->szTitle,255);
	TCHAR *szDesc = URLEncodeW( pCal->szDesc,255);
	TCHAR *szTimeZone = URLEncodeW( pCal->szTimeZone,255);
	TCHAR *szLocation = URLEncodeW( pCal->szLocation,255);
	TCHAR *szRRULE = URLEncodeW( pCal->szRRULE,255);

	CString sTmp;
	sTmp.Format(_T("&title=%s&desc=%s&timezone=%s&location=%s&rrule=%s"),szTitle,szDesc,szTimeZone,szLocation,szRRULE);
	free(szTitle);
	free(szDesc);
	free(szTimeZone);
	free(szLocation);
	free(szRRULE);

	sReq += sTmp;

	time_t startTime = FileTime2JavaTime(&pCal->ftDTSTART);
	time_t endTime = FileTime2JavaTime(&pCal->ftDTEND);

	sTmp.Format(_T("&starttime=%lld&endtime=%lld&duration=%llu&allday=%d&hasalarm=%d&reminderdelta=%d"),
		startTime,endTime, pCal->tDuration, pCal->dwAllDay, pCal->dwHasAlarm, pCal->dwReminderDelta);
	sReq += sTmp;

	DataBuffer data;
	//PEWriteLog(sReq.GetBuffer());
	if (PE_RET_OK != DeviceAgent::HttpPost(_T("CalendarList.xml"),sReq.GetBuffer(),data))
	{
		sReq.ReleaseBuffer();
		DeInitDataBuf(data);
		PEWriteLog(_T("SaveCalendar2Phone failed2"));
		return PE_RET_FAIL;
	}

	int iCode = ParseRetCodeXML(data);
	if (iCode != 0)
	{
		CString sInfo;
		sInfo.Format(_T("SaveCalendar2Phone,phone response with code %d"),iCode);
		//PEWriteLog(sInfo.GetBuffer());
		sInfo.ReleaseBuffer();
	}

	DeInitDataBuf(data);
	sReq.ReleaseBuffer();

	return PE_RET_OK;
}


int DeviceAgent::RestoreContact(CONTACT_ITEM  *pCal,int &iPhoneRetCode)
{
	CString sReq = _T("action=insertcontact&");

	CString sID = _T("");
	sID.Format(_T("id=%d"),pCal->ulContactID);
	sReq += sID;

	CString sName;
	TCHAR *szFamilyName = URLEncodeW( pCal->szFamilyName,255);
	TCHAR *szMiddleName = URLEncodeW( pCal->szMiddleName,255);
	TCHAR *szGivenName = URLEncodeW( pCal->szGivenName,255);
	TCHAR *szDisplayName = URLEncodeW( pCal->szDisplayName,255);
	sName.Format(_T("&familyname=%s&middlename=%s&givenname=%s&displayname=%s"),szFamilyName,szMiddleName,szGivenName,szDisplayName);
	free(szFamilyName);
	free(szMiddleName);
	free(szGivenName);
	free(szDisplayName);
	sReq += sName;

	CString sPhones = _T("&cphone=");
	if ( pCal->pPhones)
	{
		list<PhoneItem*>::iterator it1;
		for (it1 =  pCal->pPhones->begin(); it1 !=  pCal->pPhones->end(); ++it1)
		{ 
			PhoneItem *p = (PhoneItem*)(*it1);
			CString sPhoneItem;
			TCHAR *szPhoneNo = URLEncodeW(p->szPhoneNo,255);
			sPhoneItem.Format(_T("%d:%s;"),p->iPhoneType,szPhoneNo);
			free(szPhoneNo);

			sPhones += sPhoneItem;
		}
	}
	sReq += sPhones;

	CString sMailList=_T("&cmail=");
	if ( pCal->pEMails)
	{
		list<EMailItem*>::iterator it;
		for (it =  pCal->pEMails->begin(); it !=  pCal->pEMails->end(); ++it)
		{ 
			EMailItem*p = (EMailItem*)*it;
			CString sMailItem;
			TCHAR *szEmail = URLEncodeW(p->szEmail,255);
			sMailItem.Format(_T("%d:%s;"),p->iType,szEmail);
			free(szEmail);
			sMailList += sMailItem;
		}
	}
	sReq += sMailList;

	CString sSiteList=_T("&csite=");
	if ( pCal->pSites)
	{
		list<WebSiteItem*>::iterator it;
		for (it =  pCal->pSites->begin(); it !=  pCal->pSites->end(); ++it)
		{ 
			WebSiteItem*p = (WebSiteItem*)*it;
			CString sSiteItem;
			TCHAR *szURL = URLEncodeW(p->szURL,1023);
			sSiteItem.Format(_T("%d[%s}"),p->iType,szURL);
			free(szURL);
			sSiteList += sSiteItem;
		}
	}
	sReq += sSiteList;

	CString sOrgList;
	TCHAR *szOrgCompany = URLEncodeW( pCal->szOrgCompany,255);
	TCHAR *szOrgTitle = URLEncodeW( pCal->szOrgTitle,255);
	sOrgList.Format(_T("&cOrg=company:%s;title:%s"),szOrgCompany,szOrgTitle);
	free(szOrgCompany);
	free(szOrgTitle);
	sReq += sOrgList;

	CString sAddrList=_T("&cAddr=");
	if ( pCal->pAddrs)
	{
		list<AddrItem*>::iterator it;
		for (it =  pCal->pAddrs->begin(); it !=  pCal->pAddrs->end(); ++it)
		{ 
			AddrItem*p = (AddrItem*)*it;
			CString sAddrItem;
			TCHAR *szAddrCity = URLEncodeW(p->szAddrCity,255);
			TCHAR *szAddrFormatAddress = URLEncodeW(p->szAddrFormatAddress,255);
			TCHAR *szAddrPostCode = URLEncodeW(p->szAddrPostCode,255);
			TCHAR *szAddrRegion = URLEncodeW(p->szAddrRegion,255);
			TCHAR *szAddrStreet = URLEncodeW(p->szAddrStreet,255);
			TCHAR *szCountry  = URLEncodeW(p->szCountry,255);
			sAddrItem.Format(_T("%d:city=%s;formataddr=%s;postcode=%s;region=%s;street=%s;country=%s;;"),p->iAddrType,szAddrCity,szAddrFormatAddress,szAddrPostCode,szAddrRegion,szAddrStreet,szCountry);
			sAddrList += sAddrItem;

			free(szAddrCity);
			free(szAddrFormatAddress);
			free(szAddrPostCode);
			free(szAddrRegion);
			free(szAddrStreet);
			free(szCountry);
		}
	}
	sReq += sAddrList;

	DataBuffer data;
	if (PE_RET_OK != DeviceAgent::HttpPost(_T("ContactList.xml"),sReq.GetBuffer(),data))
	{
		sReq.ReleaseBuffer();
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	iPhoneRetCode = ParseRetCodeXML(data);

	/*CString sInfo;
	sInfo.Format(_T("SaveContact2Phone,phone response with code %d"),iPhoneRetCode);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();*/

	DeInitDataBuf(data);
	sReq.ReleaseBuffer();

	return PE_RET_OK;
}

int DeviceAgent::Export(int iRestoreType,TCHAR *szFolderID)
{
	int iRet = PE_RET_OK;
	COutLookHelper *pHelper = NULL;
	switch (iRestoreType)
	{
	case PE_ACT_SAVE_SMS_2_PHONE:
		{
			pHelper = new SmsHelper();
			break;
		}
	case PE_ACT_SAVE_CALL_LOG_2_PHONE:
		{
			pHelper = new CallLogHelper();
			break;
		}
	case PE_ACT_SAVE_CONTACT_2_PHONE:
		{
			pHelper = new ContactsHelper();
			break;
		}
	}

	if (pHelper)
	{
		BOOL bRet = FALSE;
		if (szFolderID)
		{
			bRet = pHelper->openFolderByID(szFolderID);	
		}
		else
		{
			bRet = pHelper->openFolder();
		}

		if (bRet)
		{
			iRet = pHelper->Export();
		}
		else
		{
			::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("Restore failed:failed to open folder"),0);
		}
		delete pHelper;
	}

	return 0;
}


void DeviceAgent::SyncSms(int iSyncType)
{
	ULONG ulLastFromID = 0;
	FILETIME ftSyncStartFrom = {0};

	ULONG ulMaxID = GetMaxSMSIDFromDevice();
	if (ulMaxID == 0)
	{
		PEWriteLog(_T("SyncSms ulMaxID is 0"));
		return;
	}

	PECore::SendSyncState2UI(PE_SYNC_SMS,100,0);

	ULONG ulStartID = 0;
	time_t syncTime = 0;

	SmsHelper *pHelper = new SmsHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("SMS sync failed:failed to open folder"),0);
		delete pHelper;
		return;
	}

	PEWriteLog(_T("Now Get Last Sync Time"));

	if (PE_RET_OK == pHelper->GetLastSyncTime(ftSyncStartFrom))
	{
		syncTime = FileTime2JavaTime(&ftSyncStartFrom);

		//skip last item already synced
		++syncTime;
	}

	PEWriteLog(_T("Now Start Sync SMS"));

	do
	{
		int iCount = GetSMSListFromDevice(syncTime);
		if (iCount < 1 )
		{
			break;
		}

		list<SMS_ITEM*>::iterator it;

		for (it = g_smsList.begin(); it != g_smsList.end(); ++it)
		{ 
			SMS_ITEM *pSms = (SMS_ITEM*)*it;

			if (PECompareFileTime(pSms->ftRecvTime,ftSyncStartFrom) == PE_RET_CMP_GREATER)
			{
				ftSyncStartFrom = pSms->ftRecvTime;
			}

			if (pSms->ulMsgID < 1)
			{
				free(pSms);
				continue;
			}

			if (ulStartID == 0)
			{
				ulStartID = pSms->ulMsgID;
			}

			if (!pSms->pszBody)
			{
				free(pSms);
				continue;
			}

			PECore::SendSyncState2UI(PE_SYNC_SMS,ulMaxID - ulStartID,pSms->ulMsgID - ulStartID);

			if (FilterHelper::IsItemInFilterList(pSms->tszPhone) == PE_RET_OK)
			{
				free(pSms->pszBody);
				free(pSms);
				continue;
			}

			pHelper->SaveSMS2OutLook(*pSms);
			PECore::SetSyncStat(PE_SYNC_SMS);

#ifndef _DEBUG
			extern DWORD g_dwShowSMSReprot;
			if (g_dwShowSMSReprot == 0 || iSyncType != PE_SYNC_SMS || pSms->ulMsgType != SMS_TYPE_RECEIVE)
			{
				free(pSms->pszBody);
				free(pSms);
				continue;
			}
#endif

			if (pSms->ulMsgType == SMS_TYPE_RECEIVE && (pSms->ulReadFlag == SMS_READ_FLAG_UNREAD) )
			{
				CString sInfo;
				if (_tcslen(pSms->tszName))
				{
					sInfo.Format(_T("SMS from %s:\r\n %s"),pSms->tszName,pSms->pszBody);
				}
				else if (_tcslen(pSms->tszPhone))
				{
					sInfo.Format(_T("SMS from %s:\r\n %s"),pSms->tszPhone,pSms->pszBody);
				}
				else
				{
					sInfo.Format(_T("SMS received:\r\n %s"),pSms->tszPhone,pSms->pszBody);
				}
				TCHAR szReply[255];
				_stprintf(szReply,_T("%s <%s>"),pSms->tszName,pSms->tszPhone);
				PECore::ShowTrayDlg(sInfo.GetBuffer(),TRAY_MSG_REPLY_SMS,szReply);
			}

			free(pSms->pszBody);
			free(pSms);
		}

		g_smsList.clear();

		if (iCount < EA_RESPONSE_LIST_SIZE)
		{
			//last list
			break;
		}

		syncTime = FileTime2JavaTime(&ftSyncStartFrom);
		++syncTime;

	}
	while (g_iCanSync);

	PEWriteLog(_T("Sync SMS done"));

	PECore::SendSyncState2UI(PE_SYNC_SMS,100,100);

	delete pHelper;
}



int DeviceAgent::ParseSMSListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pSmsNodes;
	MSXML2::IXMLDOMNodeListPtr pSmsSubNodes;
	MSXML2::IXMLDOMNodePtr	   pChatListNode;
	MSXML2::IXMLDOMNodePtr	   pSmsItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	pChatListNode = pDoc->selectSingleNode("/SmsList");
	if (pChatListNode == NULL)
	{
		return -1;
	}

	pSmsNodes = pChatListNode->GetchildNodes();
	if (pSmsNodes == NULL)
	{
		return -1;
	}

	g_smsList.clear();

	int iMsgCount = 0;
	while (pSmsItem = pSmsNodes->Getitem(iMsgCount))
	{
		int n = 0;

		pSmsSubNodes = pSmsItem->GetchildNodes();

		SMS_ITEM *pSms = (SMS_ITEM *)calloc(1,sizeof(SMS_ITEM));

		while (pSubNode = pSmsSubNodes->Getitem(n++))
		{
			/*解析单个SmsThreadInfo节点*/
			BSTR NodeText;
			BSTR NodeName;

			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("thread_id") )
			{
				pSms->ulThreadID = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("_id") )
			{
				pSms->ulMsgID = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("Name") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pSms->tszName,255);
			}
			else if (sNodeName == _T("address"))
			{
				URLDecoder(sNodeValue.GetBuffer(),pSms->tszPhone,255);
			}
			else if (sNodeName == _T("body") )
			{
				int iLen = sNodeValue.GetLength();
				if (iLen)
				{
					pSms->pszBody = (TCHAR*)calloc(iLen * 10,sizeof(TCHAR));
					URLDecoder(sNodeValue.GetBuffer(),pSms->pszBody,iLen);
				}
			}
			else if (sNodeName == _T("type") )
			{
				//类型 1是接收到的，2是发出的
				pSms->ulMsgType = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("date") )
			{
				time_t ulTime = _tstoi64(sNodeValue.GetBuffer());
				JavaTime2Filetime(ulTime,&pSms->ftRecvTime);
			}
			else if (sNodeName == _T("read"))
			{
				if (PECore::GetCurPhone()->dwSetReadFlag)
				{
					pSms->ulReadFlag = _ttoi(sNodeValue.GetBuffer());
				}
			}

			sNodeValue.ReleaseBuffer();
		}

		g_smsList.push_back(pSms);
		++iMsgCount;
	}

	return (int)g_smsList.size();
}


int DeviceAgent::GetCallLogListFromDevice(time_t fromDate)
{
	CString sReq;
	sReq.Format(_T("CallLog.xml?action=getcallloglist&startdate=%lld"),fromDate);
	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseCallLogListFromXML(data);
	DeInitDataBuf(data);
	return iRet;
}


int DeviceAgent::ParseCallLogListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pCallLogNodes;
	MSXML2::IXMLDOMNodeListPtr pCallLogSubNodes;
	MSXML2::IXMLDOMNodePtr	   pCallLogListNode;
	MSXML2::IXMLDOMNodePtr	   pCallLogItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	char *sTag = "/CallLogList";
	pCallLogListNode = pDoc->selectSingleNode(sTag);
	if (pCallLogListNode == NULL)
	{
		return -1;
	}

	pCallLogNodes = pCallLogListNode->GetchildNodes();
	if (pCallLogNodes == NULL)
	{
		return -1;
	}

	int m = 0;
	g_callList.clear();

	while (pCallLogItem = pCallLogNodes->Getitem(m))
	{
		int n = 0;
		pCallLogSubNodes = pCallLogItem->GetchildNodes();
		ULONG64 ulDuration = 0;

		CALL_LOG_ITEM *pCall = (CALL_LOG_ITEM*)calloc(1,sizeof(CALL_LOG_ITEM));
		pCall->dwNumberType = PHONE_TYPE_MOBILE;

		while (pSubNode = pCallLogSubNodes->Getitem(n++))
		{
			/*解析单个SmsThreadInfo节点*/
			BSTR NodeText,NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("type"))
			{
				pCall->ulCallType = _ttoi(sNodeValue.GetBuffer());;
			}
			else if (sNodeName == _T("_id") )
			{
				pCall->ulCallID = _ttoi(sNodeValue.GetBuffer());;
			}
			else if (sNodeName == _T("NAME") )
			{
				if (sNodeValue == _T("-2"))
				{
					_tcscpy(pCall->szNameW,_T("Private Call"));
				}
				else
				{
					URLDecoder(sNodeValue.GetBuffer(),pCall->szNameW,255);
				}
			}
			else if (sNodeName == _T("duration") )
			{
				ulDuration = _ttoi(sNodeValue.GetBuffer());;
			}
			else if (sNodeName == _T("number") )
			{
				if (sNodeValue == _T("-2"))
				{
					_tcscpy(pCall->szNumberW,_T("Private Number"));
				}
				else
				{
					URLDecoder(sNodeValue.GetBuffer(),pCall->szNumberW,255);
				}
			}
			else if (sNodeName == _T("date") )
			{
				time_t ulTime = _tstoi64(sNodeValue.GetBuffer());
				JavaTime2Filetime(ulTime,&pCall->ftStartTime);
			}
			else if (sNodeName == _T("numbertype") )
			{
				if (sNodeValue.GetLength() > 0)
				{
					pCall->dwNumberType = _ttoi(sNodeValue.GetBuffer());
					if (PHONE_TYPE_UNKONW == pCall->dwNumberType)
					{
						PEWriteLog(_T("Unknow number type found!"));
					}
				}
			}

			sNodeValue.ReleaseBuffer();
		}

		ULONG64 ulTime;
		ulDuration = ulDuration * ONE_SECOND;
		memcpy(&ulTime,&(pCall->ftStartTime),sizeof(FILETIME) );
		ulTime += ulDuration;
		memcpy(&(pCall->ftEndTime),&ulTime,sizeof(FILETIME));

		CheckCallItem(*pCall);
		g_callList.push_back(pCall);
		++m;
	}

	return m;
}



int DeviceAgent::GetSimContactListFromDevice(ULONG ulFromID)
{
	CString sReq;
	sReq.Format(_T("ContactList.xml?action=getsimcontactlist&from=%u"),ulFromID);
	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseSimContactListFromXML(data);
	DeInitDataBuf(data);
	return iRet;
}

int DeviceAgent::ParseSimContactListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pCallLogNodes;
	MSXML2::IXMLDOMNodeListPtr pCallLogSubNodes;
	MSXML2::IXMLDOMNodePtr	   pCallLogListNode;
	MSXML2::IXMLDOMNodePtr	   pCallLogItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;
	MSXML2::IXMLDOMNodeListPtr pSubPhoneNodes;
	MSXML2::IXMLDOMNodePtr	   pSubPhoneNode;
	MSXML2::IXMLDOMNodeListPtr pItemNodes;
	MSXML2::IXMLDOMNodePtr	   pItemNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	char *sTag = "/Contacts";
	pCallLogListNode = pDoc->selectSingleNode(sTag);
	if (pCallLogListNode == NULL)
	{
		return -1;
	}

	pCallLogNodes = pCallLogListNode->GetchildNodes();
	if (pCallLogNodes == NULL)
	{
		return -1;
	}

	int m = 0;
	FreeContactList();

	while (pCallLogItem = pCallLogNodes->Getitem(m))
	{
		int n = 0;
		pCallLogSubNodes = pCallLogItem->GetchildNodes();

		CONTACT_ITEM *pContact = (CONTACT_ITEM*)calloc(1,sizeof(CONTACT_ITEM));
		pContact->iIsSim = 1;
		while (pSubNode = pCallLogSubNodes->Getitem(n++))
		{
			BSTR NodeText;
			BSTR NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sCNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("_id") )
			{
				pContact->ulContactID = _ttoi(sCNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("name") )
			{
				URLDecoder(sCNodeValue.GetBuffer(),pContact->szDisplayName,255);
			}
			else if (sNodeName == _T("number") )
			{
				PhoneItem *p = (PhoneItem*)calloc(1,sizeof(PhoneItem));
				URLDecoder(sCNodeValue.GetBuffer(),p->szPhoneNo,255);
				p->iPhoneType = PHONE_TYPE_MOBILE;
				if (pContact->pPhones == NULL)
				{
					pContact->pPhones = new list<PhoneItem*>;
				}

				pContact->pPhones->push_back(p);
			}

			sCNodeValue.ReleaseBuffer();
		}

		g_contactList.push_back(pContact);
		++m;
	}

	return m;
}



int DeviceAgent::GetContactListFromDevice(ULONG ulFromID,TCHAR *szAccount)
{
	CString sReq;
	sReq.Format(_T("ContactList.xml?action=getcontactlist&from=%u&accountname=%s"),ulFromID,szAccount);
	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseContactListFromXML(data);
	DeInitDataBuf(data);
	return iRet;
}

int DeviceAgent::ParseContactListFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pCallLogNodes;
	MSXML2::IXMLDOMNodeListPtr pCallLogSubNodes;
	MSXML2::IXMLDOMNodePtr	   pCallLogListNode;
	MSXML2::IXMLDOMNodePtr	   pCallLogItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;
	MSXML2::IXMLDOMNodeListPtr pSubPhoneNodes;
	MSXML2::IXMLDOMNodePtr	   pSubPhoneNode;
	MSXML2::IXMLDOMNodeListPtr pItemNodes;
	MSXML2::IXMLDOMNodePtr	   pItemNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	char *sTag = "/Contacts";
	pCallLogListNode = pDoc->selectSingleNode(sTag);
	if (pCallLogListNode == NULL)
	{
		return -1;
	}

	pCallLogNodes = pCallLogListNode->GetchildNodes();
	if (pCallLogNodes == NULL)
	{
		return -1;
	}

	int m = 0;
	FreeContactList();

	while (pCallLogItem = pCallLogNodes->Getitem(m))
	{
		int n = 0;
		pCallLogSubNodes = pCallLogItem->GetchildNodes();

		CONTACT_ITEM *pContact = (CONTACT_ITEM*)calloc(1,sizeof(CONTACT_ITEM));
		pContact->iIsSim = 0;
		while (pSubNode = pCallLogSubNodes->Getitem(n++))
		{
			BSTR NodeText;
			BSTR NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sCNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("_id") )
			{
				pContact->ulContactID = _ttoi(sCNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("display_name") )
			{
				URLDecoder(sCNodeValue.GetBuffer(),pContact->szDisplayName,255);
			}
			else if (sNodeName == _T("photo_id"))
			{
				pContact->ulPhotoID = _ttoi(sCNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("Names") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;
				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					BSTR NodeText;
					BSTR NodeName;
					NodeName = pSubPhoneNode->GetnodeName();
					CString sNodeName = BSTR2StrringThenFree(NodeName);

					pSubPhoneNode->get_text(&NodeText);
					CString sNodeValue = BSTR2StrringThenFree(NodeText);

					SysFreeString(NodeName);
					SysFreeString(NodeText);

					if (sNodeName == _T("GIVEN_NAME"))
					{
						URLDecoder(sNodeValue.GetBuffer(),pContact->szGivenName,255);
					}
					else if (sNodeName == _T("FAMILY_NAME"))
					{
						URLDecoder(sNodeValue.GetBuffer(),pContact->szFamilyName,255);
					}
					else if (sNodeName == _T("MIDDLE_NAME"))
					{
						URLDecoder(sNodeValue.GetBuffer(),pContact->szMiddleName,255);
					}

					sNodeValue.ReleaseBuffer();
				}
			}
			else if (sNodeName == _T("Addrs") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;
				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					AddrItem *p = (AddrItem*)calloc(1,sizeof(AddrItem));
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("street"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szAddrStreet,255);
						}
						else if (sNodeName == _T("city"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szAddrCity,255);
						}
						else if (sNodeName == _T("region"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szAddrRegion,255);
						}
						else if (sNodeName == _T("postCode"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szAddrPostCode,255);
						}
						else if (sNodeName == _T("poBox"))
						{
							//URLDecoder(sNodeValue.GetBuffer(),p->szPoBox,255);
							//sNodeValue.ReleaseBuffer();
						}
						else if (sNodeName == _T("coutry"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szCountry,255);
						}
						else if (sNodeName == _T("type"))
						{
							p->iAddrType = _ttoi(sNodeValue.GetBuffer());

						}
						else if (sNodeName == _T("formatAddress"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szAddrFormatAddress,255);
						}

						sNodeValue.ReleaseBuffer();
					}

					if (pContact->pAddrs == NULL)
					{
						pContact->pAddrs = new list<AddrItem*>;
					}

					pContact->pAddrs->push_back(p);
				}
			}
			else if (sNodeName == _T("ORGs") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;
				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("company"))
						{
							URLDecoder(sNodeValue.GetBuffer(),pContact->szOrgCompany,255);
						}
						else if (sNodeName == _T("title"))
						{
							URLDecoder(sNodeValue.GetBuffer(),pContact->szOrgTitle,255);
						}

						sNodeValue.ReleaseBuffer();
					}
				}
			}
			else if (sNodeName == _T("WebSites") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;

				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					WebSiteItem *p = (WebSiteItem*)calloc(1,sizeof(WebSiteItem));
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("URL"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szURL,1023);
						}
						else if (sNodeName == _T("TYPE"))
						{
							p->iType = _ttoi(sNodeValue.GetBuffer());
						}

						sNodeValue.ReleaseBuffer();
					}

					if (pContact->pSites == NULL)
					{
						pContact->pSites = new list<WebSiteItem*>;
					}

					pContact->pSites->push_back(p);
				}
			}
			else if (sNodeName == _T("Birthday") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;

				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					WebSiteItem *p = (WebSiteItem*)calloc(1,sizeof(WebSiteItem));
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("Day"))
						{
							PEWriteLog(_T("Contact Birthday found:"));
							PEWriteLog(sNodeValue.GetBuffer());
						}

						sNodeValue.ReleaseBuffer();
					}
				}
			}
			else if (sNodeName == _T("EMails") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;

				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					EMailItem *p = (EMailItem*)calloc(1,sizeof(EMailItem));
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("ADDRESS"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szEmail,255);
						}
						else if (sNodeName == _T("TYPE"))
						{
							p->iType = _ttoi(sNodeValue.GetBuffer());
						}

						sNodeValue.ReleaseBuffer();
					}

					if (pContact->pEMails == NULL)
					{
						pContact->pEMails = new list<EMailItem*>;
					}

					pContact->pEMails->push_back(p);
				}
			}
			else if (sNodeName == _T("IMs") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;

				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					ImItem *p = (ImItem*)calloc(1,sizeof(ImItem));
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("DATA"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szIM,255);
						}
						else if (sNodeName == _T("PROTOCOL"))
						{
							p->iIMType = _ttoi(sNodeValue.GetBuffer());
						}

						sNodeValue.ReleaseBuffer();
					}

					if (pContact->pIMs == NULL)
					{
						pContact->pIMs = new list<ImItem*>;
					}

					pContact->pIMs->push_back(p);
				}
			}
			else if (sNodeName == _T("Phones") )
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;
				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					pItemNodes = pSubPhoneNode->GetchildNodes();

					int index = 0;
					PhoneItem *p = (PhoneItem*)calloc(1,sizeof(PhoneItem));
					while (	pItemNode = pItemNodes->Getitem(index++))	
					{
						BSTR NodeText;
						BSTR NodeName;
						NodeName = pItemNode->GetnodeName();
						CString sNodeName = BSTR2StrringThenFree(NodeName);

						pItemNode->get_text(&NodeText);
						CString sNodeValue = BSTR2StrringThenFree(NodeText);

						SysFreeString(NodeName);
						SysFreeString(NodeText);

						if (sNodeName == _T("NUMBER"))
						{
							URLDecoder(sNodeValue.GetBuffer(),p->szPhoneNo,255);
						}
						else if (sNodeName == _T("TYPE"))
						{
							p->iPhoneType = _ttoi(sNodeValue.GetBuffer());
						}

						sNodeValue.ReleaseBuffer();
					}

					if (ContactsHelper::IsPhoneTypeSupported(p->iPhoneType) == PE_RET_OK)
					{
						if (pContact->pPhones == NULL)
						{
							pContact->pPhones = new list<PhoneItem*>;
						}

						pContact->pPhones->push_back(p);
					}
					else
					{
						free(p);
					}
				}
			}
			else if (sNodeName == _T("in_visible_group") )
			{
				pContact->iVisible = _ttoi(sCNodeValue.GetBuffer());

			}
			else if (_T("BirthDay") == sNodeName)
			{

			}
			else if (_T("Notes") == sNodeName)
			{
				pSubPhoneNodes = pSubNode->GetchildNodes();
				int iItemCount = 0;
				while (	pSubPhoneNode = pSubPhoneNodes->Getitem(iItemCount++))
				{
					BSTR NodeText;
					BSTR NodeName;
					NodeName = pSubPhoneNode->GetnodeName();
					CString sNodeName = BSTR2StrringThenFree(NodeName);

					pSubPhoneNode->get_text(&NodeText);
					CString sNodeValue = BSTR2StrringThenFree(NodeText);

					SysFreeString(NodeName);
					SysFreeString(NodeText);

					if (sNodeName == _T("NoteInfo"))
					{
						int iLen = sNodeValue.GetLength() * 4;
						pContact->szNotes = (TCHAR*)calloc(iLen,1);
						URLDecoder(sNodeValue.GetBuffer(),pContact->szNotes,iLen/2);
					}


					sNodeValue.ReleaseBuffer();
				}
			}

			sCNodeValue.ReleaseBuffer();
		}

		g_contactList.push_back(pContact);
		++m;
	}

	return m;
}


void TestSyncContactFromFolder2Phone()
{
	ContactsHelper *pHelper = new ContactsHelper();
	//pHelper->openFolder();
	//pHelper->SyncAllContact2Phone();
	//pHelper->EmulateContact();

	delete pHelper;
}



void DeviceAgent::SyncSimContact()
{
	PEWriteLog(_T("Enter"));

	ULONG ulMaxID = GetMaxSimContactIDFromDevice();
	if (ulMaxID == 0)
	{
		PEWriteLog(_T("SyncSimContact ulMaxID is 0"));
		return;
	}

	ULONG ulStartID = 0;

	PECore::SendSyncState2UI(PE_SYNC_SIM_CONTACT,100,0);

	ContactsHelper *pHelper = new ContactsHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("SIM contacts sync failed:failed to open folder"),0);
		delete pHelper;
		return;
	}

	ULONG ulFromID = 0;

	do
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("ulFromID is %d"),ulFromID);
		PEWriteLog(szInfo);

		int iCount = GetSimContactListFromDevice(ulFromID);
		if (iCount < 1)
		{
			PEWriteLog(_T("iCount <= 0"));
			break;
		}

		list<CONTACT_ITEM*>::iterator it;
		for (it = g_contactList.begin(); it != g_contactList.end(); ++it)
		{ 
			if (!g_iCanSync)
			{
				break;
			}

			CONTACT_ITEM *pContact = (CONTACT_ITEM*)*it;
			if (ulFromID < pContact->ulContactID)
			{
				ulFromID = pContact->ulContactID;
			}

			pHelper->SaveContact2OutLook(pContact);
		}

		if (ulStartID == 0)
		{
			ulStartID = ulFromID;
		}

		PECore::SendSyncState2UI(PE_SYNC_SIM_CONTACT,ulMaxID - ulStartID,ulFromID - ulStartID);

		++ulFromID;

		FreeContactList();

		if (iCount < EA_RESPONSE_LIST_SIZE)
		{
			//last list
			break;
		}
	}
	while (g_iCanSync);

	PECore::SendSyncState2UI(PE_SYNC_SIM_CONTACT,100,100);

	delete pHelper;

	return;

}




void DeviceAgent::SyncContact(TCHAR *szAccount)
{
	PEWriteLog(_T("Enter"));

	ULONG ulMaxID = GetMaxContactIDFromDevice();
	if (ulMaxID == 0)
	{
		PEWriteLog(_T("SyncContact ulMaxID is 0"));
		return;
	}

	ULONG ulStartID = 0;

	PECore::SendSyncState2UI(PE_SYNC_CONTACT,100,0);

	ContactsHelper *pHelper = new ContactsHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("Contacts sync failed:failed to open folder"),0);
		delete pHelper;
		return;
	}

	ULONG ulFromID = 1;

	if (!szAccount)
	{
		szAccount = _T("");
	}

	do
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("ulFromID is %d"),ulFromID,szAccount);
		PEWriteLog(szInfo);

		int iCount = GetContactListFromDevice(ulFromID,szAccount);
		if (iCount < 1)
		{
			PEWriteLog(_T("iCount <= 0"));
			break;
		}

		GetContactPhotoFromDevice();

		list<CONTACT_ITEM*>::iterator it;
		for (it = g_contactList.begin(); it != g_contactList.end(); ++it)
		{ 
			if (!g_iCanSync)
			{
				break;
			}

			CONTACT_ITEM *pContact = (CONTACT_ITEM*)*it;
			if (pContact->ulContactID < 1)
			{
				PEWriteLog(_T("ulContactID < 0"));
				break;
			}

			if (ulFromID < pContact->ulContactID)
			{
				ulFromID = pContact->ulContactID;
			}

			if (!pContact->iVisible)
			{
				continue;
			}

			pHelper->SaveContact2OutLook(pContact);
		}

		if (ulStartID == 0)
		{
			ulStartID = ulFromID;
		}

		PECore::SendSyncState2UI(PE_SYNC_CONTACT,ulMaxID - ulStartID,ulFromID - ulStartID);

		++ulFromID;

		FreeContactList();

		if (iCount < EA_RESPONSE_LIST_SIZE)
		{
			//last list
			break;
		}
	}
	while (g_iCanSync);

	PECore::SendSyncState2UI(PE_SYNC_CONTACT,100,100);

	delete pHelper;

	return;

}



#if 0
void emulateTestCal()
{
	CAL_EVT_ITEM cal;
	memset(&cal,0,sizeof(cal));

	time_t ulTime = _tstoi64(_T("1365465600000"));

	JavaTime2Filetime(ulTime,&cal.ftDTSTART);

	memcpy(&cal.ftDTEND,&cal.ftDTSTART,sizeof(FILETIME));
	GetNextDay(cal.ftDTEND);

	cal.tDuration = GetDurationTime(_T("P3600S"));
	time_t span = cal.tDuration;
	if (span != 0)
	{
		AddFileTimeSpan(cal.ftDTEND,span);
	}

	FILETIME ft;
	ft = cal.ftDTSTART;
	LocalFileTimeToFileTime(&ft,&cal.ftDTSTART);
	ft = cal.ftDTEND;
	LocalFileTimeToFileTime(&ft,&cal.ftDTEND);

	cal.ulEvtID = 1;
	cal.ulCalendarID = 1;
	_tcscpy(cal.szLocation,_T("zte"));
	_tcscpy(cal.szTitle,_T("TUTC"));
	//_tcscpy(cal.szRRULE,_T("FREQ=YEARLY;WKST=SU"));

	cal.dwHasAlarm = 1;
	cal.dwReminderDelta = 17;
	cal.dwAllDay = 1;

	CalendarHelper *pHelper = new CalendarHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		delete pHelper;
		return;
	}

	//pHelper->IsCalendarExist(0,0,NULL,&cal);

	pHelper->SaveCalendar2OutLook(cal);

	/*
	if sync base on outlook
	insert a new contact on device and then delete the existing item
	finally update the contact id in outlook item
	*/
	/*=======================conflict handle end=================*/

	delete pHelper;
}
#endif


void DeviceAgent::SyncCalendar(TCHAR *szCalendarID,FILETIME ftSyncAfter)
{
	PEWriteLog(_T("Enter"));
	if (szCalendarID)
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("szCalendarID is %s"),szCalendarID);
	}

#if 0
	emulateTestCal();
	return;
#endif

	CalendarHelper *pHelper = new CalendarHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("Calendar sync failed:failed to open folder"),0);
		delete pHelper;
		return;
	}

	ULONG ulMaxID = GetMaxCalEvtIDFromDevice(szCalendarID);
	if (ulMaxID == 0)
	{
		PEWriteLog(_T("SyncCalendar ulMaxID is 0"));
		return;
	}

	PECore::SendSyncState2UI(PE_SYNC_CALENDAR,100,0);

	ULONG ulStartID = 0;

	FILETIME ftSyncStartFrom = {0};
	time_t syncTime = 0;
	if (PE_RET_OK == pHelper->GetLastSyncTime(szCalendarID,ftSyncStartFrom))
	{
		if (PECompareFileTime(ftSyncAfter,ftSyncStartFrom) == PE_RET_CMP_GREATER)
		{
			PEWriteLog(_T("use time from setting"));
			ftSyncStartFrom = ftSyncAfter;
		}

		syncTime = FileTime2JavaTime(&ftSyncStartFrom);

		//skip last item already synced
		++syncTime;

		CString sInfo;
		sInfo.Format(_T("last sync time:%lld"),syncTime);
		PEWriteLog(sInfo.GetBuffer());
		sInfo.ReleaseBuffer();
	}
	else
	{
		PEWriteLog(_T("Get last sync time failed"));
	}



	do
	{
		int iCount = GetEvtListFromDevice(syncTime,szCalendarID);
		if (iCount <= 0)
		{
			PEWriteLog(_T("Calendar iCount <= 0"));
			break;
		}

		list<CAL_EVT_ITEM*>::iterator it;
		for (it = g_calEvtList.begin(); it != g_calEvtList.end(); ++it)
		{ 
			CAL_EVT_ITEM *pCalendar = (CAL_EVT_ITEM*)*it;
			if (pCalendar->ulCalendarID < 1 || pCalendar->ulEvtID < 1)
			{
				PEWriteLog(_T("ulContactID < 0"));
				free(pCalendar);
				continue;
			}

			if (PECompareFileTime(pCalendar->ftDTSTART,ftSyncStartFrom) == PE_RET_CMP_GREATER)
			{
				ftSyncStartFrom = pCalendar->ftDTSTART;
			}

			if (ulStartID == 0)
			{
				ulStartID = pCalendar->ulEvtID;
			}

			PECore::SendSyncState2UI(PE_SYNC_CALENDAR,ulMaxID - ulStartID,pCalendar->ulEvtID - ulStartID);

			pHelper->SaveCalendar2OutLook(pCalendar);
			free(pCalendar);
		}

		g_calEvtList.clear();

		if (iCount < EA_RESPONSE_LIST_SIZE)
		{
			//last list
			break;
		}

		syncTime = FileTime2JavaTime(&ftSyncStartFrom);
		++syncTime;
	}
	while (g_iCanSync);

	delete pHelper;

	PECore::SendSyncState2UI(PE_SYNC_CALENDAR,100,100);

	return;

}


void DeviceAgent::SyncMMS()
{
	PEWriteLog(_T("SyncMMS"));

	MmsHelper *pHelper = new MmsHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("MMS sync failed:failed to open folder"),0);
		delete pHelper;
		return;
	}

	ULONG ulMaxID = GetMaxSMSIDFromDevice();
	if (ulMaxID == 0)
	{
		PEWriteLog(_T("SyncMMS ulMaxID is 0"));
		delete pHelper;
		return;
	}

	ULONG ulStartID = 0;

	PECore::SendSyncState2UI(PE_SYNC_MMS,100,0);

	FILETIME ftSyncStartFrom = {0};
	time_t syncTime = 0;
	if (PE_RET_OK == pHelper->GetLastSyncTime(ftSyncStartFrom))
	{
		syncTime = FileTimeToUnixTime(&ftSyncStartFrom);

		//skip last item already synced
		++syncTime;
	}

	do
	{
		int iCount = GetMmsListFromDevice(syncTime);
		if (iCount < 1 )
		{
			break;
		}

		GetMmsMimeDataFromDevice();

		list<MMS_ITEM*>::iterator it;
		for (it = g_mmsList.begin(); it != g_mmsList.end(); ++it)
		{ 
			MMS_ITEM *pMms = (MMS_ITEM*)*it;
			if (PECompareFileTime(pMms->ftRecvTime,ftSyncStartFrom) == PE_RET_CMP_GREATER)
			{
				ftSyncStartFrom = pMms->ftRecvTime;
			}

			if (pMms->ulMsgID < 1)
			{
				continue;
			}

			if (ulStartID == 0)
			{
				ulStartID = pMms->ulMsgID;
			}

			PECore::SendSyncState2UI(PE_SYNC_MMS,ulMaxID - ulStartID,pMms->ulMsgID - ulStartID);

			if (FilterHelper::IsItemInFilterList(pMms->tszPhone) == PE_RET_OK)
			{
				continue;
			}

			pHelper->SaveMMS2OutLook(pMms);
			PECore::SetSyncStat(PE_SYNC_MMS);
		}

		FreeMmsList();

		if (iCount < EA_RESPONSE_LIST_SIZE)
		{
			//last list
			break;
		}

		syncTime = FileTime2JavaTime(&ftSyncStartFrom);
		++syncTime;
	}
	while (g_iCanSync);

	PECore::SendSyncState2UI(PE_SYNC_MMS,100,100);

	delete pHelper;
}



ULONG DeviceAgent::GetMaxSMSIDFromDevice()
{
	CString sReq = _T("SmsList.xml?action=getmaxsmsid");

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return 0;
	}

	ULONG ulID = ParseMaxID(data);
	DeInitDataBuf(data);

	return ulID;
}



ULONG DeviceAgent::GetMaxCallIDFromDevice()
{
	CString sReq;
	sReq.Format(_T("CallLog.xml?action=getmaxcallid"));

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return 0;
	}

	ULONG ulID = ParseMaxID(data);
	DeInitDataBuf(data);

	return ulID;
}


ULONG DeviceAgent::GetMaxSimContactIDFromDevice()
{
	CString sReq = _T("ContactList.xml?action=getmaxsimcontactid");

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return 0;
	}

	ULONG ulID = ParseMaxID(data);
	DeInitDataBuf(data);

	return ulID;
}




ULONG DeviceAgent::GetMaxContactIDFromDevice()
{
	CString sReq = _T("ContactList.xml?action=getmaxcontactid");

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return 0;
	}

	ULONG ulID = ParseMaxID(data);
	DeInitDataBuf(data);

	return ulID;
}



ULONG DeviceAgent::GetMaxCalEvtIDFromDevice(TCHAR *szCalendarID)
{
	CString sReq;
	sReq.Format(_T("CalendarList.xml?action=getmaxcalendarevtid&calendarid=%s"),szCalendarID);

	DataBuffer data;

	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	ULONG ulID = ParseMaxID(data);
	DeInitDataBuf(data);

	return ulID;
}

int DeviceAgent::GetAccountsFromDevice()
{
	CString sReq = _T("SysInfo.xml?action=getaccounts");
	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iRet = ParseAccountsFromXML(data);
	DeInitDataBuf(data);
	if (iRet > 0)
	{
		return PE_RET_OK;
	}

	return PE_RET_FAIL;
}


int DeviceAgent::ParseAccountsFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pAccountNodes;
	MSXML2::IXMLDOMNodeListPtr pAccountSubNodes;
	MSXML2::IXMLDOMNodePtr	   pAccountListNode;
	MSXML2::IXMLDOMNodePtr	   pAccountItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return -1;
	}

	char *sTag = "/Accounts";
	pAccountListNode = pDoc->selectSingleNode(sTag);
	if (pAccountListNode == NULL)
	{
		return -1;
	}

	pAccountNodes = pAccountListNode->GetchildNodes();
	if (pAccountNodes == NULL)
	{
		return -1;
	}

	int m = 0;
	g_accountList.clear();

	while (pAccountItem = pAccountNodes->Getitem(m))
	{
		int n = 0;
		pAccountSubNodes = pAccountItem->GetchildNodes();
		ULONG64 ulDuration = 0;

		ACCOUNT_ITEM *pAccount = (ACCOUNT_ITEM*)calloc(1,sizeof(ACCOUNT_ITEM));

		while (pSubNode = pAccountSubNodes->Getitem(n++))
		{
			/*解析单个SmsThreadInfo节点*/
			BSTR NodeText,NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("Name"))
			{
				URLDecoder(sNodeValue.GetBuffer(),pAccount->szName,255);
			}
			else if (sNodeName == _T("Type") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pAccount->szType,255);
			}

			sNodeValue.ReleaseBuffer();
		}

		g_accountList.push_back(pAccount);
		++m;
	}

	return m;

}



list<SCHEDULE_SMS_ITEM*> DeviceAgent::GetScheduleSMSFromDevice()
{
	CString sReq = _T("SmsList.xml?action=getschedulesmslist");
	DataBuffer data;
	list<SCHEDULE_SMS_ITEM*> smsList;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return smsList;
	}

	smsList = ParseScheduleSMSFromXML(data);
	DeInitDataBuf(data);

	return smsList;
}


list<SCHEDULE_SMS_ITEM*> DeviceAgent::ParseScheduleSMSFromXML(DataBuffer &data)
{
	MSXML2::IXMLDOMNodeListPtr pAccountNodes;
	MSXML2::IXMLDOMNodeListPtr pAccountSubNodes;
	MSXML2::IXMLDOMNodePtr	   pAccountListNode;
	MSXML2::IXMLDOMNodePtr	   pAccountItem;
	MSXML2::IXMLDOMNodePtr	   pSubNode;

	list<SCHEDULE_SMS_ITEM*> smsList;

	MSXML2::IXMLDOMDocumentPtr pDoc = GetXmlDoc(data);
	if ( !pDoc )
	{
		return smsList;
	}

	char *sTag = "/SmsList";
	pAccountListNode = pDoc->selectSingleNode(sTag);
	if (pAccountListNode == NULL)
	{
		return smsList;
	}

	pAccountNodes = pAccountListNode->GetchildNodes();
	if (pAccountNodes == NULL)
	{
		return smsList;
	}

	int m = 0;

	while (pAccountItem = pAccountNodes->Getitem(m))
	{
		int n = 0;
		pAccountSubNodes = pAccountItem->GetchildNodes();

		SCHEDULE_SMS_ITEM *pSMS = (SCHEDULE_SMS_ITEM*)calloc(1,sizeof(SCHEDULE_SMS_ITEM));

		while (pSubNode = pAccountSubNodes->Getitem(n++))
		{
			/*解析单个SmsThreadInfo节点*/
			BSTR NodeText,NodeName;
			NodeName = pSubNode->GetnodeName();
			CString sNodeName = BSTR2StrringThenFree(NodeName);

			pSubNode->get_text(&NodeText);
			CString sNodeValue = BSTR2StrringThenFree(NodeText);

			SysFreeString(NodeName);
			SysFreeString(NodeText);

			if (sNodeName == _T("id"))
			{
				pSMS->ulID = _ttoi(sNodeValue.GetBuffer());
			}
			else if (sNodeName == _T("ToList"))
			{
				URLDecoder(sNodeValue.GetBuffer(),pSMS->szTo,255);
			}
			else if (sNodeName == _T("Content") )
			{
				URLDecoder(sNodeValue.GetBuffer(),pSMS->szContent,255);
			}
			else if (sNodeName == _T("ScheduleTime") )
			{
				time_t ulTime = _tstoi64(sNodeValue.GetBuffer());
				JavaTime2Filetime(ulTime,&pSMS->ftSendTime);
			}

			sNodeValue.ReleaseBuffer();
		}

		if (pSMS->ulID < 1)
		{
			free(pSMS);
		}
		else
		{
			smsList.push_back(pSMS);
		}

		++m;
	}

	return smsList;

}


int DeviceAgent::DeleteScheduleSMS(ULONG ulID)
{
	CString sReq;
	sReq.Format(_T("SmsList.xml?action=delschedulesmslist&id=%u"),ulID);
	DataBuffer data;
	list<SCHEDULE_SMS_ITEM*> smsList;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	int iCode = ParseRetCodeXML(data);

	CString sInfo;
	sInfo.Format(_T("DeleteScheduleSMS,phone response with code %d"),iCode);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	DeInitDataBuf(data);

	return PE_RET_OK;
}

int DeviceAgent::AddScheduleSMS(TCHAR *szTo,TCHAR *szContent,FILETIME& ftSendTime)
{
	time_t SendTime = FileTime2JavaTime(&ftSendTime);
	TCHAR *tzEncTo = URLEncodeW(szTo,(int)_tcslen(szTo));
	TCHAR *tzEncContent = URLEncodeW(szContent,(int)_tcslen(szContent));

	CString sReq;
	sReq.Format(_T("action=addschedulesms&content=%s&receiver=%s&smsscheduletime=%lld"),tzEncContent,tzEncTo,SendTime);

	free(tzEncTo);
	free(tzEncContent);

	DataBuffer data;
	if (PE_RET_OK != DeviceAgent::HttpPost(_T("SmsList.xml"),sReq.GetBuffer(),data))
	{
		sReq.ReleaseBuffer();
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	DeInitDataBuf(data);
	sReq.ReleaseBuffer();

	return PE_RET_OK;


}


ULONG DeviceAgent::ParseMaxID(DataBuffer &data)
{
	if (data.iBufferLen <= 0)
	{
		return 0;
	}

	TCHAR *szXML = data.pBuffer;


	TCHAR *szMaxID = _tcsstr(szXML,_T("<MaxID>"));
	if (szMaxID == NULL)
	{
		return 0;
	}

	*szMaxID = 0;
	++szMaxID;
	szMaxID += _tcslen(_T("MaxID>"));
	TCHAR *szEnd = _tcsstr(szMaxID,_T("</MaxID>"));
	if (szEnd == NULL)
	{
		return 0;
	}

	*szEnd = 0;

	ULONG ulMaxMsg = _ttol(szMaxID);

	return ulMaxMsg;
}


int DeviceAgent::ParseRetCodeXML(DataBuffer &data)
{
	if (data.iBufferLen <= 0)
	{
		return 0;
	}

	TCHAR *szXML = data.pBuffer;
	TCHAR *szCode = _tcsstr(szXML,_T("<EARetCode>"));
	if (szCode == NULL)
	{
		return 0;
	}

	*szCode = 0;
	++szCode;
	szCode += _tcslen(_T("EARetCode>"));
	TCHAR *szEnd = _tcsstr(szCode,_T("</EARetCode>"));
	if (szEnd == NULL)
	{
		return 0;
	}

	*szEnd = 0;

	if (szCode && _tcslen(szCode))
	{
		return _ttol(szCode);
	}

	return -1;
}


int DeviceAgent::GetItemCountFromDevice(TCHAR *szAction)
{
	CString sReq = szAction;

	DataBuffer data;

	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return -1;
	}

	int iCount = ParseItemCountXML(data);
	DeInitDataBuf(data);

	return iCount;
}

int DeviceAgent::ParseItemCountXML(DataBuffer &data)
{
	if (data.iBufferLen <= 0)
	{
		return 0;
	}

	TCHAR *szXML = data.pBuffer;
	TCHAR *szCode = _tcsstr(szXML,_T("<ItemCount>"));
	if (szCode == NULL)
	{
		return 0;
	}

	*szCode = 0;
	++szCode;
	szCode += _tcslen(_T("ItemCount>"));
	TCHAR *szEnd = _tcsstr(szCode,_T("</ItemCount>"));
	if (szEnd == NULL)
	{
		return 0;
	}

	*szEnd = 0;

	if (szCode && _tcslen(szCode))
	{
		return _ttol(szCode);
	}

	return -1;
}


int DeviceAgent::SaveContact(TCHAR *szName,TCHAR *szPhone)
{
	CString sReq;
	sReq.Format(_T("ContactList.xml.xml?action=insertcontact&givenname=%s&displayname=%s&cphone=1:%s"),szName,szName,szPhone);

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		return PE_RET_FAIL;
	}

	if (data.iBufferLen < 1)
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	//ShowSyncInfo(_T("Contact saved to device"));
	return PE_RET_OK;
}

int DeviceAgent::DeleteSms(ULONG ulThreadID,ULONG ulID,TCHAR* szPhoneID)
{
	CString sReq;
	sReq.Format(_T("SmsList.xml?action=deletesms&threadid=%u&id=%u&phoneid=%s"),ulThreadID,ulID,szPhoneID);

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	if (data.iBufferLen < 1)
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	DeInitDataBuf(data);
	return PE_RET_OK;
}

int DeviceAgent::DeleteContact(ULONG ulID,TCHAR* szPhoneID)
{
	CString sReq;
	sReq.Format(_T("ContactList.xml?action=deletecontact&id=%d&phoneid=%s"),ulID,szPhoneID);
	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		return PE_RET_FAIL;
	}

	if (data.iBufferLen < 1)
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	DeInitDataBuf(data);
	return PE_RET_OK;
}

int DeviceAgent::DeleteCall(ULONG ulID,TCHAR* szPhoneID)
{
	CString sReq;
	sReq.Format(_T("CallLog.xml?action=deletecalllog&id=%d&phoneid=%s"),ulID,szPhoneID);

	DataBuffer data;
	if (PE_RET_OK != HttpRequest(sReq,data))
	{
		return PE_RET_FAIL;
	}

	if (data.iBufferLen < 1)
	{
		DeInitDataBuf(data);
		return PE_RET_FAIL;
	}

	DeInitDataBuf(data);
	return PE_RET_OK;
}





