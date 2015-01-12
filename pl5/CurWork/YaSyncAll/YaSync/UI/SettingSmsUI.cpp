// SettingSmsUI.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "SettingSmsUI.h"
#include "PECore.h"
#include "SettingHelper.h"


// CSettingSmsUI dialog

IMPLEMENT_DYNAMIC(CSettingSmsUI, CDialog)

extern HWND g_hSettingUI;


CSettingSmsUI::CSettingSmsUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingSmsUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingSmsUI::~CSettingSmsUI()
{
}

void CSettingSmsUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SMS_FOLDER_LIST, m_SMSFolderList);
}


BEGIN_MESSAGE_MAP(CSettingSmsUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ENABLE_SMS_SYNC, &CSettingSmsUI::OnBnClickedEnableSmsSync)
	ON_BN_CLICKED(IDC_ENABLE_MMS_SYNC, &CSettingSmsUI::OnBnClickedEnableMmsSync)
	ON_BN_CLICKED(IDC_STORE_SMS_TO_FOLDER, &CSettingSmsUI::OnBnClickedStoreSmsToFolder)
	ON_CBN_SELCHANGE(IDC_SMS_FOLDER_LIST, &CSettingSmsUI::OnCbnSelchangeSmsFolderList)
	ON_BN_CLICKED(IDC_CHECK_SMS_READ_FLAG, &CSettingSmsUI::OnBnClickedCheckSmsReadFlag)
	ON_BN_CLICKED(IDC_SHOW_SMS_REPORT, &CSettingSmsUI::OnBnClickedShowSmsReport)
END_MESSAGE_MAP()


// CSettingSmsUI message handlers

HBRUSH CSettingSmsUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSettingSmsUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingSmsUI::OnSave()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_SMS_SYNC);
	s->dwSyncSMS = pBtn->GetCheck();

	CButton* pBtn1 = (CButton*)GetDlgItem(IDC_ENABLE_MMS_SYNC);
	s->dwSyncMMS = pBtn1->GetCheck();

	CButton* pBtn2 = (CButton*)GetDlgItem(IDC_CHECK_SMS_READ_FLAG);
	s->dwSetReadFlag = pBtn2->GetCheck();

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_STORE_SMS_TO_FOLDER);
	if (pBtn3->GetCheck())
	{
		CString sTxt;
		m_SMSFolderList.GetWindowText(sTxt);
		_tcscpy(s->szSMSFolderName,sTxt.GetBuffer());
		sTxt.ReleaseBuffer();
	}
	else
	{
		s->szSMSFolderName[0] = 0;
	}

	CButton* pBtn4 = (CButton*)GetDlgItem(IDC_SHOW_SMS_REPORT);
	s->dwShowSMSReprot = pBtn4->GetCheck();
}

void CSettingSmsUI::OnDeviceConnected()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_SMS_SYNC);
	pBtn->SetCheck(s->dwSyncSMS);

	CButton* pBtn1 = (CButton*)GetDlgItem(IDC_ENABLE_MMS_SYNC);
	pBtn1->SetCheck(s->dwSyncMMS);

	CButton* pBtn2 = (CButton*)GetDlgItem(IDC_CHECK_SMS_READ_FLAG);
	pBtn2->SetCheck(s->dwSetReadFlag);

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_STORE_SMS_TO_FOLDER);
	if (_tcslen(s->szSMSFolderName) > 0)
	{
		PECore::PostMessage(WM_GET_SUB_FOLDER,FOLDER_TYPE_SMS,(WPARAM)m_hWnd);
		pBtn3->SetCheck(1);
		m_SMSFolderList.ShowWindow(SW_SHOW);
	}
	else
	{
		pBtn3->SetCheck(0);
		m_SMSFolderList.ShowWindow(SW_HIDE);
	}

	CButton* pBtn4 = (CButton*)GetDlgItem(IDC_SHOW_SMS_REPORT);
	pBtn4->SetCheck(s->dwShowSMSReprot);

}

void CSettingSmsUI::OnBnClickedEnableSmsSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,23,0);
}

void CSettingSmsUI::OnBnClickedEnableMmsSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,41,0);
}

void CSettingSmsUI::OnBnClickedStoreSmsToFolder()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,51,0);

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_STORE_SMS_TO_FOLDER);
	if (pBtn3->GetCheck())
	{
		PECore::PostMessage(WM_GET_SUB_FOLDER,FOLDER_TYPE_SMS,(WPARAM)m_hWnd);
		m_SMSFolderList.ShowWindow(SW_SHOW);
	}
	else
	{
		m_SMSFolderList.ShowWindow(SW_HIDE);
	}
}

void CSettingSmsUI::OnCbnSelchangeSmsFolderList()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,61,0);
}

void CSettingSmsUI::OnBnClickedCheckSmsReadFlag()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,71,0);
}

void CSettingSmsUI::OnSubFolderListComing(list<TCHAR*> *pList)
{
	m_SMSFolderList.ResetContent();
	list<TCHAR*>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;
		m_SMSFolderList.AddString(szName);
	}

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	if (_tcslen(s->szSMSFolderName))
	{
		m_SMSFolderList.SelectString(0,s->szSMSFolderName);
	}
	else
	{
		m_SMSFolderList.SetCurSel(0);
	}

}

BOOL CSettingSmsUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_SUB_FOLDER_COMING)
	{
		list<TCHAR*> *pList = (list<TCHAR*> *)wParam;
		OnSubFolderListComing(pList);
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

void CSettingSmsUI::OnBnClickedShowSmsReport()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,81,0);
}
