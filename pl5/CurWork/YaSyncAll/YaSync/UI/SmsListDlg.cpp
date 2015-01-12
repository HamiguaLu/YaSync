// SmsListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SmsListDlg.h"
#include "PECore.h"

// CSmsListDlg dialog

IMPLEMENT_DYNAMIC(CSmsListDlg, CDialog)

CSmsListDlg::CSmsListDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSmsListDlg::IDD, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CSmsListDlg::~CSmsListDlg()
{
}

void CSmsListDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SMS_LIST, m_SmsList);
}


BEGIN_MESSAGE_MAP(CSmsListDlg, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(ID_DEL_SMS_BTN, &CSmsListDlg::OnBnClickedDelSmsBtn)
	ON_BN_CLICKED(IDCANCEL, &CSmsListDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CSmsListDlg message handlers

HBRUSH CSmsListDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

BOOL CSmsListDlg::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}


LRESULT CSmsListDlg::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

BOOL CSmsListDlg::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (WM_SCHEDULE_SMS_LIST_COMING == message)
	{
		list<SCHEDULE_SMS_ITEM*> *pList = (list<SCHEDULE_SMS_ITEM*> *)wParam;

		if (!pList || pList->size() < 1)
		{
			MessageBox(_T("No Scheduled SMS found in device"),_T("Error"),MB_ICONSTOP|MB_OK);
			return TRUE;
		}

		list<SCHEDULE_SMS_ITEM*>::iterator it1;
		for (it1 = pList->begin(); it1 != pList->end(); ++it1)
		{ 
			SCHEDULE_SMS_ITEM *pSMS = (SCHEDULE_SMS_ITEM*)*it1;

			SYSTEMTIME stTime;
			FileTimeToSystemTime(&pSMS->ftSendTime,&stTime);

			TCHAR szDate[255];
			GetDateFormat(LOCALE_USER_DEFAULT,0,&stTime,NULL,szDate,255);

			TCHAR szTime[255];
			GetTimeFormat(LOCALE_USER_DEFAULT,0,&stTime,NULL,szTime,255);

			CString sTime = szDate;
			sTime += _T(" ");
			sTime += szTime;
			
			CString sID;
			sID.Format(_T("%d"),pSMS->ulID);
			m_SmsList.InsertItem(0,pSMS->szTo,pSMS->szContent,sTime.GetBuffer(),sID);
			sTime.ReleaseBuffer();

			free(pSMS);
		}

		pList->clear();
		return TRUE;
	}

	return CDialog::OnWndMsg(message, wParam, lParam, pResult);
}

BOOL CSmsListDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_btnDelSMS.SubclassDlgItem(ID_DEL_SMS_BTN, this);
	m_btnDelSMS.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN_COLOR);
	m_btnDelSMS.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN_SEL_COLOR);
	m_btnDelSMS.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN_COLOR);
	m_btnDelSMS.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN_FONT_FOCUS_COLOR);
	m_btnDelSMS.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN_FONT_COLOR);
	m_btnDelSMS.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN_FONT_COLOR);

	m_btnCancel.SubclassDlgItem(IDCANCEL, this);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_OUT,SUB_BTN2_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_IN,SUB_BTN2_SEL_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_BK_FOCUS,SUB_BTN2_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_IN,SUB_BTN2_FONT_FOCUS_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_OUT,SUB_BTN2_FONT_COLOR);
	m_btnCancel.SetColor(CButtonST::BTNST_COLOR_FG_FOCUS,SUB_BTN2_FONT_COLOR);

	m_SmsList.SetHeadings(_T("To, 220; Content,220;SendTime,130;id,0"));
	m_SmsList.SetGridLines(TRUE);
	m_SmsList.SetCheckboxes(TRUE);
	m_SmsList.SetFullRowSelect(TRUE);

	PECore::PostMessage(WM_GET_SCHEDULE_SMS_LIST,(WPARAM)m_hWnd,0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CSmsListDlg::OnBnClickedDelSmsBtn()
{
	int iCount = m_SmsList.GetItemCount();
	for (int iItem = 0; iItem < iCount; ++iItem)
	{
		BOOL bCheck = m_SmsList.IsItemChecked(iItem);
		if (!bCheck)
		{
			continue;
		}

		CString sID = m_SmsList.GetItemText(iItem, 3);

		ULONG ulID = _ttoi(sID.GetBuffer());
		sID.ReleaseBuffer();

		PECore::PostMessage(WM_DEL_SCHEDULE_SMS,ulID,0);

		m_SmsList.DeleteItem(iItem);

		--iItem;
		--iCount;
	}

}

void CSmsListDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}
