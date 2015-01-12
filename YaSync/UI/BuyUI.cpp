// BuyUI.cpp : implementation file
//

#include "stdafx.h"
#include "AppTools.h"

#include "PEProtocol.h"

#include "YaSync.h"
#include "BuyUI.h"

#include "AppTools.h"
#include "PEInterface.h"

// CBuyUI dialog

IMPLEMENT_DYNAMIC(CBuyUI, CDialog)

CBuyUI::CBuyUI(CWnd* pParent /*=NULL*/)
	: CDialog(CBuyUI::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CBuyUI::~CBuyUI()
{
}

void CBuyUI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CBuyUI, CDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_BUY_ONLINE, &CBuyUI::OnBnClickedBtnBuyOnline)
	ON_BN_CLICKED(IDC_BTN_ACTIVATE, &CBuyUI::OnBnClickedBtnActivate)
END_MESSAGE_MAP()


// CBuyUI message handlers

BOOL CBuyUI::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_btnBuy.SubclassDlgItem(IDC_BTN_BUY_ONLINE, this);
	m_btnActivate.SubclassDlgItem(IDC_BTN_ACTIVATE, this);
	
	m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN2_SEL_COLOR );
	m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN2_COLOR);
	m_btnBuy.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN2_SEL_COLOR);
	m_btnBuy.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN2_FONT_COLOR);
	m_btnBuy.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN2_FONT_FOCUS_COLOR);
	m_btnBuy.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN2_FONT_COLOR);

	m_btnActivate.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnActivate.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnActivate.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnActivate.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnActivate.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnActivate.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	if (PEIsRegistered() == PE_RET_OK)
	{
		SetDlgItemText(IDC_STATIC_EXPIR_INFO,_T("This is a registered copy,thanks for your support"));
		return TRUE;
	}

	int iDays = GetTrailDays();
	if (iDays >= MAX_EXPIRE_DAY || iDays < 0)
	{
		SetDlgItemText(IDC_STATIC_EXPIR_INFO,_T("YaSync has expired,please regeister first"));
	}
	else
	{
		CString sInfo;
		iDays = MAX_EXPIRE_DAY - iDays;
		sInfo.Format(_T("YaSync will expire after %d days"),iDays);
		SetDlgItemText(IDC_STATIC_EXPIR_INFO,sInfo);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CBuyUI::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void CBuyUI::OnBnClickedBtnBuyOnline()
{
	ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/android-outlook-sync-buy.htm"), NULL, NULL, SW_SHOWMAXIMIZED);
}

void CBuyUI::OnBnClickedBtnActivate()
{
	CString sSN;
	GetDlgItemText(IDC_EDIT_SN,sSN);

	if (sSN.GetLength() < 1 || sSN.GetLength() > 100)
	{
		MessageBox(_T("Invalid Serial Number"),_T("Error"),MB_ICONSTOP);
		return;
	}

	char szUTFSN[255];

	WideCharToMultiByte (CP_OEMCP,NULL,(TCHAR*)(LPCTSTR)sSN,-1,(LPSTR)szUTFSN,255,NULL,FALSE);

	if (PE_RET_OK != VerifySerialNum(szUTFSN))
	{
		MessageBox(_T("Invalid Serial Number"),_T("Error"),MB_ICONSTOP);
		return;
	}

	MessageBox(_T("Thanks for register YaSync"),_T("Thanks"),MB_OK|MB_ICONINFORMATION);
	SetDlgItemText(IDC_STATIC_EXPIR_INFO,_T("This is a registered copy,thanks for your support"));

	SetDlgItemText(IDC_STATIC_EXPIR_INFO,_T("This is a registered copy,thanks for your support"));

}



