#pragma once
#include "resource.h"
#include "BtnST.h"
// CInstallDlg dialog

class CInstallDlg : public CDialog
{
	DECLARE_DYNAMIC(CInstallDlg)

public:
	CInstallDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInstallDlg();

// Dialog Data
	enum { IDD = IDD_INSTALL_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;

	CButtonST m_btnStartInstall;
	CButtonST m_btnCancel;

	BOOL PtInClientRect(CPoint p);
public:
	afx_msg void OnBnClickedStartInstall();
	afx_msg void OnBnClickedCancel();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
