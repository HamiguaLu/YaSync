#pragma once

#include "BtnST.h"
#include "afxwin.h"

// CSettingCommonUI dialog

class CSettingCommonUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingCommonUI)

public:
	CSettingCommonUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingCommonUI();

// Dialog Data
	enum { IDD = IDD_SETTING_COMMON_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();
	afx_msg void OnBnClickedAutoStart();
	afx_msg void OnBnClickedAutoSync();
	afx_msg void OnBnClickedCheckUseDefaultProfile();
	afx_msg void OnCbnSelchangeOutlookProfile();
	afx_msg void OnBnClickedCheckExitAfterSync();

	void LoadSetting();
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	CComboBox m_profileList;
};
