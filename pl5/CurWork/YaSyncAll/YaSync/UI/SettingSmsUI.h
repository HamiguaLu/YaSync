#pragma once

#include "BtnST.h"
#include "afxwin.h"

// CSettingSmsUI dialog

class CSettingSmsUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingSmsUI)

public:
	CSettingSmsUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingSmsUI();

// Dialog Data
	enum { IDD = IDD_SETTING_SMS_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();

	void OnDeviceConnected();
	afx_msg void OnBnClickedEnableSmsSync();
	afx_msg void OnBnClickedEnableMmsSync();
	afx_msg void OnBnClickedStoreSmsToFolder();
	afx_msg void OnCbnSelchangeSmsFolderList();
	afx_msg void OnBnClickedCheckSmsReadFlag();

	void OnSubFolderListComing(list<TCHAR*> *pList);

public:
	CComboBox m_SMSFolderList;
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnBnClickedShowSmsReport();
};
