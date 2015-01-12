// YaSync.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "YaSync.h"
#include "YaSyncDlg.h"
#include "SettingHelper.h"
#include "AppTools.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CYaSyncApp

BEGIN_MESSAGE_MAP(CYaSyncApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CYaSyncApp construction

CYaSyncApp::CYaSyncApp()
{
	EnableHtmlHelp();

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CYaSyncApp object

CYaSyncApp theApp;


// CYaSyncApp initialization

int SendCmd2YaSync(DWORD dwCmd,void *cmdData,int iDataLen)
{
	HWND hWnd = FindWindow(NULL,_T("YaSync"));
	if (hWnd == NULL)
	{
		return PE_RET_FAIL;
	}

	COPYDATASTRUCT cpd;
    cpd.dwData = dwCmd;
    cpd.cbData = iDataLen;
    cpd.lpData = cmdData;
 
    SendMessage(hWnd,WM_COPYDATA,NULL,(LPARAM)&cpd);

	return PE_RET_OK;
}

int g_iAutoSync = 1;
BOOL CYaSyncApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	::CoInitialize(NULL);

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	//SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CString sCmd = ::GetCommandLine();
	sCmd.Trim(_T(" "));

	TCHAR *tszRegCmd = _T("-r");
	TCHAR *tszUnRegCmd = _T("-u");

	int iPos = sCmd.Find(tszRegCmd);
	int iLen = sCmd.GetLength();
	//printf("iPos is %d,iLen is %d",iPos,iLen);
	if (iPos + _tcslen(tszRegCmd) == iLen)
	{
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
		
		exit(0);
	}

	iPos = sCmd.Find(tszUnRegCmd);
	//iLen = sCmd.GetLength();
	//printf("iPos is %d,iLen is %d",iPos,iLen);
	if (iPos + _tcslen(tszUnRegCmd) == iLen)
	{
		int iVer = 0;
		GetOutlookVersion(iVer);
		if (iVer >= 14)
		{
			RegDll(_T("AndroidAddIn.dll"),1);
		}
		else
		{
			RegDll(_T("SMSOutLookAddin.dll"),1);
		}
		exit(0);
	}

#if 0
	if (sCmd.Find(_T("Install")) >= 0)
	{
		CString sInfo;
		sInfo.Format(_T("http://a.mobitnt.com/install.php?P=YaSync%d"),APP_VER);
		ShellExecute(NULL,_T("open"),sInfo,NULL,NULL,SW_SHOW);

		PE_COMMON_SETTING *s = (PE_COMMON_SETTING*)calloc(1,sizeof(PE_COMMON_SETTING));
		//SettingHelper::GetCommonSetting(s);
		free(s);

		exit(0);
	}
#endif

	if (sCmd.Find(_T("Uninstall")) >= 0)
	{
		CString sInfo;
		sInfo.Format(_T("http://a.mobitnt.com/uninstall.php?P=YaSync%d"),APP_VER);
		ShellExecute(NULL,_T("open"),sInfo,NULL,NULL,SW_SHOW);

		int iVer = 0;
		GetOutlookVersion(iVer);
		if (iVer >= 14)
		{
			RegDll(_T("AndroidAddIn.dll"),1);
		}
		else
		{
			RegDll(_T("SMSOutLookAddin.dll"),1);
		}
		exit(0);
	}

	if (IsInstanceExist(_T("YaSync")) == PE_RET_OK)
	{
		Sleep(500);
		HWND hWnd = FindWindow(NULL,YASYNC_MAIN_WIN_NAME);
		if (hWnd == NULL)
		{
			//AfxMessageBox(_T("Oops,can not find YaSync window"),MB_OK|MB_ICONSTOP);
			return PE_RET_FAIL;
		}

		COPYDATASTRUCT cpd;
    	cpd.dwData = PE_CMD_SHOW_UI;
    	cpd.cbData = 0;
    	cpd.lpData = 0;
 
	    SendMessage(hWnd,WM_COPYDATA,NULL,(LPARAM)&cpd);

		exit(0);
	}

	int iAutoStart = 0;
	if (sCmd.Find(_T("autoStart")) >= 0)
	{
		iAutoStart = 2;
	}

	if (sCmd.Find(_T("startByOutlook")) >= 0)
	{
		g_iAutoSync = 0;
		iAutoStart = 2;
	}

	CYaSyncDlg dlg;
	dlg.m_iAutoStart = iAutoStart;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}

