#pragma once

#include "BtnST.h"

// CHelpUI dialog

class CHelpUI : public CDialog
{
	DECLARE_DYNAMIC(CHelpUI)

public:
	CHelpUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHelpUI();

	// Dialog Data
	enum { IDD = IDD_HELP_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;

	CButtonST m_btnTroublShooting;
	CButtonST m_btnGetLog;

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnTroubleShooting();
	//afx_msg void OnBnClickedBtnSendMail();
	afx_msg void OnBnClickedBtnGetLogFile();
};
