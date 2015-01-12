// TrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TrayDlg.h"


// CTrayDlg dialog

IMPLEMENT_DYNAMIC(CTrayDlg, CDialog)

CTrayDlg::CTrayDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTrayDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(TRAY_DLG_BK_COLOR);
	memset(m_szReplyAddr,0,sizeof(m_szReplyAddr));
}

CTrayDlg::~CTrayDlg()
{
}

void CTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTrayDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CTrayDlg::OnBnClickedOk)
	ON_WM_TIMER()
	ON_BN_CLICKED(ID_NEXT_MSG_BTN, &CTrayDlg::OnBnClickedNextMsgBtn)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_CLOSE_BTN, &CTrayDlg::OnBnClickedCloseBtn)
	ON_BN_CLICKED(ID_REPLY_BTN, &CTrayDlg::OnBnClickedReplyBtn)
	ON_WM_LBUTTONDOWN()
	ON_WM_ACTIVATE()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CTrayDlg message handlers

void CTrayDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

BOOL CTrayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnClose.SubclassDlgItem(IDC_CLOSE_BTN, this);
	m_btnClose.SetIcon(IDI_CLOSE_BTN);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_IN,TRAY_BAR_BK_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_FG_IN,TRAY_BAR_BK_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_OUT,TRAY_BAR_BK_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_FG_OUT,TRAY_BAR_BK_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,TRAY_BAR_BK_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,TRAY_BAR_BK_COLOR);
	m_btnClose.SetColor(CButtonST::BTNST_COLOR_BK_IN,TRAY_BAR_BK_COLOR);

	m_btnReply.SubclassDlgItem(ID_REPLY_BTN, this);
	m_btnReply.SetColor(CButtonST::BTNST_COLOR_BK_OUT,TRAY_DLG_BK_COLOR);
	m_btnReply.SetColor(CButtonST::BTNST_COLOR_BK_IN,TRAY_DLG_BK_COLOR);
	m_btnReply.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,TRAY_DLG_BK_COLOR);
	m_btnReply.SetColor(CButtonST::BTNST_COLOR_FG_IN,TRAY_DLG_FG_COLOR);
	m_btnReply.SetColor(CButtonST::BTNST_COLOR_FG_OUT,TRAY_DLG_FG_COLOR);
	m_btnReply.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,TRAY_DLG_FG_COLOR);
	m_btnReply.SetIcon(IDI_ICON_REPLY);

	m_btnNextMsg.SubclassDlgItem(ID_NEXT_MSG_BTN, this);
	m_btnNextMsg.SetColor(CButtonST::BTNST_COLOR_BK_OUT,TRAY_DLG_BK_COLOR);
	m_btnNextMsg.SetColor(CButtonST::BTNST_COLOR_BK_IN,TRAY_DLG_BK_COLOR);
	m_btnNextMsg.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,TRAY_DLG_BK_COLOR);
	m_btnNextMsg.SetColor(CButtonST::BTNST_COLOR_FG_IN,TRAY_DLG_FG_COLOR);
	m_btnNextMsg.SetColor(CButtonST::BTNST_COLOR_FG_OUT,TRAY_DLG_FG_COLOR);
	m_btnNextMsg.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,TRAY_DLG_FG_COLOR);

	m_MsgList.clear();

	Move2Bottome();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CTrayDlg::ShowOneMsg()
{
	list<TRAY_MSG*>::iterator it = m_MsgList.begin();
	TRAY_MSG* pMsg = (TRAY_MSG*)*it;

	SetDlgItemText(IDC_STATIC_MSG,pMsg->szMsg);
	if (pMsg->iType = TRAY_MSG_REPLY_SMS)
	{
		GetDlgItem(ID_REPLY_BTN)->ShowWindow(SW_SHOW);
		_tcscpy(m_szReplyAddr,pMsg->szReplyAddr);
	}
	else
	{
		GetDlgItem(ID_REPLY_BTN)->ShowWindow(SW_HIDE);
	}

	KillTimer(TRAY_TIMEER_EVT);
#ifndef _DEBUG
	SetTimer(TRAY_TIMEER_EVT,1000 * 30,0);
#else
	SetTimer(TRAY_TIMEER_EVT,1000 * 10,0);
#endif
}


void CTrayDlg::ShowMsg(TRAY_MSG *pMsg)
{
	if (!pMsg || _tcslen(pMsg->szMsg) < 1)
	{
		return;
	}

	m_MsgList.push_back(pMsg);

	if (m_MsgList.size() == 1)
	{
		ShowOneMsg();
	}

	ShowWindow(SW_SHOW);
	SetForegroundWindow();
	::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);

	int iSize = (int)m_MsgList.size();
	if (iSize > 1)
	{
		GetDlgItem(ID_NEXT_MSG_BTN)->ShowWindow(SW_SHOW);
		CString sBtnTitle;
		sBtnTitle.Format(_T("Next Message(%d)"),iSize - 1);
		SetDlgItemText(ID_NEXT_MSG_BTN,sBtnTitle);
	}
	else
	{
		GetDlgItem(ID_NEXT_MSG_BTN)->ShowWindow(SW_HIDE);
	}
}

void CTrayDlg::Free1stMsg()
{
	if (m_MsgList.size() < 1)
	{
		return;
	}

	list<TRAY_MSG*>::iterator it = m_MsgList.begin();
	TRAY_MSG* pMsg = (TRAY_MSG*)*it;

	free(pMsg->szMsg);
	free(pMsg->szReplyAddr);
	free(pMsg);

	m_MsgList.pop_front();
}

void CTrayDlg::FreeAllMsg()
{
	list<TRAY_MSG*>::iterator it;
	for (it = m_MsgList.begin(); it != m_MsgList.end(); ++it)
	{ 
		TRAY_MSG* pMsg = (TRAY_MSG*)*it;
		free(pMsg->szMsg);
		free(pMsg->szReplyAddr);
		free(pMsg);
	}

	m_MsgList.clear();
}


void CTrayDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (TRAY_TIMEER_EVT == nIDEvent)
	{
		ShowWindow(SW_HIDE);
		KillTimer(TRAY_TIMEER_EVT);
		FreeAllMsg();
	}

	CDialog::OnTimer(nIDEvent);
}

void CTrayDlg::OnBnClickedNextMsgBtn()
{
	KillTimer(TRAY_TIMEER_EVT);

	Free1stMsg();

	int iSize = (int)m_MsgList.size();
	if (iSize == 0)
	{
		GetDlgItem(ID_REPLY_BTN)->ShowWindow(SW_HIDE);
		GetDlgItem(ID_NEXT_MSG_BTN)->ShowWindow(SW_HIDE);
		ShowWindow(SW_HIDE);
		KillTimer(TRAY_TIMEER_EVT);
		return;
	}
	
	CString sBtnTitle;
	if (iSize == 1)
	{
		GetDlgItem(ID_NEXT_MSG_BTN)->ShowWindow(SW_HIDE);
	}
	else
	{
		sBtnTitle.Format(_T("Next Message(%d)"),iSize - 1);
		SetDlgItemText(ID_NEXT_MSG_BTN,sBtnTitle);
	}

	ShowOneMsg();

	return;
}


void CTrayDlg::Move2Bottome()
{
	CRect   rect;  
	CWnd *pWndPrev=   FindWindow(_T("Shell_TrayWnd"), _T(""));
	pWndPrev->GetWindowRect(&rect); 

	RECT rc;
	this->GetClientRect(&rc);

	RECT rt;
	 // 获得工作区大小
	SystemParametersInfo(SPI_GETWORKAREA,	0,	 &rt,	0);

	//上面   
	int w = rc.right - rc.left;
	int h = rc.bottom - rc.top;
	int x = rt.right - 20 - w;
	int y = rt.bottom - rect.Height() - h;

	MoveWindow(x,y,w,h);

}


BOOL CTrayDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CTrayDlg::OnNcHitTest(CPoint point)
{
	KillTimer(TRAY_TIMEER_EVT);

	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

HBRUSH CTrayDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor   ==   CTLCOLOR_STATIC)  
	{  
		pDC->SetTextColor(RGB(0,0,0));  
		pDC->SetBkColor(TRAY_DLG_BK_COLOR);  
		return m_hDialogBrush;
	}
	else if (nCtlColor == CTLCOLOR_DLG)
	{
		//pDC->SetTextColor(COLOR_DLG_VC);
		return m_hDialogBrush;
	}
	return m_hDialogBrush;
}

void CTrayDlg::OnBnClickedCloseBtn()
{
	KillTimer(TRAY_TIMEER_EVT);
	FreeAllMsg();
	ShowWindow(SW_HIDE);
}

#include "NewSmsDlg.h"
void CTrayDlg::OnBnClickedReplyBtn()
{
	KillTimer(TRAY_TIMEER_EVT);

	CNewSmsDlg dlg;
	_tcscpy(dlg.m_szReceiver,m_szReplyAddr);
	dlg.DoModal();

	ShowWindow(SW_SHOW);
	::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0, SWP_NOSIZE | SWP_NOMOVE);
}



void CTrayDlg::OnClose()
{
	FreeAllMsg();

	CDialog::OnClose();
}
