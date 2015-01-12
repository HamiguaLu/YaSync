#pragma once

#include "resource.h"
#include "reportctrl.h"
#include "BtnST.h"
#include "afxwin.h"
#include "afxdtctl.h"

// CSettingFilterUI dialog

class CSettingFilterUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingFilterUI)

public:
	CSettingFilterUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingFilterUI();

// Dialog Data
	enum { IDD = IDD_SETTING_FILTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;
	CButtonST m_btnAdd;
	CButtonST m_btnSelect;
	CButtonST m_btnApply;
	CReportCtrl m_FilterList;
	
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSelectAll();
	afx_msg void OnBnClickedBtnApplyFilter();
	afx_msg void OnSave();

	afx_msg void OnBnClickedBtnAddFilter();

	void SetFilter(TCHAR *szName,TCHAR *szPhone);
};
