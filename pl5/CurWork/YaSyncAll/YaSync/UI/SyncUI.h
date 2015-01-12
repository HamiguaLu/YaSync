#pragma once

#include "BtnST.h"
#include "afxcmn.h"
#include "TextProgressCtrl.h"

// CSyncUI dialog

class CSyncUI : public CDialog
{
	DECLARE_DYNAMIC(CSyncUI)

public:
	CSyncUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSyncUI();

// Dialog Data
	enum { IDD = IDD_SYNC_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;

	CButtonST m_btnStartSync;
	int m_iSyncDone;
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnStartSync();
	int OnSyncInfo(PE_SYNC_STATUS *s);
	int OnSyncProcess(int iStart);

	void LoadSetting();
	
	CTextProgressCtrl m_SyncProgress;
	afx_msg void OnBnClickedCheckSyncImage();
	afx_msg void OnBnClickedCheckSyncVideo();
	afx_msg void OnBnClickedCheckSyncAudio();
};

