#pragma once

#include "BtnST.h"

// CBuyUI dialog

class CBuyUI : public CDialog
{
	DECLARE_DYNAMIC(CBuyUI)

public:
	CBuyUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBuyUI();

// Dialog Data
	enum { IDD = IDD_BUY_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;

	CButtonST m_btnBuy;
	CButtonST m_btnActivate;
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnBuyOnline();
	afx_msg void OnBnClickedBtnActivate();
};
