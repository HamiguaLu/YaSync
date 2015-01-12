#pragma once
#include "resource.h"
#include "reportctrl.h"
#include "BtnST.h"
#include "afxwin.h"
#include "afxdtctl.h"
#include "CustomAutoComplete.h"

// CNewSmsDlg dialog

class CNewSmsDlg : public CDialog
{
	DECLARE_DYNAMIC(CNewSmsDlg)

public:
	CNewSmsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNewSmsDlg();

// Dialog Data
	enum { IDD = IDD_NEW_SMS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	BOOL PtInClientRect(CPoint p);

	HBRUSH m_hDialogBrush;

	CButtonST m_btnSendNow;
	CButtonST m_btnClose;
	CButtonST m_btnSelAll;
	CButtonST m_btnInsert;
	CButtonST m_btnSmsList;

	CReportCtrl m_Contacts;

	CCustomAutoComplete* m_pac;

	//void ShowSentReport(TCHAR *szName,int iSent);
public:
	TCHAR m_szReceiver[1024];

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnBnClickedBtnSelAll();
	afx_msg void OnBnClickedBtnSendNow();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnClose();
	afx_msg void OnBnClickedSmsListBtn();
	CDateTimeCtrl m_yearPicker;
	CDateTimeCtrl m_timePicker;

	void SetAutoComplete(list<CONTACT_ITEM*> *pList);
	afx_msg void OnBnClickedInsertContactBtn();
	afx_msg void OnBnClickedSendAtTime();
	afx_msg void OnEnChangeEditSmsContent();
};
