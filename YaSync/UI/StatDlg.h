#pragma once
#include "resource.h"
#include "BtnST.h"
#include "afxcmn.h"


// CStatDlg dialog

class CStatDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatDlg)

public:
	CStatDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatDlg();

// Dialog Data
	enum { IDD = IDD_STAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL PtInClientRect(CPoint p);

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;
	CButtonST m_btnOK;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();

	void SetStatInfo(SMS_STAT *pSmsStat,CALL_LOG_STAT *pCallStat);
	SMS_STAT *m_pSmsStat;
	CALL_LOG_STAT *m_pCallStat;
	afx_msg void OnBnClickedBtnStatOk();
};

