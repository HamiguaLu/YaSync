// SaveContactsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SaveContactsDlg.h"
#include "PECore.h"
#include "AppTools.h"
// CSaveContactsDlg dialog

IMPLEMENT_DYNAMIC(CSaveContactsDlg, CDialog)

CSaveContactsDlg::CSaveContactsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSaveContactsDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);

}

CSaveContactsDlg::~CSaveContactsDlg()
{
}

void CSaveContactsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSaveContactsDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_SAVE_CONTACTS, &CSaveContactsDlg::OnBnClickedBtnSaveContacts)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CSaveContactsDlg::OnBnClickedBtnCancel)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CSaveContactsDlg message handlers

BOOL CSaveContactsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnSave.SubclassDlgItem(IDC_BTN_SAVE_CONTACTS, this);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnSave.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_btnCancel.SubclassDlgItem(IDC_BTN_CANCEL, this);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	SetDlgItemText(IDC_CONTACT_NAME,m_szName);
	SetDlgItemText(IDC_CONTACT_PHONE,m_szPhone);

	SetWinOnOutlook(m_hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CSaveContactsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CSaveContactsDlg::SetContacts(TCHAR *szName,TCHAR* szPhone)
{
	m_szName = szName;
	m_szPhone = szPhone;
}

void CSaveContactsDlg::OnBnClickedBtnSaveContacts()
{
	CString sName,sPhone;
	GetDlgItemText(IDC_CONTACT_NAME,sName);
	GetDlgItemText(IDC_CONTACT_PHONE,sPhone);

	TCHAR *szName = (TCHAR*)calloc(255,sizeof(TCHAR));
	TCHAR *szPhone = (TCHAR*)calloc(255,sizeof(TCHAR));
	_tcscpy(szName,sName.GetBuffer());
	_tcscpy(szPhone,sPhone.GetBuffer());

	sName.ReleaseBuffer();
	sPhone.ReleaseBuffer();
	
	PECore::PostMessage(WM_SAVE_CONTACT,(WPARAM)szName,(LPARAM)szPhone);

	OnClose();
}

void CSaveContactsDlg::OnBnClickedBtnCancel()
{
	OnCancel();
}



BOOL CSaveContactsDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CSaveContactsDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

