#pragma once

#include "BtnST.h"
#include "afxwin.h"
// CSettingCallLogUI dialog

class CSettingCallLogUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingCallLogUI)

public:
	CSettingCallLogUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingCallLogUI();

// Dialog Data
	enum { IDD = IDD_SETTING_CALLLOG_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();

	void OnDeviceConnected();
	afx_msg void OnBnClickedEnableCalllogSync();
	afx_msg void OnBnClickedEnableCallLogFolder();
	afx_msg void OnCbnSelchangeCallLogFolder();
	void OnSubFolderListComing(list<TCHAR*> *pList);

public:
	CComboBox m_CallLogFolderList;
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};
