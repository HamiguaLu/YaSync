// SettingUI.cpp : implementation file
//

#include "stdafx.h"
#include "PEProtocol.h"
#include "YaSync.h"
#include "SettingUI.h"
#include "PECore.h"
#include "SettingHelper.h"

// CSettingUI dialog

IMPLEMENT_DYNAMIC(CSettingUI, CDialog)

CSettingUI::CSettingUI(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingUI::~CSettingUI()
{
}

void CSettingUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SETTING_TAB, m_tabDlgList);
}


BEGIN_MESSAGE_MAP(CSettingUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(TCN_SELCHANGE, IDC_SETTING_TAB, &CSettingUI::OnTcnSelchangeSettingTab)
	ON_BN_CLICKED(IDC_BTN_SAVE_SETTING, &CSettingUI::OnBnClickedBtnSaveSetting)
	ON_BN_CLICKED(IDC_BTN_RESET_SETTING, &CSettingUI::OnBnClickedBtnResetSetting)
END_MESSAGE_MAP()


// CSettingUI message handlers

HBRUSH CSettingUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor   ==   CTLCOLOR_STATIC)  
	{  
		pDC->SetTextColor(RGB(0,0,0));  
		pDC->SetBkColor(SUB_DLG_BK_COLOR);  
		return m_hDialogBrush;
	}
	else if (nCtlColor == CTLCOLOR_DLG)
	{
		//pDC->SetTextColor(COLOR_DLG_VC);
		return m_hDialogBrush;
	}
	return m_hDialogBrush;
}

HWND g_hSettingUI = 0;
BOOL CSettingUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnSave.SubclassDlgItem(IDC_BTN_SAVE_SETTING, this);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);
	m_btnSave.EnableWindow(0);

	m_btnReset.SubclassDlgItem(IDC_BTN_RESET_SETTING, this);
	m_btnReset.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnReset.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnReset.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnReset.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnReset.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnReset.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_tabDlgList.InsertItem(0,_T("Common"));
	m_tabDlgList.InsertItem(1,_T("Contact"));
	m_tabDlgList.InsertItem(2,_T("Calendar"));
	m_tabDlgList.InsertItem(3,_T("SMS"));
	m_tabDlgList.InsertItem(4,_T("Call Log"));
	m_tabDlgList.InsertItem(5,_T("Media"));
	m_tabDlgList.InsertItem(6,_T("Black List"));
	m_tabDlgList.InsertItem(7,_T("Blocked Device"));

	//初始化建立属性页各页
	m_CommonUI.Create(IDD_SETTING_COMMON_UI,GetDlgItem(IDC_SETTING_TAB));
	m_ContactUI.Create(IDD_SETTING_CONTACT_UI,GetDlgItem(IDC_SETTING_TAB));
	m_CalendarUI.Create(IDD_SETTING_CALENDAR_UI,GetDlgItem(IDC_SETTING_TAB));
	m_SmsUI.Create(IDD_SETTING_SMS_UI,GetDlgItem(IDC_SETTING_TAB));
	m_CallLogUI.Create(IDD_SETTING_CALLLOG_UI,GetDlgItem(IDC_SETTING_TAB));
	m_FilterUI.Create(IDD_SETTING_FILTER,GetDlgItem(IDC_SETTING_TAB));
	m_DeviceUI.Create(IDD_SETTING_DEVICE_UI,GetDlgItem(IDC_SETTING_TAB));
	m_MediaUI.Create(IDD_SETTING_MEDIA,GetDlgItem(IDC_SETTING_TAB));

	//设置页面的位置在m_tab控件范围内
	CRect rs;
	m_tabDlgList.GetClientRect(rs);
	rs.top+=20;
	rs.bottom-=4;
	rs.left+=4;
	rs.right-=4;

	g_hSettingUI = m_hWnd;

	//m_DevCmdDlg.MoveWindow(rs);
	m_CommonUI.MoveWindow(rs);
	m_ContactUI.MoveWindow(rs);
	m_CalendarUI.MoveWindow(rs);
	m_SmsUI.MoveWindow(rs);
	m_CallLogUI.MoveWindow(rs);
	m_DeviceUI.MoveWindow(rs);
	m_FilterUI.MoveWindow(rs);
	m_MediaUI.MoveWindow(rs);

	m_CommonUI.ShowWindow(SW_SHOW);
	m_tabDlgList.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingUI::OnTcnSelchangeSettingTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int CurSel = m_tabDlgList.GetCurSel();

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if ( !s && CurSel != 0 && CurSel != 6 && CurSel != 7)
	{
		MessageBox(_T("Can not change phone related settings when no phone is connected"),_T("Error"),MB_ICONSTOP|MB_OK);
		CurSel = 0;
		m_tabDlgList.SetCurSel(0);
	}

	switch(CurSel)
	{
	case 0:
		{
			m_CommonUI.ShowWindow(SW_SHOW);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_HIDE);
			
			break;
		}
	case 1:
		{
			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_SHOW);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_HIDE);
			break;
		}
	case 2:
		{
			//calendar 
			if (s->dwSDKVer < 14)
			{
				MessageBox(_T("Sorry,Calendar Sync doesn't work below android 4.0"),_T("Error"),MB_ICONSTOP|MB_OK);
				s->dwSyncCalendar = 0;
				CSettingHelper::SetPhoneSetting(s);
				break;
			}

			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_SHOW);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_HIDE);
			break;
		}
	case 3:
		{
			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_SHOW);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_HIDE);
			break;
		}
	case 4:
		{
			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_SHOW);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_HIDE);
			break;
		}
	case 5:
		{
			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_SHOW);
			m_FilterUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_HIDE);
			
			break;
		}
	case 6:
		{
			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_SHOW);
			m_DeviceUI.ShowWindow(SW_HIDE);
			break;
		}
	case 7:
		{
			m_CommonUI.ShowWindow(SW_HIDE);
			m_ContactUI.ShowWindow(SW_HIDE);
			m_CalendarUI.ShowWindow(SW_HIDE);
			m_SmsUI.ShowWindow(SW_HIDE);
			m_CallLogUI.ShowWindow(SW_HIDE);
			m_MediaUI.ShowWindow(SW_HIDE);
			m_FilterUI.ShowWindow(SW_HIDE);
			m_DeviceUI.ShowWindow(SW_SHOW);
			break;
		}
	default: ;
	}

	*pResult = 0;
}


void CSettingUI::OnAddFilter(TCHAR *szName,TCHAR *szPhone)
{
	m_tabDlgList.SetCurSel(5);
	LONG_PTR Res;
	OnTcnSelchangeSettingTab(NULL, &Res);
	
	m_FilterUI.SetFilter(szName,szPhone);

}



void CSettingUI::OnDeviceConnected()
{
	m_ContactUI.OnDeviceConnected();
	m_CalendarUI.OnDeviceConnected();
	m_SmsUI.OnDeviceConnected();
	m_CallLogUI.OnDeviceConnected();
	m_MediaUI.OnDeviceConnected();
}



BOOL CSettingUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_ENABLE_SAVE)
	{
		m_btnSave.EnableWindow(1);
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

void CSettingUI::OnBnClickedBtnSaveSetting()
{
	m_btnSave.EnableWindow(0);

	m_CommonUI.OnSave();
	m_FilterUI.OnSave();
	m_DeviceUI.OnSave();

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if ( s)
	{
		m_ContactUI.OnSave();
		m_CalendarUI.OnSave();
		m_SmsUI.OnSave();
		m_CallLogUI.OnSave();
		m_MediaUI.OnSave();

		CSettingHelper::SetPhoneSetting(s);
	}
}

void CSettingUI::OnBnClickedBtnResetSetting()
{
	PE_COMMON_SETTING *setting = new PE_COMMON_SETTING();
	CSettingHelper::GetCommonSetting(setting);
	CSettingHelper::ResetCommonSetting(setting);
	delete setting;

	m_CommonUI.LoadSetting();

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if ( s)
	{
		CSettingHelper::ResetPhoneSetting(s);

		m_ContactUI.OnDeviceConnected();
		m_CalendarUI.OnDeviceConnected();
		m_SmsUI.OnDeviceConnected();
		m_CallLogUI.OnDeviceConnected();
		m_MediaUI.OnDeviceConnected();
	}
}

