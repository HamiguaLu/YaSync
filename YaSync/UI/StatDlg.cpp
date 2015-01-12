// StatDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StatDlg.h"
#include "AppTools.h"


// CStatDlg dialog

IMPLEMENT_DYNAMIC(CStatDlg, CDialog)

CStatDlg::CStatDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);

}

CStatDlg::~CStatDlg()
{
}

void CStatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStatDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(ID_BTN_STAT_OK, &CStatDlg::OnBnClickedBtnStatOk)
END_MESSAGE_MAP()


// CStatDlg message handlers

HBRUSH CStatDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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


void CStatDlg::SetStatInfo(SMS_STAT *pSmsStat,CALL_LOG_STAT *pCallStat)
{
	m_pSmsStat = pSmsStat;
	m_pCallStat = pCallStat;
}

BOOL CStatDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CStatDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}


BOOL CStatDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnOK.SubclassDlgItem(ID_BTN_STAT_OK, this);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	CString sTitle;
	sTitle.Format(_T("Stat of %s<%s>"),m_pSmsStat->szName,m_pSmsStat->szPhone);
	SetDlgItemText(IDC_STAT_TITLE,sTitle);

	CString sSMSinfo;
	sSMSinfo.Format(_T("Sent out %d ,received %d"),m_pSmsStat->ulOutCount,m_pSmsStat->ulInCount);
	SetDlgItemText(IDC_STAT_SMS_INFO,sSMSinfo);

	CString sCallInfo;
	sCallInfo.Format(_T("%d outgoing calls (%d minutes)\r\n%d incoming calls (%d minutes)\r\n%d missed calls"),
					m_pCallStat->ulOutCount,m_pCallStat->ulOutTime,
					m_pCallStat->ulInCount,m_pCallStat->ulInTime,
					m_pCallStat->ulMissedCount);
	SetDlgItemText(IDC_STAT_CALL_STAT,sCallInfo);

	SetWinOnOutlook(m_hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CStatDlg::OnBnClickedBtnStatOk()
{
	SendMessage(WM_CLOSE);
}
