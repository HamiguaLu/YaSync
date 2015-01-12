#pragma once
#include "reportctrl.h"
#include "BtnST.h"
#include "afxwin.h"
#include "afxdtctl.h"
#include "reportctrl.h"

// CSettingDeviceUI dialog

class CSettingDeviceUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingDeviceUI)

public:
	CSettingDeviceUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingDeviceUI();

// Dialog Data
	enum { IDD = IDD_SETTING_DEVICE_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnSave();

	CReportCtrl m_FilterList;
	afx_msg void OnNMClickDeviceList(NMHDR *pNMHDR, LRESULT *pResult);
};
