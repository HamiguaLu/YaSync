#pragma once

#include "BtnST.h"
#include "reportctrl.h"
#include "afxwin.h"
// CSettingContactUI dialog

class CSettingContactUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingContactUI)

public:
	CSettingContactUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingContactUI();

// Dialog Data
	enum { IDD = IDD_SETTING_CONTACT_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();

	void OnDeviceConnected();
	afx_msg void OnBnClickedEnableContactSync();
	CReportCtrl m_AndroidAccountList;
	CReportCtrl m_ContactFolderList;

	void OnSubFolderListComing(list<TCHAR*> *pList);
	void OnAccountsComing(int iState);

	CComboBox m_cbSyncType;
	afx_msg void OnCbnSelchangeContactSyncType();
	afx_msg void OnBnClickedEnableContactAccount();
	afx_msg void OnNMClickAndroidAccountList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickContactFolder(NMHDR *pNMHDR, LRESULT *pResult);
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};



