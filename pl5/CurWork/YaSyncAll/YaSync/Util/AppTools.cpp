#include "stdafx.h"
#include "AppTools.h"

#include "SettingHelper.h"

TCHAR g_szLogFilePath[MAX_PATH];
char g_szLogFilePathA[MAX_PATH];
HINSTANCE g_HInstance;
CRITICAL_SECTION cs4Log;

TCHAR *GetLogPath()
{
	return g_szLogFilePath;
}


void SetAppInstance(HINSTANCE hInstance)
{
	g_HInstance = hInstance;
}

HINSTANCE GetAppInstance()
{
	return g_HInstance;
}

FILE * openLog()
{
	FILE *pLog = _tfopen(g_szLogFilePath,_T("at"));
	if (pLog == NULL)
	{
		return NULL;
	}

	int iRet = fseek(pLog,0,SEEK_END);
	if (iRet != 0)
	{
		fclose(pLog);
		return NULL;
	}

	static int iCount = 0;
	if (++iCount < 1000)
	{
		return pLog;
	}

	iCount = 0;
	int iLen = ftell(pLog);
	if (iLen <= 0)
	{
		return pLog;
	}

	iLen /= 1024;
	if (iLen > 1024 * 5)
	{
		fclose(pLog);
		pLog = _tfopen(g_szLogFilePath,_T("wt"));
	}

	return pLog;
}


void PEWriteLog2File(TCHAR *szLogInfo)
{
	if (szLogInfo == NULL)
	{
		return;
	}

	SYSTEMTIME stStart;
	GetLocalTime(&stStart);
	TCHAR szStartTime[255];
	_stprintf(szStartTime,_T("[%02d-%02d-%04d %02d:%02d:%02d]"),
		stStart.wMonth,stStart.wDay,stStart.wYear,stStart.wHour,stStart.wMinute,stStart.wSecond);

	FILE *pLog = openLog();
	if (pLog == NULL)
	{
		return;
	}

	_ftprintf(pLog,_T("%s :%s\r\n"),szStartTime,szLogInfo);

	fflush(pLog);
	fclose(pLog);
}


void PEWriteLog2FileA(char *szLogInfo)
{
	if (szLogInfo == NULL)
	{
		return;
	}

	SYSTEMTIME stStart;
	GetLocalTime(&stStart);
	char szStartTime[255];
	sprintf(szStartTime,"[%02d-%02d-%04d %02d:%02d:%02d]",
		stStart.wMonth,stStart.wDay,stStart.wYear,stStart.wHour,stStart.wMinute,stStart.wSecond);

	FILE *pLog = fopen(g_szLogFilePathA,"at");
	if (pLog == NULL)
	{
		return;
	}

	if (pLog != NULL)
	{
		fprintf(pLog,"%s :%s\n",szStartTime,szLogInfo);
		fclose(pLog);
	}
}


int InitLog()
{
	InitializeCriticalSectionAndSpinCount(&cs4Log,0x80000400);

	memset(g_szLogFilePath,0,MAX_PATH);
	if (PE_RET_OK != GetAppDataPath(g_szLogFilePath))
	{
		_stprintf(g_szLogFilePath,PE_LOG_FILE_NAME);
		sprintf(g_szLogFilePathA,PE_LOG_FILE_NAMEA);
	}
	else
	{
		_tcscat(g_szLogFilePath,PE_LOG_FILE_NAME);
		WideCharToMultiByte (CP_ACP,NULL,(TCHAR*)(LPCTSTR)g_szLogFilePath,-1,(LPSTR)g_szLogFilePathA,255,NULL,FALSE);
		strcat(g_szLogFilePathA,PE_LOG_FILE_NAMEA);
	}

	FILE *pLog = _tfopen(g_szLogFilePath,_T("at"));
	if (pLog == NULL)
	{
		return PE_RET_FAIL;
	}

	int iRet = fseek(pLog,0,SEEK_END);
	if (iRet != 0)
	{
		fclose(pLog);
		return PE_RET_FAIL;
	}

	int iLen = ftell(pLog);
	if (iLen <= 0)
	{
		fclose(pLog);
		return 0;
	}

	iLen /= 1024;
	if (iLen > 1024 * 5)
	{
		fclose(pLog);
		pLog = _tfopen(g_szLogFilePath,_T("wt"));

	}

	fclose(pLog);


	pLog = fopen(g_szLogFilePathA,"at");
	if (pLog == NULL)
	{
		return PE_RET_FAIL;
	}

	iRet = fseek(pLog,0,SEEK_END);
	if (iRet != 0)
	{
		fclose(pLog);
		return PE_RET_FAIL;
	}

	iLen = ftell(pLog);
	if (iLen <= 0)
	{
		fclose(pLog);
		return 0;
	}

	iLen /= 1024;
	if (iLen > 1024 * 1024 * 5)
	{
		fclose(pLog);
		pLog = fopen(g_szLogFilePathA,"wt");
	}

	fclose(pLog);

	return 0;
}

int GetAPPName(TCHAR* szAPPName)
{
	TCHAR szAPPPath[255];
	GetModuleFileName(NULL, szAPPPath, MAX_PATH);
	TCHAR* pData = _tcsstr(szAPPPath, _T("\\"));
	if (pData == NULL)
	{
		return -1;
	}

	_tcscpy_s(szAPPName,255,pData);

	return 0;
}


#include "Shlobj.h"

int GetAppDataPath(TCHAR* szPath)
{
	//	TCHAR tszAppDataPath[_MAX_PATH];  
	SHGetSpecialFolderPath(0,szPath,CSIDL_LOCAL_APPDATA,0);

	_tcscat(szPath,_T("\\mobitnt\\"));
	_tmkdir(szPath);

	_tcscat(szPath,_T("YaSync\\"));
	_tmkdir(szPath);

	return PE_RET_OK;
}


int GetDocPath(TCHAR* szPath)
{
	SHGetSpecialFolderPath(0,szPath,CSIDL_PERSONAL,0);

	return PE_RET_OK;
}



int GetAPPPathEx(TCHAR* szPath)
{
	if (0 == GetModuleFileName(GetAppInstance(), szPath, MAX_PATH))
	{
		return PE_RET_FAIL;
	}

	TCHAR* pData = _tcsrchr(szPath, _T('\\'));
	if (pData == NULL)
	{
		return -1;
	}
	*(++pData) = 0;
	return PE_RET_OK;
}


bool VerifyAPP(void)
{
	int iDays =	 GetTrailDays();

	if (iDays <= MAX_EXPIRE_DAY && iDays >= 0)
	{
		return true;
	}

	return false;
}

int GetTrailDays(void)
{
	PE_COMMON_SETTING *s = (PE_COMMON_SETTING*)calloc(1,sizeof(PE_COMMON_SETTING));
	CSettingHelper::GetCommonSetting(s);

	time_t ltime;
	time( &ltime );

	int iDiff = (int)ltime - (int)s->dwTrialStartTime;

	free(s);

	return (int)iDiff/(60*60*24);
}


int IsInstanceExist(TCHAR *szAppName)
{
	HANDLE m_hMutex = CreateMutex(NULL, FALSE, szAppName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;
		return PE_RET_OK;
	}

	return PE_RET_FAIL;
}




/*time interval in seconds*/
long GetFileTimeIntervalBySeconds(FILETIME &ft1,FILETIME &ft2)
{
	ULONG64 iFt1 = 0;
	ULONG64 iFt2 = 0;
	ULONG64 iInterval = 0;

	memcpy(&iFt1,&ft1,sizeof(ULONG64));
	memcpy(&iFt2,&ft2,sizeof(ULONG64));

	if (iFt1 > iFt2)
	{
		iInterval = iFt1 - iFt2;
	}
	else
	{
		iInterval = iFt2 - iFt1;
	}

	iInterval = iInterval/ONE_SECOND;/* File time is in 100s of nanoseconds */
	return long(iInterval);
}

void GetNextDay(FILETIME &ft)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft,&st);
	CTime t(st.wYear,st.wMonth,st.wDay,23,59,59);
	t.GetAsSystemTime(st);

	SystemTimeToFileTime(&st,&ft);

	AddFileTimeSpan(ft,(ONE_SECOND + 1)* 60);

}


void GetMidnightTime(FILETIME &ft)
{
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft,&st);

	time_t t = 0;
	memcpy(&t,&ft,sizeof(FILETIME));

	time_t t1 = st.wHour * 60 * 60;
	t1 = t1 * ONE_SECOND;
	t -= t1;

	t1 = st.wMinute * 60;
	t1 = t1 * ONE_SECOND; 
	t -= t1;

	t1 = st.wSecond * ONE_SECOND;
	t -= t1;
	t -= st.wMilliseconds;


	memcpy(&ft,&t,sizeof(FILETIME));
}

void GetLocalMidnightTime(FILETIME &ft)
{
	FILETIME ft2;
	FileTimeToLocalFileTime(&ft,&ft2);
	SYSTEMTIME st;
	FileTimeToSystemTime(&ft2,&st);

	time_t t = 0;
	memcpy(&t,&ft2,sizeof(FILETIME));

	time_t t1 = st.wHour * 60 * 60;
	t1 = t1 * ONE_SECOND;
	t -= t1;

	t1 = st.wMinute * 60;
	t1 = t1 * ONE_SECOND; 
	t -= t1;

	t1 = st.wSecond * ONE_SECOND;
	t -= t1;
	t -= st.wMilliseconds;


	memcpy(&ft2,&t,sizeof(FILETIME));

	LocalFileTimeToFileTime(&ft2,&ft);

}

void AddFileTimeSpan(FILETIME &ft,time_t timeSpan)
{
	time_t iFt1 = 0;
	memcpy(&iFt1,&ft,sizeof(FILETIME));
	iFt1 += timeSpan;
	memcpy(&ft,&iFt1,sizeof(FILETIME));
}

int PECompareFileTime(FILETIME &ft1,FILETIME &ft2)
{
	ULONG64 iFt1 = 0;
	ULONG64 iFt2 = 0;
	ULONG64 iInterval = 0;

	memcpy(&iFt1,&ft1,sizeof(ULONG64));
	memcpy(&iFt2,&ft2,sizeof(ULONG64));

	if (iFt1 > iFt2)
	{
		return PE_RET_CMP_GREATER;
	}

	if (iFt1 < iFt2)
	{
		return PE_RET_CMP_LOWER;
	}

	return PE_RET_CMP_EQUAL;
}


void FmtTimeString(SYSTEMTIME &st,TCHAR *tszTime)
{
	TCHAR szDate[100];
	TCHAR szTime[100];

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL,szTime, 255);
	GetDateFormat(LOCALE_USER_DEFAULT,0,&st,NULL,szDate,255);

	_stprintf(tszTime,_T("\"%s %s\""),szDate,szTime);
}

void FormatTimeString(__int64 iTimeInSecs,TCHAR *tszTime)
{
	DWORD dwHour = (DWORD)(iTimeInSecs / 3600);
	DWORD dwMin  = (DWORD)((iTimeInSecs - dwHour * 3600)/60);
	int dwSec  = int(iTimeInSecs - dwHour * 3600 - dwMin * 60);

	if (dwHour > 0)
	{
		_stprintf(tszTime,_T(" : %u hours %u mins %u secs"),dwHour,dwMin,dwSec);
		return;
	}

	if (dwMin > 0)
	{
		_stprintf(tszTime,_T(" : %u mins %u secs"),dwMin,dwSec);
		return;
	}

	_stprintf(tszTime,_T(" : %u secs"),dwSec);

	return;
}

void CheckXmlString(TCHAR *str)
{
	if (!str || _tcslen(str) < 1)
	{
		_tcscpy(str,_T("None"));
		return;
	}

	CString sVal = str;

	sVal.Replace(_T("&amp;"),_T("&"));
	sVal.Replace(_T("&lt;"),_T("<"));
	sVal.Replace(_T("&gt;"),_T(">"));
	sVal.Replace(_T("&apos;"),_T("'"));
	sVal.Replace(_T("&quot;"),_T("\""));

	_tcscpy(str,sVal.GetBuffer(-1));
	sVal.ReleaseBuffer();
}


int VerifySerialNum(char *szSN)
{
	TCHAR tszNO[255];
	MultiByteToWideChar(CP_ACP,0,szSN,-1,tszNO,50);

	CString sSN = tszNO;

	PEWriteLog(_T("VerifySerialNum"));

	int iPos = sSN.Find(_T("-"));
	long lNum[4];
	CString sRemain = sSN;
	CString sNum;
	for (int i = 0; i < 4; ++i)
	{
		iPos = sRemain.Find(_T("-"));
		if (iPos == -1)
		{
			sNum = sRemain;
		}
		else
		{
			sNum = sRemain.Left(iPos);
		}

		sRemain = sRemain.Right(sRemain.GetLength() - iPos - 1);
		if (sRemain.GetLength() < 1)
		{
			return PE_RET_FAIL;
		}

		lNum[i] = _tcstol(sNum.GetBuffer(),NULL,16);
		sNum.ReleaseBuffer();
	}

	for (int i = 0; i < 3; ++i)
	{
		if (lNum[i] == 0)
		{
			return PE_RET_FAIL;
		}
	}

	if ( (lNum[0] + lNum[3] != lNum[2] + lNum[1] )&& (lNum[0] + lNum[3] != lNum[2] - lNum[1]))
	{
		return PE_RET_FAIL;
	}

	PEWriteLog(_T("VerifySerialNum:success!"));
	PE_COMMON_SETTING *s = (PE_COMMON_SETTING*)calloc(1,sizeof(PE_COMMON_SETTING));
	CSettingHelper::GetCommonSetting(s);
	s->dwIsRegistered = 1;
	_tcscpy(s->szSN,tszNO);
	CSettingHelper::SetCommonSetting(s);

	free(s);

	return PE_RET_OK;
}



#define FILETIME_EPOCH_DIFF 				11644473600000L
#define FILETIME_ONE_MILLISECOND   			(10 * 1000)

time_t FileTime2JavaTime(LPFILETIME pft) {
	time_t filetime;
	memcpy(&filetime,pft,sizeof(FILETIME));
	if (filetime == 0)
	{
		return 0;
	}

	filetime = filetime / FILETIME_ONE_MILLISECOND;
	filetime -= FILETIME_EPOCH_DIFF;
	return filetime;
}

void JavaTime2Filetime(time_t javaTime,LPFILETIME pft) {
	long long int filetime;
	filetime = (javaTime + FILETIME_EPOCH_DIFF) * FILETIME_ONE_MILLISECOND;
	memcpy(pft,&filetime,sizeof(filetime));

	return;
}

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = (DWORD)(ll >> 32);
}

#define WINDOWS_TICK 10000000
#define SEC_TO_UNIX_EPOCH 11644473600LL

time_t FileTimeToUnixTime(LPFILETIME pft)
{
	time_t windowsTicks;
	memcpy(&windowsTicks,pft,sizeof(FILETIME));
	if (windowsTicks == 0)
	{
		return 0;
	}

	windowsTicks = windowsTicks/WINDOWS_TICK;
	return windowsTicks - SEC_TO_UNIX_EPOCH;
}


#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>
void killProcessByName(TCHAR *filename)
{
#ifndef _DEBUG
	int nRetCode = 0;

	///////////////////////////////////////////////////////////////
	// Finding Application which we have specified once that 
	// application finds it will give you message Application found
	// then first of all it opens that process and getting all rights 
	// of that application.Once we have all rights then we can kill 
	// that application with Terminate Process.
	///////////////////////////////////////////////////////////////
	HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
	DWORD dwsma = GetLastError();

	DWORD dwExitCode = 0;

	PROCESSENTRY32  procEntry={0};
	procEntry.dwSize = sizeof( PROCESSENTRY32 );
	Process32First(hndl,&procEntry);
	do
	{
		if(!_tcscmp(procEntry.szExeFile,filename))
		{
			HANDLE hHandle = ::OpenProcess(PROCESS_ALL_ACCESS,0,procEntry.th32ProcessID);

			::GetExitCodeProcess(hHandle,&dwExitCode);
			::TerminateProcess(hHandle,dwExitCode);
			::CloseHandle(hHandle);
			PEWriteLog(_T("adb process terminated"));
		}

	}while(Process32Next(hndl,&procEntry));
#endif
}

void PrintSysInfo()
{
	OSVERSIONINFOEX ifo;
	ifo.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&ifo);

	DWORD a=ifo.dwBuildNumber;
	DWORD b=ifo.dwMajorVersion;
	DWORD c=ifo.dwMinorVersion;
	DWORD d=ifo.dwOSVersionInfoSize;
	DWORD e=ifo.dwPlatformId;
	CString f=ifo.szCSDVersion;

	CString str;
	str.Format(_T("OS info:buildNumber:%d,MajorVersion:%d,MinorVersion:%d,PlatformId:%d,CSDVersion:%s"),a,b,c,e,f);
	PEWriteLog(str.GetBuffer());
	str.ReleaseBuffer();

	str.Format(_T("YaSyncVer:%d build at %s %s"),APP_VER,_T(__DATE__),_T(__TIME__));
	PEWriteLog(str.GetBuffer());
	str.ReleaseBuffer();
}

void FreeStringList(list<TCHAR*> strlist)
{
	list<TCHAR*>::iterator it;
	for (it = strlist.begin(); it != strlist.end(); ++it)
	{ 
		TCHAR*p = (TCHAR*)*it;
		free(p);
	}
	strlist.clear();
}

list<TCHAR*> splitString(TCHAR *srcStr,TCHAR cDiv)
{
	list<TCHAR*> strList;
	strList.clear();

	int iLen = (int)_tcslen(srcStr);
	TCHAR *str = (TCHAR *)calloc(iLen + 1,sizeof(TCHAR));
	TCHAR *tmp = (TCHAR *)calloc(iLen + 1,sizeof(TCHAR));
	_tcscpy(str,srcStr);

	TCHAR *szStart = str;
	TCHAR *szEnd = NULL;

	int iPos = 0;
	while (iLen-- >= 0)
	{
		if (*szStart == cDiv || iLen < 0)
		{
			tmp[iPos++] = 0;
			if (tmp && _tcslen(tmp) > 0)
			{
				TCHAR *szItem = (TCHAR*)calloc(iPos + 1,sizeof(TCHAR));
				_tcscpy(szItem,tmp);
				strList.push_back(szItem);
			}
			iPos = 0;
		}
		else
		{
			tmp[iPos++] = *szStart;
		}
		++szStart;
	}

	free(str);
	free(tmp);

	return strList;
}


int RegDll(TCHAR *tszDllName,int iUnReg)
{
	TCHAR tszPath[1024];
	GetAPPPathEx(tszPath);

	TCHAR tszCmd[1024];

	_stprintf(tszCmd,_T("\"%s\\%s\""),tszPath,tszDllName);

	if (iUnReg)
	{
		_tcscat(tszCmd,_T(" /u"));
	}

	SHELLEXECUTEINFO ShExecInfo = {0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	//ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = _T("runas");
	ShExecInfo.lpFile = _T("regsvr32.exe");		
	ShExecInfo.lpParameters = tszCmd;	
	//ShExecInfo.lpDirectory = tszPath;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;	
	return ShellExecuteEx(&ShExecInfo);
}


int g_iOutlookCmd = 0;
void SetWinOnOutlook(HWND hWnd)
{
	if (g_iOutlookCmd == 0)
	{
		return;
	}

	HWND hOutlook = FindWindow(_T("rctrl_renwnd32"), NULL);

	SetWindowPos(hOutlook,hWnd,0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);

	g_iOutlookCmd = 0;

	SetForegroundWindow(hWnd);
}


#define HKEY_PE_APP_PATH 	_T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\outlook.exe")

int GetOutlookVersion(int &iVer)
{
	TCHAR szPath[255];
	szPath[254] = 0;
	HKEY hkey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, HKEY_PE_APP_PATH, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
	{
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, HKEY_PE_APP_PATH, 0, KEY_ALL_ACCESS, &hkey)!= ERROR_SUCCESS)
		{
			return PE_RET_FAIL;
		}
	}

	DWORD dwType,dwLen = 255;
	RegQueryValueEx(hkey, _T("Path"), 0, &dwType, (BYTE*)(szPath) ,&dwLen);

	RegCloseKey(hkey);

	TCHAR szOutlookPath[1024];
	szOutlookPath[1023] = 0;
	_stprintf(szOutlookPath,_T("%s\\outlook.exe"),szPath);

	DWORD handle = 0;
	int size = GetFileVersionInfoSize(szOutlookPath, &handle);
	BYTE* versionInfo = new BYTE[size];
	if (!GetFileVersionInfo(szOutlookPath, handle, size, versionInfo))
	{
		delete[] versionInfo;
		return PE_RET_FAIL;
	}
	// we have version information
	UINT    			len = 0;
	VS_FIXEDFILEINFO*   vsfi = NULL;
	VerQueryValue(versionInfo, L"\\", (void**)&vsfi, &len);
#if 0
	int aVersion[4];
	aVersion[0] = HIWORD(vsfi->dwFileVersionMS);
	aVersion[1] = LOWORD(vsfi->dwFileVersionMS);
	aVersion[2] = HIWORD(vsfi->dwFileVersionLS);
	aVersion[3] = LOWORD(vsfi->dwFileVersionLS);
#else
	iVer = HIWORD(vsfi->dwFileVersionMS);
#endif
	delete[] versionInfo;

	return PE_RET_OK;
}






