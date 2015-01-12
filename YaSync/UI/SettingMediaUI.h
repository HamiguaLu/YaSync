#pragma once

#include "BtnST.h"
#include "reportctrl.h"
#include "afxwin.h"
#include "afxdtctl.h"
#include "resource.h"

// CSettingMediaUI dialog

class CSettingMediaUI : public CDialog
{
	DECLARE_DYNAMIC(CSettingMediaUI)

public:
	CSettingMediaUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingMediaUI();

	HBRUSH m_hDialogBrush;

// Dialog Data
	enum { IDD = IDD_SETTING_MEDIA };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CButtonST m_btnImagePath;
	CButtonST m_btnVideoPath;
	CButtonST m_btnAudioPath;

	DECLARE_MESSAGE_MAP()
public:
	void OnDeviceConnected();
	afx_msg void OnSave();

	afx_msg void OnBnClickedBtnGetImagePath();
	afx_msg void OnBnClickedBtnGetVideoPath();
	afx_msg void OnBnClickedBtnGetAudioPath();
	afx_msg void OnBnClickedEnableImageSync();
	afx_msg void OnBnClickedEnableVideoSync();
	afx_msg void OnBnClickedEnableAudioSync();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	virtual BOOL OnInitDialog();
};
