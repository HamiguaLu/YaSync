// HelpUI.cpp : implementation file
//

#include "stdafx.h"

#include "PEProtocol.h"

#include "YaSync.h"
#include "HelpUI.h"

#include "AppTools.h"


// CHelpUI dialog

IMPLEMENT_DYNAMIC(CHelpUI, CDialog)

CHelpUI::CHelpUI(CWnd* pParent /*=NULL*/)
: CDialog(CHelpUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CHelpUI::~CHelpUI()
{
}

void CHelpUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHelpUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_TROUBLE_SHOOTING, &CHelpUI::OnBnClickedBtnTroubleShooting)
	ON_BN_CLICKED(IDC_BTN_GET_LOG_FILE, &CHelpUI::OnBnClickedBtnGetLogFile)
END_MESSAGE_MAP()


// CHelpUI message handlers

HBRUSH CHelpUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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
	else if (CTLCOLOR_EDIT == nCtlColor)
	{
		pDC->SetBkColor(RGB(248,248,224));
		HBRUSH b=CreateSolidBrush(RGB(248,248,224));
		return b;
	}

	return m_hDialogBrush;
}

BOOL CHelpUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	m_btnTroublShooting.SubclassDlgItem(IDC_BTN_TROUBLE_SHOOTING, this);
	m_btnGetLog.SubclassDlgItem(IDC_BTN_GET_LOG_FILE, this);

	m_btnTroublShooting.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnTroublShooting.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnTroublShooting.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnTroublShooting.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnTroublShooting.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnTroublShooting.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_btnGetLog.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN2_SEL_COLOR );
	m_btnGetLog.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN2_COLOR);
	m_btnGetLog.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN2_SEL_COLOR);
	m_btnGetLog.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN2_FONT_COLOR);
	m_btnGetLog.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN2_FONT_FOCUS_COLOR);
	m_btnGetLog.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN2_FONT_COLOR);

	CString sRev;
	sRev.Format(_T("YaSync 2 Build %d %s %s"),APP_VER,_T(__DATE__),_T(__TIME__));
	SetDlgItemText(IDC_STATIC_REV,sRev);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CHelpUI::OnBnClickedBtnTroubleShooting()
{
	ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/sync-outlook-with-android-guide.htm"), NULL, NULL, SW_SHOWNORMAL);
}


extern void SendLogFile();
extern void SendSupportReq(TCHAR *szEmail,TCHAR *szProblem);
void CHelpUI::OnBnClickedBtnGetLogFile()
{
	TCHAR szLogPath[MAX_PATH];
	GetAppDataPath(szLogPath);

#ifndef _DEBUG
	CString sEmail,sProblem;
	GetDlgItemText(IDC_EDIT_EMAIL,sEmail);
	GetDlgItemText(IDC_EDIT_PROBLEM,sProblem);

	if (sEmail.GetLength() < 3 || sProblem.GetLength() < 1)
	{
		MessageBox(_T("Please enter a valid mail address and problem description,thanks"),_T(""),MB_ICONEXCLAMATION |MB_OK);
		return;
	}

	SendSupportReq(sEmail.GetBuffer(),sProblem.GetBuffer());
	sEmail.ReleaseBuffer();
	sProblem.ReleaseBuffer();

	SendLogFile();
#else
	ShellExecute(NULL,_T("open"),szLogPath,NULL,NULL,SW_SHOW);
#endif

	MessageBox(_T("Support request has been sent to MobiTNT.com, please wait 1 or 2 days for response,thanks"),_T(""),MB_ICONEXCLAMATION |MB_OK);
}


