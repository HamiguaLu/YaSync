// SettingCalendarUI.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "SettingCalendarUI.h"
#include "PECore.h"
#include "SettingHelper.h"
#include "AppTools.h"
#include "UrlEscape.h" 

// CSettingCalendarUI dialog

IMPLEMENT_DYNAMIC(CSettingCalendarUI, CDialog)
extern HWND g_hSettingUI;

CSettingCalendarUI::CSettingCalendarUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingCalendarUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingCalendarUI::~CSettingCalendarUI()
{
}

void CSettingCalendarUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX); 
	DDX_Control(pDX, IDC_CAL_LIST, m_CalList);
	DDX_Control(pDX, IDC_CAL_FOLDER_NEED_SYNC, m_CalSyncFolder);
	DDX_Control(pDX, IDC_CAL_SYNC_TYPE, m_cbSyncType);
	DDX_Control(pDX, IDC_CALENDAR_DATE_PICKER, m_calendarDate);
	DDX_Control(pDX, IDC_CALENDAR_TIME_PICKER, m_calendarTime);
}


BEGIN_MESSAGE_MAP(CSettingCalendarUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_ENABLE_CAL_SYNC, &CSettingCalendarUI::OnBnClickedEnableCalSync)
	ON_BN_CLICKED(IDC_CHECK_SYNC_AFTER_TIME, &CSettingCalendarUI::OnBnClickedCheckSyncAfterTime)
	ON_NOTIFY(NM_CLICK, IDC_CAL_LIST, &CSettingCalendarUI::OnNMClickCalList)
	ON_NOTIFY(NM_CLICK, IDC_CAL_FOLDER_NEED_SYNC, &CSettingCalendarUI::OnNMClickCalFolderNeedSync)
	ON_CBN_SELCHANGE(IDC_CAL_SYNC_TYPE, &CSettingCalendarUI::OnCbnSelchangeCalSyncType)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_CALENDAR_DATE_PICKER, &CSettingCalendarUI::OnDtnDatetimechangeCalendarDatePicker)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_CALENDAR_TIME_PICKER, &CSettingCalendarUI::OnDtnDatetimechangeCalendarTimePicker)
	ON_BN_CLICKED(IDC_CHECK_ONLY_SYNC_SELECTED_CALS, &CSettingCalendarUI::OnBnClickedCheckOnlySyncSelectedCals)
END_MESSAGE_MAP()


// CSettingCalendarUI message handlers

BOOL CSettingCalendarUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	//OnBnClickedEnableCalSync();
	//OnBnClickedCheckSyncAfterTime();

	m_CalList.SetHeadings(_T("Name, 290; id,0;"));
	m_CalList.SetGridLines(TRUE);
	m_CalList.SetCheckboxes(TRUE);
	m_CalList.SetFullRowSelect(TRUE);

	m_CalSyncFolder.SetHeadings(_T("Folder Name, 290; id,0;"));
	m_CalSyncFolder.SetGridLines(TRUE);
	m_CalSyncFolder.SetCheckboxes(TRUE);
	m_CalSyncFolder.SetFullRowSelect(TRUE);

	m_cbSyncType.ResetContent();
	m_cbSyncType.AddString(_T("Sync both directions"));
	m_cbSyncType.AddString(_T("Sync from phone to outlook"));
	m_cbSyncType.AddString(_T("Sync from outlook to phone"));

	

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CSettingCalendarUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CSettingCalendarUI::OnSave()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CAL_SYNC);
	s->dwSyncCalendar = pBtn->GetCheck();
	if (s->dwSDKVer < 14)
	{
		s->dwSyncCalendar = 0;
	}

	if (s->dwSyncCalendar)
	{
		int iSel = m_cbSyncType.GetCurSel();
		if (iSel < 0)
		{
			MessageBox(_T("Please selet the calendar sync type first"),_T("Error"),MB_ICONSTOP|MB_OK);
			return;
		}

		s->dwCalSyncType = iSel;
	}

	pBtn = (CButton*)GetDlgItem(IDC_CHECK_ONLY_SYNC_SELECTED_CALS);
	s->dwOnlySyncSelectedCal = pBtn->GetCheck();

	pBtn = (CButton*)GetDlgItem(IDC_CHECK_SYNC_AFTER_TIME);
	s->dwOnlySyncCalAfterTime = pBtn->GetCheck();
	if (s->dwOnlySyncCalAfterTime)
	{
		SYSTEMTIME stDate,stDayTime;
		m_calendarDate.GetTime(&stDate);
		m_calendarTime.GetTime(&stDayTime);

		stDate.wHour = stDayTime.wHour;
		stDate.wMinute = stDayTime.wMinute;
		stDate.wSecond = stDayTime.wSecond;

		SystemTimeToFileTime(&stDate,&s->ftCalendarSyncAfter);
	}

	int iCount = m_CalList.GetItemCount();
	CString sCalIDList = _T("");
	int iFirstID = 1;
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		BOOL bCheck = m_CalList.IsItemChecked(iItem);

		if (!bCheck)
		{
			continue;
		}

		CString sCalID = m_CalList.GetItemText(iItem, 1);
		sCalIDList += sCalID;
		sCalIDList += _T(";");
	}

	_tcscpy(s->szCalendarIDs,sCalIDList.GetBuffer());
	sCalIDList.ReleaseBuffer();

	iCount = m_CalSyncFolder.GetItemCount();
	CString sCalFolderList = _T("");
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		BOOL bCheck = m_CalSyncFolder.IsItemChecked(iItem);

		if (!bCheck)
		{
			continue;
		}

		CString sCalFolder = m_CalSyncFolder.GetItemText(iItem, 0);
		if (sCalFolder != CALENDAR_ROOT_FOLDER_NAME)
		{
			TCHAR *szFolder = URLEncodeW(sCalFolder.GetBuffer(),sCalFolder.GetLength());
			sCalFolderList += szFolder;
			sCalFolderList += _T(";");
			sCalFolder.ReleaseBuffer();

			free(szFolder);
		}
	}

	_tcscpy(s->szCalendarFolderNames,sCalFolderList.GetBuffer());
	sCalFolderList.ReleaseBuffer();
}


void CSettingCalendarUI::OnDeviceConnected()
{
	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CAL_SYNC);
	pBtn->SetCheck(s->dwSyncCalendar);
	if (s->dwSDKVer < 14)
	{
		pBtn->EnableWindow(0);
		
		s->dwSyncCalendar = 0;
		return;
	}

	if (s->dwSyncCalendar)
	{
		m_cbSyncType.SetCurSel(s->dwCalSyncType);
		m_cbSyncType.ShowWindow(SW_SHOW);
	}
	else
	{
		m_cbSyncType.ShowWindow(SW_HIDE);
	}

	pBtn = (CButton*)GetDlgItem(IDC_CHECK_ONLY_SYNC_SELECTED_CALS);
	pBtn->SetCheck(s->dwOnlySyncSelectedCal);
	
	pBtn = (CButton*)GetDlgItem(IDC_CHECK_SYNC_AFTER_TIME);
	pBtn->SetCheck(s->dwOnlySyncCalAfterTime);
	if (s->dwOnlySyncCalAfterTime)
	{
		if (s->ftCalendarSyncAfter.dwHighDateTime > 0)
		{
			SYSTEMTIME stTime;
			FileTimeToSystemTime(&s->ftCalendarSyncAfter,&stTime);
			m_calendarDate.SetTime(&stTime);
			m_calendarTime.SetTime(&stTime);
		}

		m_calendarDate.ShowWindow(SW_SHOW);
		m_calendarTime.ShowWindow(SW_SHOW);
	}
	else
	{
		m_calendarDate.ShowWindow(SW_HIDE);
		m_calendarTime.ShowWindow(SW_HIDE);
	}

	PECore::PostMessage(WM_GET_CAL_LIST,(WPARAM)m_hWnd,0);
	PECore::PostMessage(WM_GET_SUB_FOLDER,FOLDER_TYPE_CALENDAR,(WPARAM)m_hWnd);
}

void CSettingCalendarUI::OnBnClickedEnableCalSync()
{
	CButton* pBtn = (CButton*)GetDlgItem(IDC_ENABLE_CAL_SYNC);
	if (pBtn->GetCheck())
	{
		m_cbSyncType.ShowWindow(SW_SHOW);
	}
	else
	{
		m_cbSyncType.ShowWindow(SW_HIDE);
	}

	m_cbSyncType.SelectString(0,_T("Sync both directions"));

	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,1,0);
}


extern list<CAL_ITEM*> 		g_calendarList;
void CSettingCalendarUI::OnCalendarListComing(int iState)
{
	if (iState == 0)
	{
		return;
	}

	m_CalList.DeleteAllItems();

	list<CAL_ITEM*>::iterator it;
	for (it = g_calendarList.begin(); it != g_calendarList.end(); ++it)
	{ 
		CAL_ITEM *pCalendar = (CAL_ITEM*)*it;
		if (pCalendar->ulCalendarID < 1 )
		{
			PEWriteLog(_T("ulContactID < 0"));
			free(pCalendar);
			continue;
		}

		CString sID;
		sID.Format(_T("%d"),pCalendar->ulCalendarID);
		m_CalList.InsertItem(0,pCalendar->szDisplayName,sID);

		free(pCalendar);
	}

	g_calendarList.clear();

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	list<TCHAR*> selCalIDList = splitString(s->szCalendarIDs,_T(';'));
	list<TCHAR*>::iterator it1;
	for (it1 = selCalIDList.begin(); it1 != selCalIDList.end(); ++it1)
	{ 
		TCHAR *szID = (TCHAR*)*it1;

		int iCount = m_CalList.GetItemCount();

		for (int iItem = 0; iItem < iCount; ++iItem)
		{
			CString sCalFolder = m_CalList.GetItemText(iItem, 1);
			if (_tcscmp(sCalFolder.GetBuffer(),szID) == 0)
			{
				m_CalList.CheckItem(iItem);
			}

			sCalFolder.ReleaseBuffer();
		}
	}

	FreeStringList(selCalIDList);
}

void CSettingCalendarUI::OnSubFolderListComing(list<TCHAR*> *pList)
{
	m_CalSyncFolder.DeleteAllItems();

	list<TCHAR*>::iterator it;
	for (it = pList->begin(); it != pList->end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;
		m_CalSyncFolder.InsertItem(0,szName,_T("0"));
	}

	PE_PHONE_SETTING* s = PECore::GetCurPhone();
	if (s == NULL)
	{
		return;
	}

	list<TCHAR*> selFolderList = splitString(s->szCalendarFolderNames,_T(';'));
	for (it = selFolderList.begin(); it != selFolderList.end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;

		TCHAR szRealName[1024];
		URLDecoder(szName,szRealName,1024);

		int iCount = m_CalSyncFolder.GetItemCount();

		for (int iItem = 0; iItem < iCount; ++iItem)
		{
			CString sCalFolder = m_CalSyncFolder.GetItemText(iItem, 0);
			if (_tcscmp(sCalFolder.GetBuffer(),szRealName) == 0)
			{
				m_CalSyncFolder.CheckItem(iItem);
			}

			sCalFolder.ReleaseBuffer();
		}
	}

	FreeStringList(selFolderList);
}


void CSettingCalendarUI::OnBnClickedCheckSyncAfterTime()
{
	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK_SYNC_AFTER_TIME);
	if (pBtn->GetCheck())
	{
		m_calendarDate.ShowWindow(SW_SHOW);
		m_calendarTime.ShowWindow(SW_SHOW);
	}
	else
	{
		m_calendarDate.ShowWindow(SW_HIDE);
		m_calendarTime.ShowWindow(SW_HIDE);
	}

	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,2,0);
}

void CSettingCalendarUI::OnNMClickCalList(NMHDR *pNMHDR, LRESULT *pResult)
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,3,0);
	*pResult = 0;
}

void CSettingCalendarUI::OnNMClickCalFolderNeedSync(NMHDR *pNMHDR, LRESULT *pResult)
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,4,0);
	*pResult = 0;
}

void CSettingCalendarUI::OnCbnSelchangeCalSyncType()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,5,0);
}


BOOL CSettingCalendarUI::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (message == WM_CAL_LIST_COMING)
	{
		OnCalendarListComing((int)wParam);
		return TRUE;
	}

	if (message == WM_SUB_FOLDER_COMING)
	{
		list<TCHAR*> *pList = (list<TCHAR*> *)wParam;
		OnSubFolderListComing(pList);
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

void CSettingCalendarUI::OnDtnDatetimechangeCalendarDatePicker(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,6,0);
	*pResult = 0;
}

void CSettingCalendarUI::OnDtnDatetimechangeCalendarTimePicker(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,7,0);
	*pResult = 0;
}

void CSettingCalendarUI::OnBnClickedCheckOnlySyncSelectedCals()
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,8,0);
}
