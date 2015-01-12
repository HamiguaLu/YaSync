// NewSmsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewSmsDlg.h"
#include "ContactsHelper.h"

#include "PECore.h"
#include "UrlEscape.h"

#include "AppTools.h"

#include "PEInterface.h"


// CNewSmsDlg dialog

IMPLEMENT_DYNAMIC(CNewSmsDlg, CDialog)

HWND g_hNewSmsWnd = 0;


CNewSmsDlg::CNewSmsDlg(CWnd* pParent /*=NULL*/)
: CDialog(CNewSmsDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(DLG_BK_COLOR);
	memset(m_szReceiver, 0 ,sizeof(m_szReceiver));
}

CNewSmsDlg::~CNewSmsDlg()
{
}

void CNewSmsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TO_CONTACTS, m_Contacts);
	DDX_Control(pDX, IDC_YEAR_PICKER, m_yearPicker);
	DDX_Control(pDX, IDC_TIME_PICKER, m_timePicker);
}


BEGIN_MESSAGE_MAP(CNewSmsDlg, CDialog)
	//ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BTN_SEL_ALL, &CNewSmsDlg::OnBnClickedBtnSelAll)
	ON_BN_CLICKED(IDC_BTN_SEND_NOW, &CNewSmsDlg::OnBnClickedBtnSendNow)
	ON_BN_CLICKED(IDC_BTN_CANCEL, &CNewSmsDlg::OnBnClickedBtnCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_SMS_LIST_BTN, &CNewSmsDlg::OnBnClickedSmsListBtn)
	ON_BN_CLICKED(IDC_INSERT_CONTACT_BTN, &CNewSmsDlg::OnBnClickedInsertContactBtn)
	ON_BN_CLICKED(IDC_SEND_AT_TIME, &CNewSmsDlg::OnBnClickedSendAtTime)
	ON_EN_CHANGE(IDC_EDIT_SMS_CONTENT, &CNewSmsDlg::OnEnChangeEditSmsContent)
END_MESSAGE_MAP()


// CNewSmsDlg message handlers

HBRUSH CNewSmsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if(nCtlColor   ==   CTLCOLOR_STATIC)  
	{  
		pDC->SetTextColor(RGB(0,0,0));  
		pDC->SetBkColor(DLG_BK_COLOR);  
		return m_hDialogBrush;
	}
	else if (nCtlColor == CTLCOLOR_DLG)
	{
		//pDC->SetTextColor(COLOR_DLG_VC);
		return m_hDialogBrush;
	}
	return m_hDialogBrush;
}

BOOL CNewSmsDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CNewSmsDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}



BOOL CNewSmsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnSelAll.SubclassDlgItem(IDC_BTN_SEL_ALL, this);
	m_btnSelAll.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnSelAll.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnSelAll.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnSelAll.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnSelAll.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnSelAll.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_btnSendNow.SubclassDlgItem(IDC_BTN_SEND_NOW, this);
	m_btnSendNow.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN1_COLOR);
	m_btnSendNow.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN1_SEL_COLOR);
	m_btnSendNow.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN1_COLOR);
	m_btnSendNow.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN1_FONT_FOCUS_COLOR);
	m_btnSendNow.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN1_FONT_COLOR);
	m_btnSendNow.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN1_FONT_COLOR);

	m_btnInsert.SubclassDlgItem(IDC_INSERT_CONTACT_BTN, this);
	m_btnInsert.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN3_COLOR);
	m_btnInsert.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN3_SEL_COLOR);
	m_btnInsert.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN3_COLOR);
	m_btnInsert.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN3_FONT_FOCUS_COLOR);
	m_btnInsert.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN3_FONT_COLOR);
	m_btnInsert.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN3_FONT_COLOR);

	m_btnSmsList.SubclassDlgItem(IDC_SMS_LIST_BTN, this);
	m_btnSmsList.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN3_COLOR);
	m_btnSmsList.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN3_SEL_COLOR);
	m_btnSmsList.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN3_COLOR);
	m_btnSmsList.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN3_FONT_FOCUS_COLOR);
	m_btnSmsList.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN3_FONT_COLOR);
	m_btnSmsList.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN3_FONT_COLOR);

	m_btnClose.SubclassDlgItem(IDC_BTN_CANCEL, this);
	m_btnClose.DrawTransparent();
	m_btnClose.SetIcon(IDI_CLOSE_BTN);

	m_Contacts.SetHeadings(_T("Name,113; Phone,110;"));
	m_Contacts.SetGridLines(TRUE);
	m_Contacts.SetCheckboxes(TRUE);
	m_Contacts.SetFullRowSelect(TRUE);

	PECore::PostMessage(WM_GET_CONTACTS_FROM_OUTLOOK,(WPARAM)m_hWnd,0);

	m_pac = new CCustomAutoComplete();
	m_pac->Bind(GetDlgItem(IDC_EDIT_INSERT_CONTACT)->m_hWnd, ACO_UPDOWNKEYDROPSLIST | ACO_AUTOSUGGEST, NULL);

	g_hNewSmsWnd = m_hWnd;

	GetDlgItem(IDC_EDIT_INSERT_CONTACT)->SetFocus();

	SetDlgItemText(IDC_STATIC_TEXT_COUNT,_T(""));

	if (m_szReceiver)
	{
		SetDlgItemText(IDC_EDIT_TO_LIST,m_szReceiver);
	}
	
	SetWinOnOutlook(m_hWnd);
	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CNewSmsDlg::SetAutoComplete(list<CONTACT_ITEM*> *pList)
{


}


BOOL CNewSmsDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if ( WM_CONTACTS_COMING == message)
	{
		PEWriteLog(_T("WM_CONTACTS_COMING"));
		list<CONTACT_ITEM*> *pList = (list<CONTACT_ITEM*> *)wParam;

		list<CONTACT_ITEM*>::iterator it;
		m_Contacts.DeleteAllItems();
		for (it = pList->begin(); it != pList->end(); ++it)
		{ 
			CONTACT_ITEM *c = (CONTACT_ITEM*)*it;
			if (!c->pPhones || c->pPhones->size() < 1)
			{
				ContactsHelper::FreeContactItem(c);
				free(c);
				continue;
			}

			list<PhoneItem*>::iterator itc;
			for (itc = c->pPhones->begin(); itc != c->pPhones->end(); ++itc)
			{
				PhoneItem *p = (PhoneItem*)*itc;
				if (p->iPhoneType == PHONE_TYPE_MOBILE || p->iPhoneType == PHONE_TYPE_WORK_MOBILE)
				{
					m_Contacts.InsertItem(0,c->szDisplayName,p->szPhoneNo);
					CString sACItem;
					sACItem.Format(_T("%s <%s>"),c->szDisplayName,p->szPhoneNo);
					m_pac->AddItem(sACItem);
				}
			}

			ContactsHelper::FreeContactItem(c);
			free(c);
		}

		m_Contacts.Sort(0,TRUE);

		return TRUE;
	}

	if (WM_DISABLE_SEND_BTN == message)
	{
		m_btnSendNow.EnableWindow(1);
		return TRUE;
	}

	if (WM_SEND_SMS_STATUS == message)
	{
		if (wParam == PE_SEND_STATUS_CONN_DOWN)
		{
			MessageBox(_T("Device seems disconnected!"),_T("Error"),MB_ICONSTOP|MB_OK);
		}
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

void CNewSmsDlg::OnBnClickedBtnSelAll()
{
	static int iSel = 1;
	if (iSel)
	{
		m_Contacts.CheckAllItems();
		m_btnSelAll.SetWindowText(_T("Unselect All"));
		iSel = 0;
	}
	else
	{
		m_Contacts.UnCheckAllItems();
		m_btnSelAll.SetWindowText(_T("Select All"));
		iSel = 1;
	}

}


void CNewSmsDlg::OnBnClickedBtnSendNow()
{
	SetDlgItemText(IDC_EDIT_SMS_SEND_RESULT,_T(""));

	CString sContent;
	GetDlgItemText(IDC_EDIT_SMS_CONTENT,sContent);

	if (sContent.GetLength() < 1)
	{
		MessageBox(_T("SMS content is empty"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	if (PEIsRegistered() != PE_RET_OK)
	{
		sContent += _T("\r\n[Sent out By YaSync]");
	}

	int iCount = m_Contacts.GetItemCount();
	CString sSelPhones = _T("");
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		if (!m_Contacts.IsItemChecked(iItem))
		{
			continue;
		}

		sSelPhones += m_Contacts.GetItemText(iItem, 1);
		sSelPhones += _T(",");
	}

	CString sPhones;
	GetDlgItemText(IDC_EDIT_TO_LIST,sPhones);

	if (sPhones.GetLength() < 1 && sSelPhones.GetLength() < 1)
	{
		MessageBox(_T("Please select at least one receiver"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	sPhones += _T(",");
	sPhones += sSelPhones;

	TCHAR *szPhones = URLEncodeW(sPhones.GetBuffer(),sPhones.GetLength());
	sPhones.ReleaseBuffer();

	TCHAR *szContent = URLEncodeW(sContent.GetBuffer(),sContent.GetLength());
	sContent.ReleaseBuffer();

	CButton* pBtn = (CButton*)GetDlgItem(IDC_SEND_AT_TIME);
	if ( pBtn->GetCheck() )
	{
		SYSTEMTIME stYear,stDayTime,stNow;
		m_yearPicker.GetTime(&stYear);
		m_timePicker.GetTime(&stDayTime);

		stYear.wHour = stDayTime.wHour;
		stYear.wMinute = stDayTime.wMinute;
		stYear.wSecond = stDayTime.wSecond;

		FILETIME ftTime,ftNow;
		SystemTimeToFileTime(&stYear,&ftTime);

		GetLocalTime(&stNow);
		SystemTimeToFileTime(&stNow,&ftNow);

		if (PECompareFileTime(ftNow,ftTime) != PE_RET_CMP_LOWER)
		{
			MessageBox(_T("Please select a valid time"),_T("Error"),MB_ICONSTOP|MB_OK);
			return;
		}

		SCHEDULE_SMS_ITEM *pSMS = (SCHEDULE_SMS_ITEM*)calloc(1,sizeof(SCHEDULE_SMS_ITEM));
		_tcscpy(pSMS->szTo,szPhones);
		_tcscpy(pSMS->szContent,szContent);
		pSMS->ftSendTime = ftTime;

		PECore::PostMessage(WM_SEND_SCHEDULE_SMS,WPARAM(pSMS),0);

		return;
	}

	PECore::PostMessage(WM_SEND_SMS,WPARAM(szPhones),LPARAM(szContent));

#ifndef _DEBUG
	PECore::PostDelayMessage(WM_DISABLE_SEND_BTN,(WPARAM)m_hWnd,1,60);
#else
	PECore::PostDelayMessage(WM_DISABLE_SEND_BTN,(WPARAM)m_hWnd,1,3);
#endif

	m_btnSendNow.EnableWindow(0);

}

void CNewSmsDlg::OnBnClickedBtnCancel()
{
	m_pac->Clear();
	m_pac->Unbind();
	//delete m_pac;

	SendMessage(WM_CLOSE);
}

void CNewSmsDlg::OnClose()
{
	g_hNewSmsWnd = 0;

	CDialog::OnClose();
}


#include "SmsListDlg.h"
void CNewSmsDlg::OnBnClickedSmsListBtn()
{
	CSmsListDlg *pSMSDlg = new CSmsListDlg();
	pSMSDlg->DoModal();

	delete pSMSDlg;
}

void CNewSmsDlg::OnBnClickedInsertContactBtn()
{
	CString sContact;
	GetDlgItemText(IDC_EDIT_INSERT_CONTACT,sContact);

	if (sContact.GetLength() < 1)
	{
		return;
	}

	CString sPhones;
	GetDlgItemText(IDC_EDIT_TO_LIST,sPhones);

	if (sPhones.GetLength() > 1)
	{
		sPhones += _T(",");
		
	}
	
	sPhones += sContact;

	SetDlgItemText(IDC_EDIT_TO_LIST,sPhones);

	SetDlgItemText(IDC_EDIT_INSERT_CONTACT,_T(""));
}

void CNewSmsDlg::OnBnClickedSendAtTime()
{
	CButton* pBtn = (CButton*)GetDlgItem(IDC_SEND_AT_TIME);
	if ( pBtn->GetCheck() )
	{
		m_btnSendNow.SetWindowText(_T("Save"));
	}
	else
	{
		m_btnSendNow.SetWindowText(_T("Send Now"));
	}
}

#define MAX_SMS_LEN		160
void CNewSmsDlg::OnEnChangeEditSmsContent()
{
	CString sTxt;
	GetDlgItemText(IDC_EDIT_SMS_CONTENT,sTxt);

	int iLen = sTxt.GetLength();
	if (iLen < 1)
	{
		SetDlgItemText(IDC_STATIC_TEXT_COUNT,_T(""));
	}

	CString sInfo;
	if (iLen < MAX_SMS_LEN)
	{
		sInfo.Format(_T("%d character entered"),iLen);
	}
	else
	{
		int iCount = iLen/MAX_SMS_LEN;
		if (iLen % MAX_SMS_LEN)
		{
			++iCount;
		}
		sInfo.Format(_T("%d character entered, will sent by %d SMS"),iLen, iCount);
	}

	SetDlgItemText(IDC_STATIC_TEXT_COUNT,sInfo);
}
