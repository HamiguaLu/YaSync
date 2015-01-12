// SettingDeviceUI.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "SettingDeviceUI.h"
#include "DeviceBlockHelper.h"


// CSettingDeviceUI dialog
extern HWND g_hSettingUI;


IMPLEMENT_DYNAMIC(CSettingDeviceUI, CDialog)

CSettingDeviceUI::CSettingDeviceUI(CWnd* pParent /*=NULL*/)
	: CDialog(CSettingDeviceUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingDeviceUI::~CSettingDeviceUI()
{
}

void CSettingDeviceUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DEVICE_LIST, m_FilterList);
}


BEGIN_MESSAGE_MAP(CSettingDeviceUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_NOTIFY(NM_CLICK, IDC_DEVICE_LIST, &CSettingDeviceUI::OnNMClickDeviceList)
END_MESSAGE_MAP()


// CSettingDeviceUI message handlers

HBRUSH CSettingDeviceUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSettingDeviceUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_FilterList.SetHeadings(_T("Device, 280;"));
	m_FilterList.SetGridLines(TRUE);
	m_FilterList.SetCheckboxes(TRUE);
	m_FilterList.SetFullRowSelect(TRUE);

	DeviceBlockHelper::Init();
	list<TCHAR *> filterList = DeviceBlockHelper::GetFilterList();

	list<TCHAR*>::iterator it;
	for (it = filterList.begin(); it != filterList.end(); ++it)
	{ 
		TCHAR *szFilter = (TCHAR*)*it;
		m_FilterList.InsertItem(0,szFilter);
	}

	m_FilterList.CheckAllItems();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingDeviceUI::OnSave()
{
	DeviceBlockHelper::FreeFilterList();

	int iCount = m_FilterList.GetItemCount();
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		if (!m_FilterList.IsItemChecked(iItem))
		{
			continue;
		}

		CString sFilter = m_FilterList.GetItemText(iItem, 0);
		DeviceBlockHelper::AddFilter(sFilter.GetBuffer());

		sFilter.ReleaseBuffer();
	}

	DeviceBlockHelper::SaveFilter();
}

void CSettingDeviceUI::OnNMClickDeviceList(NMHDR *pNMHDR, LRESULT *pResult)
{
	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,19,0);
}
