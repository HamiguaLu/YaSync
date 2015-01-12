#pragma once

#include "BtnST.h"
#include "reportctrl.h"
#include "afxwin.h"
#include "afxdtctl.h"

// CSettingCalendarUI dialog

class CSettingCalendarUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingCalendarUI)

public:
	CSettingCalendarUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingCalendarUI();

// Dialog Data
	enum { IDD = IDD_SETTING_CALENDAR_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;

	
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSave();

	void OnDeviceConnected();
	afx_msg void OnBnClickedEnableCalSync();
	CReportCtrl m_CalList;
	CReportCtrl m_CalSyncFolder;

	void OnCalendarListComing(int iState);

	void OnSubFolderListComing(list<TCHAR*> *pList);
	afx_msg void OnBnClickedCheckSyncAfterTime();
	CComboBox m_cbSyncType;

	afx_msg void OnNMClickCalList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickCalFolderNeedSync(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCbnSelchangeCalSyncType();
	afx_msg void OnCbnSelchangeComboYear();
	afx_msg void OnCbnSelendcancelComboMonth();
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	CDateTimeCtrl m_calendarDate;
	CDateTimeCtrl m_calendarTime;
	afx_msg void OnDtnDatetimechangeCalendarDatePicker(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDtnDatetimechangeCalendarTimePicker(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCheckOnlySyncSelectedCals();
};


