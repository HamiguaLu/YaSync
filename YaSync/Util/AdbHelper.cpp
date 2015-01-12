
#include "stdafx.h"

#include "PEProtocol.h"
#include "AppTools.h"
#include "AdbHelper.h"
#include "PECore.h"


HANDLE CAdbHelper::m_hExitEvt;

CAdbHelper::CAdbHelper()
{
	m_hChildStdRd = NULL;
	m_hChildStdWr = NULL;
}


int CAdbHelper::StartAdbProxy(int iPort)
{
	//[EXP]clean other adb process first
	/*_stprintf(m_tszCmd,_T("kill-server"));
	startadb();*/

	_stprintf(m_tszCmd,_T("forward tcp:%d tcp:%d"),iPort,iPort);
	if (PE_RET_FAIL == startadb() )
	{
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}

int  CAdbHelper::StartApp()
{
	//adb shell am start -n com.package.name/com.package.name.ActivityName
	_stprintf(m_tszCmd,_T("shell am start -n mobitnt.android.YaSync/mobitnt.android.YaSync.YaSync"));
	if (PE_RET_FAIL == startadb() )
	{
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}

int CAdbHelper::GetDeviceState()
{
	int iState = PE_CONNNETION_STATE_UNKONW;
	_stprintf(m_tszCmd,_T("get-state"));
	if (PE_RET_FAIL == startadb() )
	{
		return iState;
	}

	iState = PE_CONNNETION_STATE_OFFLINE;
	if (strstr(m_szAdbOut,"device"))
	{
		iState = PE_CONNNETION_STATE_ONLINE;
	}

	return iState;
}


int CAdbHelper::InstallApp2Device()
{
	TCHAR tszAdbPath[255];
	GetAPPPathEx(tszAdbPath);

	_stprintf(m_tszCmd,_T("install -r \"%s\\YaSyncClient.apk\""),tszAdbPath);
	if (PE_RET_FAIL == startadb() )
	{
		return PE_RET_FAIL;
	}

#if 0
	if (strcmp(m_szAdbOut,"online") == 0)
	{
		return PE_RET_FAIL;
	}
#endif

	int iRet = StartApp();
	PECore::PostDelayMessage(WM_SCAN_DEVICE,0,0,10);

	return iRet;
}



int CAdbHelper::startadb() 
{ 
	SECURITY_ATTRIBUTES saAttr;
	PEWriteLog(TEXT("startadb enter"));

	// Set the bInheritHandle flag so pipe handles are inherited. 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; 
	saAttr.lpSecurityDescriptor = NULL; 

	// Create a pipe for the child process's STDOUT. 
	if ( ! CreatePipe(&m_hChildStdRd, &m_hChildStdWr, &saAttr, 0) ) 
	{
		PEWriteLog(_T("StdoutRd CreatePipe"));
		return PE_RET_FAIL;
	}

	// Ensure the read handle to the pipe for STDOUT is not inherited.
	if ( ! SetHandleInformation(m_hChildStdRd, HANDLE_FLAG_INHERIT, 0) )
	{
		PEWriteLog(_T("Stdout SetHandleInformation"));
		return PE_RET_FAIL;
	}

	// Create the child process. 
	CreateChildProcess();

	ReadFromPipe();

	PEWriteLog(TEXT("startadb exit"));

	return PE_RET_OK; 
} 

// Create a child process that uses the previously created pipes for STDIN and STDOUT.
void CAdbHelper::CreateChildProcess()
{ 
	TCHAR szCmdline[500];
	TCHAR tszAdbPath[255];
	GetAPPPathEx(tszAdbPath);
	_stprintf(szCmdline,_T("%sadb.exe %s"),tszAdbPath,m_tszCmd);

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE; 

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO); 
	siStartInfo.hStdError = m_hChildStdWr;
	siStartInfo.hStdOutput = m_hChildStdWr;
	//siStartInfo.hStdInput = g_hChildStdRd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL, 
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if ( ! bSuccess ) 
	{
		DWORD dwError = GetLastError();

		CString sInfo;
		sInfo.Format(_T("CreateProcess failed error code is %d"),dwError);
		PEWriteLog(sInfo.GetBuffer(sInfo.GetLength()));
		sInfo.ReleaseBuffer();
		return;
	}
	else 
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
}



// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
void CAdbHelper::ReadFromPipe(void) 
{ 
	DWORD dwRead; 

	memset(m_szAdbOut,0,BUFSIZE * sizeof(CHAR));
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// Close the write end of the pipe before reading from the 
	// read end of the pipe, to control child process execution.
	// The pipe is assumed to have enough buffer space to hold the
	// data the child process has already written to it.

	if (!CloseHandle(m_hChildStdWr)) 
	{
		//ErrorExit(TEXT("StdOutWr CloseHandle"));
		return;
	}

	for (;;) 
	{ 
		bSuccess = ReadFile( m_hChildStdRd, m_szAdbOut, BUFSIZE, &dwRead, NULL);
		if( ! bSuccess || dwRead == 0 ) break; 

		//PEWriteLog2File(_T("adb says:"));
		PEWriteLog2FileA(m_szAdbOut);
	} 
} 


DWORD WINAPI CAdbHelper::AdbScannerT(LPVOID  lparam)
{
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	PEWriteLog(_T("AdbScannerT started"));

	TCHAR szCmdline[500];
	TCHAR tszAdbPath[255];
	GetAPPPathEx(tszAdbPath);
	_stprintf(szCmdline,_T("%sadb.exe wait-for-device"),tszAdbPath);

	PROCESS_INFORMATION piProcInfo; 
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE; 

	ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.
	ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );

	// Create the child process. 
	bSuccess = CreateProcess(NULL, 
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		CREATE_NO_WINDOW,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

	// If an error occurs, exit the application. 
	if (!bSuccess ) 
	{
		DWORD dwError = GetLastError();

		CString sInfo;
		sInfo.Format(_T("CreateProcess failed error code is %d"),dwError);
		PEWriteLog(sInfo.GetBuffer(sInfo.GetLength()));
		sInfo.ReleaseBuffer();
		return 0;
	}

	HANDLE Events[2];
	Events[0] = piProcInfo.hProcess;
	Events[1] = m_hExitEvt;
	
	int nIndex = WaitForMultipleObjects(2, Events, FALSE,INFINITE);
	if (nIndex == WAIT_OBJECT_0 + 1)
	{
		PEWriteLog(_T("YaSync will exit,stop wait adb"));
		return 0;
	}

	if (nIndex != WAIT_OBJECT_0)
	{
		PEWriteLog(_T("Error occured when waiting ADB"));
		return 0;
	}

	PEWriteLog(_T("device connected?"));

	CAdbHelper *pHelper = new CAdbHelper();
	pHelper->StartAdbProxy(PE_ANDROID_SVR_PORT);
	//pHelper->StartAdbProxy(PE_ANDROID_EVENT_PORT);
	delete pHelper;

	PEWriteLog(_T("Test Conn from ADB"));
	if (PE_RET_FAIL == DeviceAgent::TestConn(_T("127.0.0.1")))
	{
		//adb connected but
		static int iInstalled = 0;
		if (iInstalled == 0)
		{
			iInstalled = 1;
			PEWriteLog(_T("adb connected but can not connect to PEClient,may need to intall it"));
			PECore::PostMessage(WM_SHOW_INSTALL_UI,0,0);
		}
	}

	PEWriteLog(_T("AdbScannerT ended"));

	return 0;
}

