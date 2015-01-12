// RestoreDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RestoreDlg.h"
#include "PECore.h"
#include "AppTools.h"

extern int g_iCanSync;

// CRestoreDlg dialog

IMPLEMENT_DYNAMIC(CRestoreDlg, CDialog)

HWND g_hRestoreDlg;

CRestoreDlg::CRestoreDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRestoreDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CRestoreDlg::~CRestoreDlg()
{
}

void CRestoreDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_RESTORE, m_RestoreProgress);
}


BEGIN_MESSAGE_MAP(CRestoreDlg, CDialog)
	ON_BN_CLICKED(IDC_BTN_RESTORE, &CRestoreDlg::OnBnClickedBtnRestore)
	ON_BN_CLICKED(IDC_BTN_CANCEL_RESTORE, &CRestoreDlg::OnBnClickedBtnCancelRestore)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CRestoreDlg message handlers

void CRestoreDlg::OnBnClickedBtnCancelRestore()
{
	g_iCanSync = 1;
	SendMessage(WM_CLOSE);
}

BOOL CRestoreDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnStartRestore.SubclassDlgItem(IDC_BTN_RESTORE, this);
	m_btnStartRestore.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnStartRestore.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnStartRestore.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnStartRestore.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnStartRestore.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnStartRestore.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_btnCancelRestore.SubclassDlgItem(IDC_BTN_CANCEL_RESTORE, this);
	m_btnCancelRestore.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN2_COLOR);
	m_btnCancelRestore.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN2_SEL_COLOR);
	m_btnCancelRestore.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN2_COLOR);
	m_btnCancelRestore.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN2_FONT_FOCUS_COLOR);
	m_btnCancelRestore.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN2_FONT_COLOR);
	m_btnCancelRestore.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN2_FONT_COLOR);

	SetDlgItemText(IDC_STATIC_RESTORE,_T(""));

	m_iRestoreState = 0;

	CButton* pBtn = (CButton*)GetDlgItem(IDC_RADIO_SMS_RESOTRE);
	pBtn->SetCheck(1);

	g_iCanSync = 0;

	SetWinOnOutlook(m_hWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



HBRUSH CRestoreDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CRestoreDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CRestoreDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

void CRestoreDlg::OnBnClickedBtnRestore()
{
	g_hRestoreDlg = m_hWnd;
	
	int iExportType = PE_ACT_SAVE_SMS_2_PHONE;
	CButton* pBtn = (CButton*)GetDlgItem(IDC_RADIO_SMS_RESOTRE);
	if (pBtn->GetCheck())
	{
		iExportType = PE_ACT_SAVE_SMS_2_PHONE;
	}

	CButton* pBtn1 = (CButton*)GetDlgItem(IDC_RADIO_CALL_LOG_RESTORE);
	if (pBtn1->GetCheck())
	{
		iExportType = PE_ACT_SAVE_CALL_LOG_2_PHONE;
	}

	if (m_iRestoreState == 0)
	{
		CString sAsk;
		if (iExportType == PE_ACT_SAVE_SMS_2_PHONE)
		{
			sAsk = _T("This will restore SMS from outlook to your Phone,please make sure the phone you connected  is correct,other wise there will be duplicat items in your phone");
		}
		else if (iExportType == PE_ACT_SAVE_CALL_LOG_2_PHONE)
		{
			sAsk = _T("This will restore Call Log from outlook to your Phone,please make sure the phone you connected  is correct,other wise there will be duplicat items in your phone");
		}
		else
		{
			MessageBox(_T("Please select data you want to restore to your phone"),_T("Error"),MB_ICONSTOP|MB_OK);
			return;
		}

		if (IDCANCEL == MessageBox(sAsk,_T("Make sure you connected the right phone"),MB_ICONEXCLAMATION |MB_OKCANCEL))
		{
			return;
		}

		MSG_Data_Truck *p = (MSG_Data_Truck*)calloc(1,sizeof(MSG_Data_Truck));
		p->p1 = (WPARAM)m_hWnd;
		p->p2 = (WPARAM)iExportType;
		p->p3 = (WPARAM)m_szFolderID;
		

		PECore::SendMessage(WM_RESTORE_ITEM,(WPARAM)p);
		free(p);
	}
	else
	{

	}
}


void CRestoreDlg::OnRestoreState(PE_RESTORE_STATUS *s)
{
	if (s->fTotalCount <= 0)
	{
		return;
	}

	float fPercent = (s->fCurCount/s->fTotalCount) * 100;

	m_RestoreProgress.SetPos((int)fPercent);

	CString sPercent;
	sPercent.Format(_T(", %0.2f %% complete"),fPercent);

	CString sInfo = _T("Unkonw Restore Type");
	switch(s->iRestoreType)
	{
	case PE_ACT_SAVE_SMS_2_PHONE:
		{
			sInfo = _T("Now Restore SMS");
			break;
		}
	case PE_ACT_SAVE_CALL_LOG_2_PHONE:
		{
			sInfo = _T("Now Restore Call Log");
			break;
		}
	}

	sInfo += sPercent;

	SetDlgItemTextW(IDC_STATIC_RESTORE,sInfo);

	return;
}




BOOL CRestoreDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: Add your specialized code here and/or call the base class
	if (WM_RESTORE_INFO == message)
	{
		PE_RESTORE_STATUS *pInfo = (PE_RESTORE_STATUS *)wParam;
		if (!pInfo)
		{
			return TRUE;
		}

		OnRestoreState(pInfo);

		free(pInfo);

		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}
