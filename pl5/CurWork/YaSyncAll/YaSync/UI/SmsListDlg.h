#pragma once
#include "resource.h"
#include "reportctrl.h"
#include "BtnST.h"
#include "PEProtocol.h"

// CSmsListDlg dialog

class CSmsListDlg : public CDialog
{
	DECLARE_DYNAMIC(CSmsListDlg)

public:
	CSmsListDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSmsListDlg();

// Dialog Data
	enum { IDD = IDD_SMS_LIST_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	BOOL PtInClientRect(CPoint p);

	HBRUSH m_hDialogBrush;

	CButtonST m_btnDelSMS;
	CButtonST m_btnCancel;

public:
	CReportCtrl m_SmsList;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedDelSmsBtn();
	afx_msg void OnBnClickedCancel();
};
