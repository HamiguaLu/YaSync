// SettingContactUI.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "SettingContactUI.h"
#include "PECore.h"
#include "SettingHelper.h"
#include "AppTools.h"
#include "UrlEscape.h" 

// CSettingContactUI dialog

IMPLEMENT_DYNAMIC(CSettingContactUI, CDialog)

extern HWND g_hSettingUI;


CSettingContactUI::CSettingContactUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingContactUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingContactUI::~CSettingContactUI()
{
}

void CSettingContactUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_ANDROID_ACCOUNT_LIST, m_AndroidAccountList);
	DDX_Control(pDX, IDC_CONTACT_FOLDER, m_ContactFolderList);

	DDX_Control(pDX, IDC_CONTACT_SYNC_TYPE, m_cbSyncType);
}


BEGIN_MESSAGE_MAP(CSettingContactUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ENABLE_CONTACT_SYNC, &CSettingContactUI::OnBnClickedEnableContactSync)
	ON_CBN_SELCHANGE(IDC_CONTACT_SYNC_TYPE, &CSettingContactUI::OnCbnSelchangeContactSyncType)
	ON_BN_CLICKED(IDC_ENABLE_CONTACT_ACCOUNT, &CSettingContactUI::OnBnClickedEnableContactAccount)
	ON_NOTIFY(NM_CLICK, IDC_ANDROID_ACCOUNT_LIST, &CSettingContactUI::OnNMClickAndroidAccountList)
	ON_NOTIFY(NM_CLICK, IDC_CONTACT_FOLDER, &CSettingContactUI::OnNMClickContactFolder)
END_MESSAGE_MAP()


// CSettingContactUI message handlers

HBRUSH CSettingContactUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSettingContactUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_AndroidAccountList.SetHeadings(_T("Account Name, 290; id,0;"));
	m_AndroidAccountList.SetGridLines(TRUE);
	m_AndroidAccountList.SetCheckboxes(TRUE);
	m_AndroidAccountList.SetFullRowSelect(TRUE);

	m_ContactFolderList.SetHeadings(_T("Folder Name, 290; id,0;"));
	m_ContactFolderList.SetGridLines(TRUE);
	m_ContactFolderList.SetCheckboxes(TRUE);
	m_ContactFolderList.SetFullRowSelect(TRUE);

	m_cbSyncType.ResetContent();
	m_cbSyncType.AddString(_T("Sync both directions"));
	m_cbSyncType.AddString(_T("Sync from phone to outlook"));
	m_cbSyncType.AddString(_T("Sync from outlook to phone"));

	OnBnClickedEnableContactSync();

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CSettingContactUI::OnSave()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CONTACT_SYNC);
	s->dwSyncContact = pBtn->GetCheck();
	
	if (s->dwSyncContact)
	{
		int iSel = m_cbSyncType.GetCurSel();
		if (iSel < 0)
		{
			MessageBox(_T("Please selet the contact sync type first"),_T("Error"),MB_ICONSTOP|MB_OK);
			return;
		}

		s->dwContactSyncType = iSel;
	}

	pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CONTACT_ACCOUNT);
	s->dwEnableContactAccount = pBtn->GetCheck();

	int iCount = m_AndroidAccountList.GetItemCount();

	if (s->dwEnableContactAccount && iCount < 1)
	{
		MessageBox(_T("You choose to sync only selected Android account but no Account was selected,please select at least one Account!"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}
	
	CString sCalIDList = _T("");
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		BOOL bCheck = m_AndroidAccountList.IsItemChecked(iItem);

		if (!bCheck)
		{
			continue;
		}

		CString sCalID = m_AndroidAccountList.GetItemText(iItem, 0);
		sCalIDList += sCalID;
		sCalIDList += _T(";");
	}

	_tcscpy(s->szContactAccounts,sCalIDList.GetBuffer());
	sCalIDList.ReleaseBuffer();

	iCount = m_ContactFolderList.GetItemCount();
	CString sContactFolders = _T("");
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		BOOL bCheck = m_ContactFolderList.IsItemChecked(iItem);

		if (!bCheck)
		{
			continue;
		}

		CString sContactFolder = m_ContactFolderList.GetItemText(iItem, 0);
		if (sContactFolder != CONTACT_ROOT_FOLDER_NAME)
		{
			TCHAR *szFolder = URLEncodeW(sContactFolder.GetBuffer(),sContactFolder.GetLength());
			sContactFolder.ReleaseBuffer();
			sContactFolders += szFolder;
			sContactFolders += _T(";");
			free(szFolder);
		}

		
	}

	_tcscpy(s->szContactFolderNames,sContactFolders.GetBuffer());
	sContactFolders.ReleaseBuffer();
}


void CSettingContactUI::OnDeviceConnected()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CONTACT_SYNC);
	pBtn->SetCheck(s->dwSyncContact);
	if (pBtn->GetCheck())
	{
		GetDlgItem(IDC_CONTACT_SYNC_TYPE)->ShowWindow(SW_SHOW);
		m_cbSyncType.SetCurSel(s->dwContactSyncType);
	}
	else
	{
		GetDlgItem(IDC_CONTACT_SYNC_TYPE)->ShowWindow(SW_HIDE);
	}
	
	CButton* pBtnAccount = (CButton*)GetDlgItem(IDC_ENABLE_CONTACT_ACCOUNT);
	pBtnAccount->SetCheck(s->dwEnableContactAccount);
	
	PECore::PostMessage(WM_GET_ACCOUNTS,(WPARAM)m_hWnd,0);
	PECore::PostMessage(WM_GET_SUB_FOLDER,FOLDER_TYPE_CONTACT,(WPARAM)m_hWnd);
	
}


void CSettingContactUI::OnBnClickedEnableContactSync()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,16,0);

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CONTACT_SYNC);
	s->dwSyncContact = pBtn->GetCheck();
	
	if (s->dwSyncContact)
	{
		m_cbSyncType.ShowWindow(SW_SHOW);
		m_cbSyncType.SelectString(0,_T("Sync both directions"));
	}
	else
	{
		m_cbSyncType.ShowWindow(SW_HIDE);
	}

}

void CSettingContactUI::OnSubFolderListComing(list<TCHAR*> *pList)
{
	m_ContactFolderList.DeleteAllItems();

	list<TCHAR*>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;
		m_ContactFolderList.InsertItem(0,szName,_T("0"));
	}

	//m_ContactFolderList.InsertItem(0,CONTACT_ROOT_FOLDER_NAME,_T("0"));

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	list<TCHAR*> selFolderList = splitString(s->szContactFolderNames,_T(';'));

	for (it = selFolderList.begin(); it != selFolderList.end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;

		TCHAR szRealName[1024];
		URLDecoder(szName,szRealName,1024);
		
		int iCount = m_ContactFolderList.GetItemCount();

		for (int iItem = 0; iItem < iCount; ++iItem)
		{
			CString sCalFolder = m_ContactFolderList.GetItemText(iItem, 0);
			if (_tcscmp(sCalFolder.GetBuffer(),szRealName) == 0)
			{
				m_ContactFolderList.CheckItem(iItem);
			}

			sCalFolder.ReleaseBuffer();
		}
	}

	FreeStringList(selFolderList);

}


extern list<ACCOUNT_ITEM*> g_accountList;
void CSettingContactUI::OnAccountsComing(int iState)
{
	if (iState == 0)
	{
		return;
	}

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	if (g_accountList.size() < 1)
	{
		s->dwEnableContactAccount = 0;
		return;
	}

	m_AndroidAccountList.DeleteAllItems();

	list<ACCOUNT_ITEM*>::iterator it;
	for (it = g_accountList.begin(); it != g_accountList.end(); ++it)
	{ 
		ACCOUNT_ITEM *pCalendar = (ACCOUNT_ITEM*)*it;
		
		m_AndroidAccountList.InsertItem(0,pCalendar->szName,pCalendar->szType);

		free(pCalendar);
	}

	g_accountList.clear();

	list<TCHAR*> selCalIDList = splitString(s->szContactAccounts,_T(';'));
	list<TCHAR*>::iterator it1;
	for (it1 = selCalIDList.begin(); it1 != selCalIDList.end(); ++it1)
	{ 
		TCHAR *szID = (TCHAR*)*it1;

		int iCount = m_AndroidAccountList.GetItemCount();

		for (int iItem = 0; iItem < iCount; ++iItem)
		{
			CString sCalFolder = m_AndroidAccountList.GetItemText(iItem, 0);
			if (_tcscmp(sCalFolder.GetBuffer(),szID) == 0)
			{
				m_AndroidAccountList.CheckItem(iItem);
			}

			sCalFolder.ReleaseBuffer();
		}
	}

	FreeStringList(selCalIDList);
}


void CSettingContactUI::OnCbnSelchangeContactSyncType()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,17,0);
}

void CSettingContactUI::OnBnClickedEnableContactAccount()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,18,0);
}

void CSettingContactUI::OnNMClickAndroidAccountList(NMHDR *pNMHDR, LRESULT *pResult)
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,19,0);
	*pResult = 0;
}

void CSettingContactUI::OnNMClickContactFolder(NMHDR *pNMHDR, LRESULT *pResult)
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,20,0);
	*pResult = 0;
}

BOOL CSettingContactUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_ACCOUNTS_COMING == message)
	{
		OnAccountsComing(int(wParam));
		return true;
	}

	if (message == WM_SUB_FOLDER_COMING)
	{
		list<TCHAR*> *pList = (list<TCHAR*> *)wParam;
		OnSubFolderListComing(pList);
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}


