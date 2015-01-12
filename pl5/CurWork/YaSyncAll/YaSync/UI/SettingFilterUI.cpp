// CSettingFilterUI.cpp : implementation file
//

#include "stdafx.h"
#include "SettingFilterUI.h"
#include "FilterHelper.h"
#include "PECore.h"

// CSettingFilterUI dialog

IMPLEMENT_DYNAMIC(CSettingFilterUI, CDialog)

extern HWND g_hSettingUI;


CSettingFilterUI::CSettingFilterUI(CWnd* pParent /*=NULL*/)
: CDialog(CSettingFilterUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSettingFilterUI::~CSettingFilterUI()
{
}

void CSettingFilterUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILTER, m_FilterList);
}


BEGIN_MESSAGE_MAP(CSettingFilterUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL, &CSettingFilterUI::OnBnClickedBtnSelectAll)
	ON_BN_CLICKED(IDC_BTN_APPLY_FILTER, &CSettingFilterUI::OnBnClickedBtnApplyFilter)
	ON_BN_CLICKED(IDC_BTN_ADD_FILTER, &CSettingFilterUI::OnBnClickedBtnAddFilter)
END_MESSAGE_MAP()


// CSettingFilterUI message handlers

HBRUSH CSettingFilterUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

	return hbr;
}

BOOL CSettingFilterUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnAdd.SubclassDlgItem(IDC_BTN_ADD_FILTER, this);
	m_btnSelect.SubclassDlgItem(IDC_BTN_SELECT_ALL, this);

	//m_btnAdd.SubclassDlgItem(IDC_BTN_SEL_ALL, this);
	m_btnAdd.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnAdd.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnAdd.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnAdd.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnAdd.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnAdd.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	//m_btnSelect.SubclassDlgItem(IDC_BTN_SEND_NOW, this);
	m_btnSelect.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnSelect.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnSelect.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnSelect.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnSelect.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnSelect.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_btnApply.SubclassDlgItem(IDC_BTN_APPLY_FILTER, this);
	m_btnApply.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnApply.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnApply.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnApply.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnApply.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnApply.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_FilterList.SetHeadings(_T("Filter, 280;"));
	m_FilterList.SetGridLines(TRUE);
	m_FilterList.SetCheckboxes(TRUE);
	m_FilterList.SetFullRowSelect(TRUE);

	PEWriteLog(_T("Init Filter from UI"));
	FilterHelper::Init();

	list<TCHAR *> filterList = FilterHelper::GetFilterList();

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

void CSettingFilterUI::OnBnClickedBtnSelectAll()
{
	static int iSel = 1;
	if (iSel)
	{
		m_FilterList.CheckAllItems();
		m_btnSelect.SetWindowText(_T("Unselect All"));
		iSel = 0;
	}
	else
	{
		m_FilterList.UnCheckAllItems();
		m_btnSelect.SetWindowText(_T("Select All"));
		iSel = 1;
	}

	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,21,0);
}

void CSettingFilterUI::OnBnClickedBtnApplyFilter()
{
	if (IDCANCEL == MessageBox(_T("Are you sure that you want to apply filter to the existing items? \r\n (This will cause some SMS/Call logs be removed from Outlook)"),_T("Are you sure?"),MB_ICONEXCLAMATION |MB_OKCANCEL))
	{
		return;
	}

	PECore::PostMessage(WM_APPLY_FILTER_NOW,0,0);
}

void CSettingFilterUI::OnSave()
{
	FilterHelper::FreeFilterList();

	int iCount = m_FilterList.GetItemCount();
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		if (!m_FilterList.IsItemChecked(iItem))
		{
			continue;
		}

		CString sFilter = m_FilterList.GetItemText(iItem, 0);
		FilterHelper::AddFilter(sFilter.GetBuffer());

		sFilter.ReleaseBuffer();
	}

	FilterHelper::SaveFilter();
	

}

void CSettingFilterUI::OnBnClickedBtnAddFilter()
{
	CString sFilter;
	GetDlgItemText(IDC_EDIT_FILTER,sFilter);

	if (sFilter.GetLength() < 1 || sFilter.GetLength() >= 254)
	{
		MessageBox(_T("Invalid Filter Name"),_T("Error"),MB_ICONSTOP);
		return;
	}

	m_FilterList.InsertItem(0,sFilter);
	m_FilterList.CheckItem(0);

	::PostMessage(g_hSettingUI,WM_ENABLE_SAVE,22,0);
}


void CSettingFilterUI::SetFilter(TCHAR *szName,TCHAR *szPhone)
{
	TCHAR *szFilter = (TCHAR*)calloc(1024,sizeof(TCHAR));
	if (szName)
	{
		_stprintf(szFilter,_T("%s <%s>"),szName,szPhone);	
	}
	else
	{
		_stprintf(szFilter,_T("<%s>"),szPhone);		
	}

	SetDlgItemText(IDC_EDIT_FILTER,szFilter);

	free(szFilter);
}




