// InstallDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InstallDlg.h"
#include "PECore.h"

// CInstallDlg dialog

IMPLEMENT_DYNAMIC(CInstallDlg, CDialog)

CInstallDlg::CInstallDlg(CWnd* pParent /*=NULL*/)
: CDialog(CInstallDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CInstallDlg::~CInstallDlg()
{
}

void CInstallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CInstallDlg, CDialog)
	ON_BN_CLICKED(ID_START_INSTALL, &CInstallDlg::OnBnClickedStartInstall)
	ON_BN_CLICKED(IDCANCEL, &CInstallDlg::OnBnClickedCancel)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CInstallDlg message handlers

void CInstallDlg::OnBnClickedStartInstall()
{
	PECore::PostMessage(WM_INSTALL_CLIENT,(WPARAM)m_hWnd,0);
	m_btnStartInstall.EnableWindow(0);

	CString sInfo = _T("Please wait...");
	SetDlgItemTextW(IDC_STATIC_INSTALL_INFO,sInfo);
	
}

void CInstallDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

HBRUSH CInstallDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CInstallDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CInstallDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}


BOOL CInstallDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnStartInstall.SubclassDlgItem(ID_START_INSTALL, this);

	m_btnStartInstall.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnStartInstall.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnStartInstall.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnStartInstall.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnStartInstall.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnStartInstall.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_btnCancel.SubclassDlgItem(IDCANCEL, this);

	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN2_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN2_SEL_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN2_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN2_FONT_FOCUS_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN2_FONT_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN2_FONT_COLOR);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CInstallDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if ( WM_INSTALL_RESULT == message)
	{
		int iRet = (int)wParam;
		if (iRet == PE_RET_OK)
		{
			SetDlgItemTextW(IDC_STATIC_INSTALL_INFO,_T("Successfully installed YaSync client\r\n on your device"));
		}
		else
		{
			SetDlgItemTextW(IDC_STATIC_INSTALL_INFO,_T("Failed to install YaSync client on your device,\r\n \
				you can install it from Google play"));
		}

		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}
