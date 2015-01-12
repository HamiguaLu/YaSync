#pragma once

#include "MyTabCtrl.h"

#include "SettingCalendarUI.h"
#include "SettingCallLogUI.h"
#include "SettingCommonUI.h"
#include "SettingContactUI.h"
#include "SettingDeviceUI.h"
#include "SettingSmsUI.h"
#include "SettingFilterUI.h"
#include "SettingMediaUI.h"


#include "BtnST.h"


// CSettingUI dialog

class CSettingUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingUI)

public:
	CSettingUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingUI();

// Dialog Data
	enum { IDD = IDD_SETTING_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;

	CTabCtrlEx m_tabDlgList;

	CButtonST m_btnSave;
	CButtonST m_btnReset;
	
	CSettingCalendarUI		m_CalendarUI;
	CSettingCallLogUI		m_CallLogUI;
	CSettingCommonUI		m_CommonUI;
	CSettingContactUI		m_ContactUI;
	CSettingDeviceUI		m_DeviceUI;
	CSettingSmsUI			m_SmsUI;
	CSettingFilterUI		m_FilterUI;
	CSettingMediaUI			m_MediaUI;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnTcnSelchangeSettingTab(NMHDR *pNMHDR, LRESULT *pResult);
	void OnDeviceConnected();
	void OnAddFilter(TCHAR *szName,TCHAR *szPhone);
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnBnClickedBtnSaveSetting();
	afx_msg void OnBnClickedBtnResetSetting();
};
