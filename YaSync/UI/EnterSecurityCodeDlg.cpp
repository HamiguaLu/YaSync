// EnterSecurityCodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EnterSecurityCodeDlg.h"
#include "SettingHelper.h"
#include "DeviceBlockHelper.h"
#include "PECore.h"

// CEnterSecurityCodeDlg dialog

IMPLEMENT_DYNAMIC(CEnterSecurityCodeDlg, CDialog)


list<TCHAR *> CEnterSecurityCodeDlg::m_filterList;
int CEnterSecurityCodeDlg::IsItemInFilterList(TCHAR *contact)
{
	list<TCHAR*>::iterator it;

	for (it = m_filterList.begin(); it != m_filterList.end();++it )
	{ 
		TCHAR *szFilter = (TCHAR*)*it;
		if (_tcsstr(szFilter,contact))
		{
			return PE_RET_OK;
		}
	}

	return PE_RET_FAIL;
}


int CEnterSecurityCodeDlg::FreeFilterList()
{
	list<TCHAR*>::iterator it;

	for (it = m_filterList.begin(); it != m_filterList.end();++it )
	{ 
		TCHAR *szFilter = (TCHAR*)*it;
		free(szFilter);
	}

	m_filterList.clear();

	return PE_RET_FAIL;
}



CEnterSecurityCodeDlg::CEnterSecurityCodeDlg(CWnd* pParent /*=NULL*/)
: CDialog(CEnterSecurityCodeDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);

}

CEnterSecurityCodeDlg::~CEnterSecurityCodeDlg()
{
}

void CEnterSecurityCodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEnterSecurityCodeDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDOK, &CEnterSecurityCodeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CEnterSecurityCodeDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_WHAT_SECURITY_CODE, &CEnterSecurityCodeDlg::OnBnClickedBtnWhatSecurityCode)
	ON_BN_CLICKED(IDC_ADD_BLOCK_DEVICE, &CEnterSecurityCodeDlg::OnBnClickedAddBlockDevice)
END_MESSAGE_MAP()


// CEnterSecurityCodeDlg message handlers

HBRUSH CEnterSecurityCodeDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CEnterSecurityCodeDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CEnterSecurityCodeDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}


BOOL CEnterSecurityCodeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnOK.SubclassDlgItem(IDOK, this);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnOK.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_btnCancel.SubclassDlgItem(IDCANCEL, this);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN2_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN2_SEL_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN2_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN2_FONT_FOCUS_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN2_FONT_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN2_FONT_COLOR);

	m_btnWhatCode.SubclassDlgItem(IDC_BTN_WHAT_SECURITY_CODE, this);
	m_btnWhatCode.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN3_COLOR);
	m_btnWhatCode.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN3_SEL_COLOR);
	m_btnWhatCode.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN3_COLOR);
	m_btnWhatCode.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN3_FONT_FOCUS_COLOR);
	m_btnWhatCode.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN3_FONT_COLOR);
	m_btnWhatCode.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN3_FONT_COLOR);
	

	CString sInfo;
	sInfo.Format(_T("Please enter security code for device:\r\n%s"),m_sPhoneName);

	SetDlgItemTextW(IDC_STATIC_ENTER_S_CODE,sInfo);

	GetDlgItem(IDC_EDIT_SECURITY_CODE)->SetFocus();

	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CEnterSecurityCodeDlg::OnBnClickedOk()
{
	PE_PHONE_SETTING *pSetting2 = (PE_PHONE_SETTING*)calloc(1,sizeof(PE_PHONE_SETTING));

	_tcscpy(pSetting2->szPhoneID,m_sPhoneID.GetBuffer());
	m_sPhoneID.ReleaseBuffer();

	CSettingHelper::GetPhoneSetting(pSetting2);

	CString sCode;
	GetDlgItemText(IDC_EDIT_SECURITY_CODE,sCode);

	_tcscpy(pSetting2->szSecurityCode,sCode.GetBuffer());
	sCode.ReleaseBuffer();

	CSettingHelper::SetPhoneSetting(pSetting2);

	PEWriteLog(_T("Test conn from CEnterSecurityCodeDlg"));

	TCHAR *szIP = (TCHAR *)calloc(1,255);
	_tcscpy(szIP,pSetting2->szDevIP);
	free(pSetting2);

	PECore::PostMessage(WM_CONNECT_2_DEVICE,(WPARAM)szIP,0);

	OnOK();
}

void CEnterSecurityCodeDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CEnterSecurityCodeDlg::OnBnClickedBtnWhatSecurityCode()
{
	ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/YaSyncGuide/What-IS-SecurityCode.htm"), NULL, NULL, SW_SHOWNORMAL);
}

void CEnterSecurityCodeDlg::OnBnClickedAddBlockDevice()
{
	CButton* pBtn = (CButton*)GetDlgItem(IDC_ADD_BLOCK_DEVICE);
	if (!pBtn->GetCheck())
	{
		return;
	}
		
	CString sDevice;
	sDevice.Format(_T("%s<%s>"),m_sPhoneName,m_sPhoneID);

	TCHAR *szTmp = (TCHAR*)calloc(sDevice.GetLength() + 1,sizeof(TCHAR));
	_tcscpy(szTmp,sDevice.GetBuffer());
	sDevice.ReleaseBuffer();

	m_filterList.push_back(szTmp);
	
}
