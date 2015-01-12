// SettingCallLogUI.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "SettingCallLogUI.h"
#include "PECore.h"
#include "SettingHelper.h"

// CSettingCallLogUI dialog

extern HWND g_hSettingUI;

IMPLEMENT_DYNAMIC(CSettingCallLogUI, CDialog)

CSettingCallLogUI::CSettingCallLogUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingCallLogUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingCallLogUI::~CSettingCallLogUI()
{
}

void CSettingCallLogUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CALL_LOG_FOLDER, m_CallLogFolderList);
}


BEGIN_MESSAGE_MAP(CSettingCallLogUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ENABLE_CALLLOG_SYNC, &CSettingCallLogUI::OnBnClickedEnableCalllogSync)
	ON_BN_CLICKED(IDC_ENABLE_CALL_LOG_FOLDER, &CSettingCallLogUI::OnBnClickedEnableCallLogFolder)
	ON_CBN_SELCHANGE(IDC_CALL_LOG_FOLDER, &CSettingCallLogUI::OnCbnSelchangeCallLogFolder)
END_MESSAGE_MAP()


// CSettingCallLogUI message handlers

HBRUSH CSettingCallLogUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSettingCallLogUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingCallLogUI::OnSave()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CALLLOG_SYNC);
	s->dwSyncCall = pBtn->GetCheck();

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_ENABLE_CALL_LOG_FOLDER);
	if (pBtn3->GetCheck())
	{
		CString sTxt;
		m_CallLogFolderList.GetWindowText(sTxt);
		_tcscpy(s->szCallFolderName,sTxt.GetBuffer());
		sTxt.ReleaseBuffer();
	}
	else
	{
		s->szCallFolderName[0] = 0;
	}
}


void CSettingCallLogUI::OnDeviceConnected()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CALLLOG_SYNC);
	pBtn->SetCheck(s->dwSyncCall);

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_ENABLE_CALL_LOG_FOLDER);
	if (_tcslen(s->szCallFolderName) > 0)
	{
		PECore::PostMessage(WM_GET_SUB_FOLDER,FOLDER_TYPE_CALL_LOG,(WPARAM)m_hWnd);
		pBtn3->SetCheck(1);
		m_CallLogFolderList.ShowWindow(SW_SHOW);
	}
	else
	{
		pBtn3->SetCheck(0);
		m_CallLogFolderList.ShowWindow(SW_HIDE);
	}
}


void CSettingCallLogUI::OnBnClickedEnableCalllogSync()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,8,0);
}

void CSettingCallLogUI::OnBnClickedEnableCallLogFolder()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,9,0);

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn3 = (CButton*)GetDlgItem(IDC_ENABLE_CALL_LOG_FOLDER);
	if (pBtn3->GetCheck())
	{
		PECore::PostMessage(WM_GET_SUB_FOLDER,FOLDER_TYPE_CALL_LOG,(WPARAM)m_hWnd);
		m_CallLogFolderList.ShowWindow(SW_SHOW);
	}
	else
	{
		m_CallLogFolderList.ShowWindow(SW_HIDE);
	}
}

void CSettingCallLogUI::OnCbnSelchangeCallLogFolder()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,10,0);
}


void CSettingCallLogUI::OnSubFolderListComing(list<TCHAR*> *pList)
{
	m_CallLogFolderList.ResetContent();
	list<TCHAR*>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;
		m_CallLogFolderList.AddString(szName);
	}

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	if (_tcslen(s->szCallFolderName))
	{
		m_CallLogFolderList.SelectString(0,s->szCallFolderName);
	}
	else
	{
		m_CallLogFolderList.SetCurSel(0);
	}

}

BOOL CSettingCallLogUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_SUB_FOLDER_COMING)
	{
		list<TCHAR*> *pList = (list<TCHAR*> *)wParam;
		OnSubFolderListComing(pList);
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}
