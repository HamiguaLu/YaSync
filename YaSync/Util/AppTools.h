#ifndef __APP_TOOL_HEADER__
#define __APP_TOOL_HEADER__


#include "peprotocol.h"
#include "atlstr.h"

//#define TTC_REGKEY_AUTO_START_SECTION			"SOFTWARE\\Microsoft\\Windows CE Services\\AutoStartOnConnect"

BOOL SetRegAutoStart(CString &sPEPath);
BOOL IsAutoStart(CString &sPEPath);


//bool TTC_SN_Verify(char *pszSN/*,TCHAR *szLetter*/);
int VerifySerialNum(char *szSN);
int GetAPPName(TCHAR* szAPPName);
int GetDocPath(TCHAR* szPath);
int GetCFGPath(TCHAR *szPath);
bool VerifyAPP(void);
int GetTrailDays(void);
void ShowMsgOnTray(TCHAR *pszInfo);
void AddFileTimeSpan(FILETIME &ft,time_t timeSpan);
void GetNextDay(FILETIME &ft);
void GetMidnightTime(FILETIME &ft);
void GetLocalMidnightTime(FILETIME &ft);
long GetFileTimeIntervalBySeconds(FILETIME &ft1,FILETIME &ft2);
int PECompareFileTime(FILETIME &ft1,FILETIME &ft2);
void FormatTimeString(__int64 iTimeInSecs,TCHAR *tszTime);
void FmtTimeString(SYSTEMTIME &st,TCHAR *tszTime);
time_t FileTime2JavaTime(LPFILETIME pft);
void JavaTime2Filetime(time_t javaTime,LPFILETIME pft);
void UnixTimeToFileTime(time_t t, LPFILETIME pft);
time_t FileTimeToUnixTime(LPFILETIME pft);
void CheckXmlString(TCHAR *str);
int IsInstanceExist(TCHAR *szAppName);
int InitLog();
void PEWriteLog2File(TCHAR *szLogInfo);
void PEWriteLog2FileA(char *szLogInfo);

int GetAppDataPath(TCHAR* szPath);
int GetAPPPathEx(TCHAR* szPath);
void SetAppInstance(HINSTANCE hInstance);
HINSTANCE GetAppInstance();
void killProcessByName(TCHAR *filename);
void PrintSysInfo();
list<TCHAR*> splitString(TCHAR *str,TCHAR szDiv);
void FreeStringList(list<TCHAR*> strlist);
int RegDll(TCHAR *tszDllName,int iUnReg);
void SetWinOnOutlook(HWND hWnd);
int GetOutlookVersion(int &iVer);

#endif