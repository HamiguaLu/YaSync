// SettingCommonUI.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "SettingCommonUI.h"
#include "SettingHelper.h"
#include "PECore.h"

// CSettingCommonUI dialog

IMPLEMENT_DYNAMIC(CSettingCommonUI, CDialog)

extern HWND g_hSettingUI;


CSettingCommonUI::CSettingCommonUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingCommonUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingCommonUI::~CSettingCommonUI()
{
}

void CSettingCommonUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OUTLOOK_PROFILE, m_profileList);
}


BEGIN_MESSAGE_MAP(CSettingCommonUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_AUTO_START, &CSettingCommonUI::OnBnClickedAutoStart)
	ON_BN_CLICKED(IDC_AUTO_SYNC, &CSettingCommonUI::OnBnClickedAutoSync)
	ON_BN_CLICKED(IDC_CHECK_USE_DEFAULT_PROFILE, &CSettingCommonUI::OnBnClickedCheckUseDefaultProfile)
	ON_CBN_SELCHANGE(IDC_OUTLOOK_PROFILE, &CSettingCommonUI::OnCbnSelchangeOutlookProfile)
	ON_BN_CLICKED(IDC_CHECK_EXIT_AFTER_SYNC, &CSettingCommonUI::OnBnClickedCheckExitAfterSync)
END_MESSAGE_MAP()


// CSettingCommonUI message handlers

HBRUSH CSettingCommonUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSettingCommonUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_USE_DEFAULT_PROFILE);
	pBtn->SetCheck(1);

	LoadSetting();

	PECore::PostMessage(WM_GET_MSG_STORE,(WPARAM)m_hWnd,0);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSettingCommonUI::LoadSetting()
{
	PE_COMMON_SETTING *setting = new PE_COMMON_SETTING();

	CSettingHelper::GetCommonSetting(setting);

	if (setting->dwAutoStart)
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_AUTO_START);
		pBtn->SetCheck(1);
	}
	else
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_AUTO_START);
		pBtn->SetCheck(0);
	}

	if (setting->dwAutoSync)
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_AUTO_SYNC);
		pBtn->SetCheck(1);
	}
	else
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_AUTO_SYNC);
		pBtn->SetCheck(0);
	}

	if (setting->dwExitAfterSync)
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_EXIT_AFTER_SYNC);
		pBtn->SetCheck(1);
	}
	else
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_EXIT_AFTER_SYNC);
		pBtn->SetCheck(0);
	}

	if (setting->dwUseDefaultPst)
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_USE_DEFAULT_PROFILE);
		pBtn->SetCheck(1);
		m_profileList.ShowWindow(SW_HIDE);
	}
	else
	{
		CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_USE_DEFAULT_PROFILE);
		pBtn->SetCheck(0);
		m_profileList.ShowWindow(SW_SHOW);
	}

	delete setting;

}



void CSettingCommonUI::OnSave()
{
	PE_COMMON_SETTING *setting = new PE_COMMON_SETTING();

	CSettingHelper::GetCommonSetting(setting);

	CButton* pBtn = (CButton*)GetDlgItem(IDC_AUTO_START);
	setting->dwAutoStart = pBtn->GetCheck();

	CButton* pBtn1 = (CButton*)GetDlgItem(IDC_AUTO_SYNC);
	setting->dwAutoSync = pBtn1->GetCheck();

	CButton* pBtn2 = (CButton*)GetDlgItem(IDC_CHECK_EXIT_AFTER_SYNC);
	setting->dwExitAfterSync = pBtn2->GetCheck();

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_CHECK_USE_DEFAULT_PROFILE);
	setting->dwUseDefaultPst = pBtn3->GetCheck();

	if (!setting->dwUseDefaultPst)
	{
		CString sProfile;
		m_profileList.GetWindowText(sProfile);
		if (sProfile.GetLength() < 1)
		{
			MessageBox(_T("Please selet a outlook profile"),_T("Error"),MB_ICONSTOP|MB_OK);
			delete setting;
			return;
		}

		_tcscpy(setting->szPst,sProfile.GetBuffer());
		sProfile.ReleaseBuffer();
	}

	CSettingHelper::SetCommonSetting(setting);
	CSettingHelper::SetAutoStartApp(setting->dwAutoStart);

	delete setting;

}

void CSettingCommonUI::OnBnClickedAutoStart()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,11,0);
}

void CSettingCommonUI::OnBnClickedAutoSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,12,0);
}

void CSettingCommonUI::OnBnClickedCheckUseDefaultProfile()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,13,0);

	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_USE_DEFAULT_PROFILE);
	if (pBtn->GetCheck())
	{
		m_profileList.ShowWindow(SW_HIDE);
	}
	else
	{
		m_profileList.ShowWindow(SW_SHOW);
	}
}

void CSettingCommonUI::OnCbnSelchangeOutlookProfile()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,14,0);
}

void CSettingCommonUI::OnBnClickedCheckExitAfterSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,15,0);
}

BOOL CSettingCommonUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_MSG_STORE_COMING == message)
	{
		list<TCHAR*> *pList = (list<TCHAR*> *)wParam;

		m_profileList.ResetContent();

		list<TCHAR*>::iterator it;
		for (it = pList->begin(); it != pList->end(); ++it)
		{ 
			TCHAR *szName = (TCHAR*)*it;
			m_profileList.AddString(szName);
		}

		PE_COMMON_SETTING *setting = new PE_COMMON_SETTING();
		CSettingHelper::GetCommonSetting(setting);

		if (_tcslen(setting->szPst))
		{
			m_profileList.SelectString(0,setting->szPst);
		}

		delete setting;

		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}


