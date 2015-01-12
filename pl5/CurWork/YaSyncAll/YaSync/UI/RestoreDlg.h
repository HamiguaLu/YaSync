#pragma once
#include "resource.h"
#include "BtnST.h"
#include "afxcmn.h"
// CRestoreDlg dialog

class CRestoreDlg : public CDialog
{
	DECLARE_DYNAMIC(CRestoreDlg)

public:
	CRestoreDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRestoreDlg();

// Dialog Data
	enum { IDD = IDD_RESTORE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	BOOL PtInClientRect(CPoint p);

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;

	CButtonST m_btnStartRestore;
	CButtonST m_btnCancelRestore;
public:
	afx_msg void OnBnClickedBtnRestore();
	afx_msg void OnBnClickedBtnCancelRestore();
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	CProgressCtrl m_RestoreProgress;

	TCHAR* m_szFolderID;

	int m_iRestoreState;//0:not in restore state  1:in restore state


	void OnRestoreState(PE_RESTORE_STATUS *pInfo);
protected:
//	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
