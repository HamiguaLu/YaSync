// YaSyncDlg.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"

#include "DeviceAgent.h"

#include "AppTools.h"
#include "PEInterface.h"
#include "YaSyncDlg.h"

#include "TrayHelper.h"

#include "PECore.h"

#include "InstallDlg.h"
#include "EnterSecurityCodeDlg.h"

#include "SaveContactsDlg.h"

#include "RestoreDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CYaSyncDlg dialog


CYaSyncDlg::CYaSyncDlg(CWnd* pParent /*=NULL*/)
: CDialog(CYaSyncDlg::IDD, pParent)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hDialogBrush = CreateSolidBrush(DLG_BK_COLOR);
}

void CYaSyncDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CYaSyncDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_HOME_BTN, &CYaSyncDlg::OnBnClickedHomeBtn)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(ID_SYNC_BTN, &CYaSyncDlg::OnBnClickedSyncBtn)
	ON_BN_CLICKED(ID_SETTING_BTN, &CYaSyncDlg::OnBnClickedSettingBtn)
	ON_BN_CLICKED(ID_HELP_BTN, &CYaSyncDlg::OnBnClickedHelpBtn)
	ON_BN_CLICKED(ID_BUY_BTN, &CYaSyncDlg::OnBnClickedBuyBtn)
	ON_COMMAND(ID_AAA_SHOWMAINUI, &CYaSyncDlg::OnAaaShowmainui)
	ON_COMMAND(ID_AAA_NEWSMS, &CYaSyncDlg::OnAaaNewsms)
	ON_WM_NCPAINT()
	ON_COMMAND(ID_AAA_FILEMANAGER, &CYaSyncDlg::OnAaaFilemanager)
	ON_COMMAND(ID_AAA_PHOTOVIEWER, &CYaSyncDlg::OnAaaPhotoviewer)
	ON_COMMAND(ID_AAA_EXIT, &CYaSyncDlg::OnAaaExit)
	ON_COMMAND(ID_AAA_SCANDEVICE, &CYaSyncDlg::OnAaaScandevice)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CLOSE_BTN, &CYaSyncDlg::OnBnClickedCloseBtn)
	ON_BN_CLICKED(IDC_MIN_BTN, &CYaSyncDlg::OnBnClickedMinBtn)
	ON_WM_ENDSESSION()
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


int CYaSyncDlg::SetBtnColor(int iFocusIndex)
{
	if (iFocusIndex == 0)
	{
		m_btnHome.SetFocus();
		m_btnHome.SetColor(CButtonST::BTNST_COLOR_BK_OUT,BTN_BK_FOCUS_COLOR);
		m_btnHome.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnHome.SetColor(CButtonST::BTNST_COLOR_FG_OUT,BTN_FONT_COLOR);
	}
	else
	{
		m_btnHome.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MAIN_BTN_BK_COLOR);
		m_btnHome.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnHome.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MAIN_BTN_FONT_COLOR);
	}

	if (iFocusIndex == 1)
	{
		m_btnSync.SetFocus();
		m_btnSync.SetColor(CButtonST::BTNST_COLOR_BK_OUT,BTN_BK_FOCUS_COLOR);
		m_btnSync.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnSync.SetColor(CButtonST::BTNST_COLOR_FG_OUT,BTN_FONT_COLOR);
	}
	else
	{
		m_btnSync.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MAIN_BTN_BK_COLOR);
		m_btnSync.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnSync.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MAIN_BTN_FONT_COLOR);
	}

	if (iFocusIndex == 2)
	{
		m_btnSetting.SetFocus();
		m_btnSetting.SetColor(CButtonST::BTNST_COLOR_BK_OUT,BTN_BK_FOCUS_COLOR);
		m_btnSetting.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnSetting.SetColor(CButtonST::BTNST_COLOR_FG_OUT,BTN_FONT_COLOR);
	}
	else
	{
		m_btnSetting.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MAIN_BTN_BK_COLOR);
		m_btnSetting.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnSetting.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MAIN_BTN_FONT_COLOR);
	}

	if (iFocusIndex == 3)
	{
		m_btnHelp.SetFocus();
		m_btnHelp.SetColor(CButtonST::BTNST_COLOR_BK_OUT,BTN_BK_FOCUS_COLOR);
		m_btnHelp.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnHelp.SetColor(CButtonST::BTNST_COLOR_FG_OUT,BTN_FONT_COLOR);
	}
	else
	{
		m_btnHelp.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MAIN_BTN_BK_COLOR);
		m_btnHelp.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnHelp.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MAIN_BTN_FONT_COLOR);
	}

	if (iFocusIndex == 4)
	{
		m_btnBuy.SetFocus();
		m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_OUT,BTN_BK_FOCUS_COLOR);
		m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnBuy.SetColor(CButtonST::BTNST_COLOR_FG_OUT,BTN_FONT_COLOR);
	}
	else
	{
		m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MAIN_BTN_BK_COLOR);
		m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,BTN_BK_FOCUS_COLOR);
		m_btnBuy.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MAIN_BTN_FONT_COLOR);
	}

	return 0;
}

// CYaSyncDlg message handlers
extern CRITICAL_SECTION cs4TrayDlg;

BOOL CYaSyncDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	SetMainUIHandle(m_hWnd);

	PE_Init();

	UpdateTrayIcon(TRAY_ICON_OFFLINE);

	PEStartSvr();

	m_homeUI.Create(IDD_MAIN_UI,this);
	m_settingUI.Create(IDD_SETTING_UI,this);
	m_syncUI.Create(IDD_SYNC_UI,this);
	m_helpUI.Create(IDD_HELP_UI,this);
	m_buyUI.Create(IDD_BUY_UI,this);
	m_trayDlg.Create(IDD_TRAY,this);

	m_btnHome.SubclassDlgItem(ID_HOME_BTN, this);
	m_btnSync.SubclassDlgItem(ID_SYNC_BTN, this);
	m_btnSetting.SubclassDlgItem(ID_SETTING_BTN, this);
	m_btnBuy.SubclassDlgItem(ID_BUY_BTN, this);
	m_btnHelp.SubclassDlgItem(ID_HELP_BTN, this);

	m_btnHome.SetIcon(IDI_HOME);
	m_btnSync.SetIcon(IDI_SYNC);
	m_btnSetting.SetIcon(IDI_SETTING);
	m_btnBuy.SetIcon(IDI_BUY);
	m_btnHelp.SetIcon(IDI_HELP);

	m_btnHome.SetAlign(CButtonST::ST_ALIGN_OVERLAP);
	m_btnSync.SetAlign(CButtonST::ST_ALIGN_OVERLAP);
	m_btnSetting.SetAlign(CButtonST::ST_ALIGN_OVERLAP);
	m_btnBuy.SetAlign(CButtonST::ST_ALIGN_OVERLAP);
	m_btnHelp.SetAlign(CButtonST::ST_ALIGN_OVERLAP);

	m_btnMIN.SubclassDlgItem(IDC_MIN_BTN, this);
	m_btnClose.SubclassDlgItem(IDC_CLOSE_BTN, this);

	m_btnMIN.SetIcon(IDI_BTN_MIN);
	m_btnClose.SetIcon(IDI_CLOSE_BTN);

	m_btnMIN.SetColor(CButtonST::BTNST_COLOR_BK_IN,MIN_BTN_COLOR);
	m_btnMIN.SetColor(CButtonST::BTNST_COLOR_FG_IN,MIN_BTN_COLOR);
	m_btnMIN.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MIN_BTN_COLOR);
	m_btnMIN.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MIN_BTN_COLOR);
	m_btnMIN.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,MIN_BTN_COLOR);
	m_btnMIN.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,MIN_BTN_COLOR);

	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_IN,MIN_BTN_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_FG_IN,MIN_BTN_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_OUT,MIN_BTN_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_FG_OUT,MIN_BTN_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,MIN_BTN_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,MIN_BTN_COLOR);

	m_btnMIN.SetTooltipText(_T("Hide this window"));
	m_btnClose.SetTooltipText(_T("Exit YaSync"));

	InitializeCriticalSectionAndSpinCount(&cs4TrayDlg,0x80000400);

	CRect rs;
	GetClientRect(rs);

	rs.left += 197;
	rs.top += 30;
	//rs.right -= 5;
	//rs.bottom -= 5;

	//m_DevCmdDlg.MoveWindow(rs);
	m_homeUI.MoveWindow(rs);
	m_settingUI.MoveWindow(rs);
	m_syncUI.MoveWindow(rs);
	m_helpUI.MoveWindow(rs);
	m_buyUI.MoveWindow(rs);

	SetWindowText(YASYNC_MAIN_WIN_NAME);

	m_settingUI.ShowWindow(SW_HIDE);
	m_syncUI.ShowWindow(SW_HIDE);
	m_helpUI.ShowWindow(SW_HIDE);
	m_buyUI.ShowWindow(SW_HIDE);
	m_homeUI.ShowWindow(SW_SHOW);
	m_trayDlg.ShowWindow(SW_HIDE);
	SetBtnColor(0);

	if (PEIsRegistered() == PE_RET_OK)
	{
		m_btnBuy.ShowWindow(SW_HIDE);
		return TRUE;
	}

	int iDays = GetTrailDays();
	int iShowBuyPage = 0;
	if (iDays >= MAX_EXPIRE_DAY || iDays < 0)
	{
		DeviceAgent::ReportAppState(_T("Expired"));
		MessageBox(_T("YaSync has expired,please regeister first"),_T("Error"),MB_ICONSTOP|MB_OK);
		iShowBuyPage = 1;
	}
	else if (iDays && (iDays % (MAX_EXPIRE_DAY/3) == 0) )
	{
		//DeviceAgent::ReportAppState(_T("AskBuy"));
		CString sInfo;
		iDays = MAX_EXPIRE_DAY - iDays;
		sInfo.Format(_T("YaSync will expire after %d days:)"),iDays);
		DeviceAgent::ReportAppState(_T("AskBuy"));
		MessageBox(sInfo,_T("Info"),MB_ICONINFORMATION|MB_OK);
		iShowBuyPage = 1;
	}

	if (iShowBuyPage == 1)
	{
		m_btnBuy.PostMessage(WM_LBUTTONDOWN ,4,4);
		m_btnBuy.PostMessage(WM_LBUTTONUP,4,4);
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CYaSyncDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CYaSyncDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


HBRUSH CYaSyncDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor == CTLCOLOR_STATIC)  
	{  
		pDC->SetTextColor(RGB(0,0,0));  
		pDC->SetBkColor(DLG_BK_COLOR);  
		return m_hDialogBrush;
	}
	else if (nCtlColor == CTLCOLOR_DLG)
	{
		//pDC->SetTextColor(COLOR_DLG_VC);
		return m_hDialogBrush;
	}

	return m_hDialogBrush;
}

void CYaSyncDlg::OnBnClickedMinBtn()
{
	ShowWindow(SW_HIDE);
}


BOOL CYaSyncDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CYaSyncDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

void CYaSyncDlg::OnBnClickedHomeBtn()
{
	PECore::PostMessage(WM_GET_DEV_INFO,0,0);

	SetBtnColor(0);

	m_settingUI.ShowWindow(SW_HIDE);
	m_syncUI.ShowWindow(SW_HIDE);
	m_helpUI.ShowWindow(SW_HIDE);
	m_buyUI.ShowWindow(SW_HIDE);
	m_homeUI.ShowWindow(SW_SHOW);
}

void CYaSyncDlg::OnBnClickedSyncBtn()
{
	SetBtnColor(1);

	m_homeUI.ShowWindow(SW_HIDE);
	m_settingUI.ShowWindow(SW_HIDE);
	m_helpUI.ShowWindow(SW_HIDE);
	m_buyUI.ShowWindow(SW_HIDE);

	m_syncUI.ShowWindow(SW_SHOW);
	m_syncUI.LoadSetting();
}

void CYaSyncDlg::OnBnClickedSettingBtn()
{
	m_settingUI.OnDeviceConnected();

	SetBtnColor(2);
	m_homeUI.ShowWindow(SW_HIDE);
	m_syncUI.ShowWindow(SW_HIDE);
	m_helpUI.ShowWindow(SW_HIDE);
	m_buyUI.ShowWindow(SW_HIDE);

	m_settingUI.ShowWindow(SW_SHOW);
}

void CYaSyncDlg::OnBnClickedHelpBtn()
{
	SetBtnColor(3);
	m_homeUI.ShowWindow(SW_HIDE);
	m_settingUI.ShowWindow(SW_HIDE);
	m_syncUI.ShowWindow(SW_HIDE);
	m_helpUI.ShowWindow(SW_SHOW);
	m_buyUI.ShowWindow(SW_HIDE);
}

void CYaSyncDlg::OnBnClickedBuyBtn()
{
	SetBtnColor(4);
	m_homeUI.ShowWindow(SW_HIDE);
	m_settingUI.ShowWindow(SW_HIDE);
	m_syncUI.ShowWindow(SW_HIDE);
	m_helpUI.ShowWindow(SW_HIDE);
	m_buyUI.ShowWindow(SW_SHOW);
}


BOOL CYaSyncDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	switch (message)
	{
	case WM_NOTIFYICON:
		{
			if (lParam == WM_LBUTTONDOWN)
			{
				m_iAutoStart = 0;

#if 1
				SetForegroundWindow();
				ShowWindow(SW_SHOW);
#else
				m_trayDlg.SetForegroundWindow();
				m_trayDlg.ShowWindow(SW_SHOW);
#endif				
			}
			else if (lParam == WM_RBUTTONDOWN)
			{
				m_iAutoStart = 0;
				CMenu t_Menu,*pSubMenu;
				t_Menu.LoadMenu(IDR__TRAY_MENU);

				CPoint Point;
				GetCursorPos(&Point);
				SetForegroundWindow();
				pSubMenu = t_Menu.GetSubMenu(0);//获取第一个弹出菜单，所以第一个菜单必须有子菜单
				pSubMenu->TrackPopupMenu(TPM_LEFTALIGN,Point.x,Point.y,this);//在指定位置显示弹出菜单
			}

			break;
		}
	case WM_SYNC_INFO:
		{
			PE_SYNC_STATUS *s = (PE_SYNC_STATUS*)wParam;
			m_syncUI.OnSyncInfo(s);
			break;
		}
	case WM_SYNC_STARTED:
		{
			m_syncUI.OnSyncProcess(1);
			break;
		}
	case WM_SYNC_DONE:
		{
			m_syncUI.OnSyncProcess(0);
			break;
		}
	case WM_ASK_FOR_REG:
		{
			MessageBox(_T("YaSync has expired,please regeister first"),_T("YaSync needs your support"),MB_ICONSTOP|MB_OK);
			OnBnClickedBuyBtn();
			break;
		}
	case WM_SHOW_INSTALL_UI:
		{
			CInstallDlg dlg;
			dlg.DoModal();
			break;
		}
	case WM_WRONG_SECURITY_CODE:
		{
			PE_PHONE_SETTING* s = (PE_PHONE_SETTING*)wParam;
			if (PE_RET_OK == CEnterSecurityCodeDlg::IsItemInFilterList(s->szPhoneID))
			{
				HANDLE hEvt = (HANDLE)lParam;
				SetEvent(hEvt);
				break;
			}

			CEnterSecurityCodeDlg *pDlg = new CEnterSecurityCodeDlg();
			pDlg->m_sPhoneID = s->szPhoneID;
			pDlg->m_sPhoneName = s->szPhoneName;

			pDlg->DoModal();

			HANDLE hEvt = (HANDLE)lParam;
			SetEvent(hEvt);

			delete pDlg;

			break;
		}
	case WM_WRONG_PC_VERSION:
		{
			static int iReported = 0;
			if (iReported == 0)
			{
				++iReported;
				//PE_PHONE_SETTING* s = (PE_PHONE_SETTING*)wParam;
				if (IDOK == MessageBox(_T("Please update YaSync on PC first,click ok to download now"),_T("Please update YaSync"),MB_ICONEXCLAMATION|MB_OKCANCEL))
				{
					//ShellExecute(0, _T("open"), _T("https://play.google.com/store/apps/details?id=mobitnt.android.YaSync"), NULL, NULL, SW_SHOWNORMAL);
					DeviceAgent::ReportAppState(_T("AskUpdate"));
					ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/InstallYaSync.php"), NULL, NULL, SW_SHOWNORMAL);
				}
			}

			//HANDLE hEvt = (HANDLE)lParam;
			//SetEvent(hEvt);	
			break;
		}
	case WM_WRONG_CLIENT_VERSION:
		{
			static int iReported = 0;
			if (iReported == 0)
			{
				++iReported;
				if (IDOK == MessageBox(_T("Please update YaSync client on Android,click ok to install"),_T("Please update YaSync Client"),MB_ICONEXCLAMATION|MB_OKCANCEL))
				{
					//ShellExecute(0, _T("open"), _T("https://play.google.com/store/apps/details?id=mobitnt.android.YaSync"), NULL, NULL, SW_SHOWNORMAL);
					ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/yasync-quick-start.htm"), NULL, NULL, SW_SHOWNORMAL);
				}
			}

			//HANDLE hEvt = (HANDLE)lParam;
			//SetEvent(hEvt);	
			break;
		}
	case WM_ASK_INSTALL_CLIENT:
		{
			static int iAsked = 0;
			if (iAsked == 0)
			{
				++iAsked;
				PE_PHONE_SETTING* s = (PE_PHONE_SETTING*)wParam;
				if (IDOK == MessageBox(_T("Click Ok to install YaSync Android client from Google Play now"),_T("Please Install YaSync Client"),MB_ICONEXCLAMATION|MB_OKCANCEL))
				{
					ShellExecute(0, _T("open"), _T("https://play.google.com/store/apps/details?id=mobitnt.android.YaSync"), NULL, NULL, SW_SHOWNORMAL);
				}
			}
			break;
		}
	case WM_SYNC_CANCELLED:
		{
			break;
		}
	case WM_STOP_SRV2:
		{
			OnBnClickedCloseBtn();
			break;
		}
	case WM_NEW_VER_FOUND:
		{
			if (IDOK == MessageBox(_T("A new version of YaSync is available,do you want to download now?"),_T("New Version Found"),MB_ICONEXCLAMATION |MB_OKCANCEL))
			{
				DeviceAgent::ReportAppState(_T("AskUpdate"));
				ShellExecute(NULL,_T("open"),_T("http://www.mobitnt.com/InstallYaSync.php?src=insideApp"),NULL,NULL,SW_SHOW);
			}
			break;
		}
	case WM_ADDIN_CMD:
		{
			DWORD dwCmd = (DWORD)wParam;
			TCHAR *szData = (TCHAR*)lParam;
			OnOutlookCmd(dwCmd,szData);
			break;
		}
	case WM_SHOW_ERROR_ON_UI:
		{
			TCHAR *szMsg = (TCHAR*)wParam;
			MessageBox((TCHAR*)wParam,_T("Error"),MB_ICONSTOP|MB_OK);
			free(szMsg);

			break;
		}
	case WM_SHOW_TRAY_INFO:
		{
			m_trayDlg.ShowMsg((TRAY_MSG *)wParam);
			break;
		}
	default:
		return CDialog::OnWndMsg(message, wParam, lParam, pResult);
	}

	return true;
}


void CYaSyncDlg::OnNcPaint()
{
	if (m_iAutoStart)
	{
		ShowWindow(SW_HIDE);
		--m_iAutoStart;
		return;
	}

	CDialog::OnNcPaint();
}

void CYaSyncDlg::OnAaaShowmainui()
{
	SetForegroundWindow();
	ShowWindow(SW_SHOW);
}

void CYaSyncDlg::OnAaaNewsms()
{
	m_homeUI.OnBnClickedBtnNewSms();
}

void CYaSyncDlg::OnAaaFilemanager()
{
	m_homeUI.StartUI(PE_UI_TYPE_FILE);
}

void CYaSyncDlg::OnAaaPhotoviewer()
{
	m_homeUI.StartUI(PE_UI_TYPE_PHOTO);
}

void CYaSyncDlg::OnAaaExit()
{
	OnBnClickedCloseBtn();
}

void CYaSyncDlg::OnAaaScandevice()
{
	PECore::PostMessage(WM_SCAN_DEVICE,0,0);
}

void CYaSyncDlg::OnBnClickedCloseBtn()
{
	m_trayDlg.OnClose();
	ShowWindow(SW_HIDE);
	PEStopSvr();
	CEnterSecurityCodeDlg::FreeFilterList();
	OnOK();
}


extern void SetThreadEntryID(TCHAR *szID);

extern int g_iOutlookCmd;

void CYaSyncDlg::OnOutlookCmd(DWORD dwCmd,TCHAR *szEntryID)
{
	TCHAR szName[255];
	TCHAR szPhone[255];
	memset(szName,0,sizeof(TCHAR)*255);
	memset(szPhone,0,sizeof(TCHAR)*255);

	PEWriteLog(_T("Cmd from outlook received"));

	g_iOutlookCmd = 1;

	switch (dwCmd)
	{
	case PE_CMD_SHOW_UI:
		{
			OnAaaShowmainui();
			break;
		}
	case PE_CMD_NEW_SMS:
		{
			PE_PHONE_SETTING* s = PECore::GetCurPhone();
			if (s == NULL)
			{
				ShowTrayInfo(_T("Device is not connected"));
				return;
			}

			m_homeUI.OnBnClickedBtnNewSms();
			break;
		}
	case PE_CMD_HELP:
		{
			OnAaaShowmainui();
			SetWinOnOutlook(m_hWnd);
			OnBnClickedHelpBtn();
			break;
		}
	case PE_CMD_THREAD:
		{
			if (szEntryID)
			{
				SetThreadEntryID(szEntryID);
				PECore::StartUI(PE_CMD_THREAD,0);
			}
			break;
		}
	case PE_CMD_DELETE:
		{
			PECore::SendMessage(WM_DELETE_ITEM,(WPARAM)szEntryID);
			break;
		}
	case PE_CMD_SAVE_CONTACT:
		{
			MSG_Data_Truck *pData = (MSG_Data_Truck*)calloc(1,sizeof(MSG_Data_Truck));
			pData->p1 = (WPARAM)szEntryID;
			pData->p2 = (WPARAM)szName;
			pData->p3 = (WPARAM)szPhone;

			PECore::SendMessage(WM_GET_NAME_PHONE,(WPARAM)pData);

			if (_tcslen(szName) || _tcslen(szPhone))
			{
				CSaveContactsDlg dlg;
				dlg.SetContacts(szName,szPhone);
				dlg.DoModal();
			}
			free(pData);
			break;
		}
	case PE_CMD_FILTER:
		{
			MSG_Data_Truck *pData = (MSG_Data_Truck*)calloc(1,sizeof(MSG_Data_Truck));
			pData->p1 = (WPARAM)szEntryID;
			pData->p2 = (WPARAM)szName;
			pData->p3 = (WPARAM)szPhone;

			PECore::SendMessage(WM_GET_NAME_PHONE,(WPARAM)pData);
			free(pData);

			if (_tcslen(szName) || _tcslen(szPhone))
			{
				OnAaaShowmainui();

				SetWinOnOutlook(m_hWnd);
				OnBnClickedSettingBtn();

				m_settingUI.OnAddFilter(szName,szPhone);
			}

			break;
		}
	case PE_CMD_SYNC:
		{
			OnAaaShowmainui();

			SetWinOnOutlook(m_hWnd);
			m_btnSync.PostMessage(WM_LBUTTONDOWN ,4,4);
			m_btnSync.PostMessage(WM_LBUTTONUP,4,4);
			break;
		}
	case PE_CMD_RESTORE:
		{
			PE_PHONE_SETTING* s = PECore::GetCurPhone();
			if (s == NULL)
			{
				ShowTrayInfo(_T("Device is not connected"));
				return;
			}

			TCHAR *szFolderID = NULL;

			if (szEntryID && _tcslen(szEntryID) > 1)
			{
				szFolderID = _tcsstr(szEntryID,_T(";"));
				if (szFolderID)
				{
					*szFolderID = 0;
					szFolderID += 1;
				}
			}

			CRestoreDlg *pDlg = new CRestoreDlg();
			pDlg->m_szFolderID = szFolderID;
			pDlg->DoModal();
			delete pDlg;

			break;
		}
	case PE_CMD_REPLY:
		{
			PE_PHONE_SETTING* s = PECore::GetCurPhone();
			if (s == NULL)
			{
				ShowTrayInfo(_T("Device is not connected"));
				return;
			}

			MSG_Data_Truck *pData = (MSG_Data_Truck*)calloc(1,sizeof(MSG_Data_Truck));
			pData->p1 = (WPARAM)szEntryID;
			pData->p2 = (WPARAM)szName;
			pData->p3 = (WPARAM)szPhone;

			PECore::SendMessage(WM_GET_NAME_PHONE,(WPARAM)pData);
			free(pData);
			if (!_tcslen(szName) && !_tcslen(szPhone))
			{
				break;
			}

			TCHAR szReceiver[1024];

			if (_tcslen(szName))
			{
				_stprintf(szReceiver,_T("%s"),szName);
			}
			else if (_tcslen(szPhone))
			{
				_stprintf(szReceiver,_T("<%s>"),szPhone);
			}

			CNewSmsDlg dlg;
			_tcscpy(dlg.m_szReceiver, szReceiver);
			dlg.DoModal();

			break;
		}
	case PE_CMD_STAT:
		{
			PECore::SendMessage(WM_STAT,(WPARAM)szEntryID);

			break;
		}
	case PE_CMD_FILE_TRANSFER:
		{
			PE_PHONE_SETTING* s = PECore::GetCurPhone();
			if (s == NULL)
			{
				ShowTrayInfo(_T("Device is not connected"));
				return;
			}

			m_homeUI.OnBnClickedBtnFile();
			break;
		}
	case PE_CMD_LOG:
		{
			PEWriteLog(szEntryID);
			break;
		}
	}

	free(szEntryID);

}


BOOL CYaSyncDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCDS)
{
	PEWriteLog(_T("OnCopyData,Cmd from outlook received"));

	DWORD dwCmd = (DWORD)pCDS->dwData;
	void *pData = pCDS->lpData;
	int iDataLen = pCDS->cbData;

	TCHAR *szEntryID = NULL;
	if (iDataLen > 0 && pData)
	{
		szEntryID = (TCHAR *)calloc(iDataLen + 1,sizeof(TCHAR));
		memcpy(szEntryID,pData,iDataLen);
		PEWriteLog(szEntryID);
	}

	PostMessage(WM_ADDIN_CMD,(WPARAM)dwCmd,(LPARAM)szEntryID);

	return TRUE;
}


