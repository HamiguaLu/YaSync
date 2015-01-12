// YaSyncDlg.h : header file
//

#pragma once
#include "BtnST.h"
#include "SyncUI.h"
#include "SettingUI.h"
#include "HomeUI.h"
#include "BuyUI.h"
#include "HelpUI.h"
#include "TrayDlg.h"


// CYaSyncDlg dialog
class CYaSyncDlg : public CDialog
{
// Construction
public:
	CYaSyncDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_YASYNC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	CHomeUI			m_homeUI;
	CSettingUI		m_settingUI;
	CSyncUI			m_syncUI;
	CHelpUI			m_helpUI;
	CBuyUI			m_buyUI;
	CTrayDlg 		m_trayDlg;

	CButtonST m_btnHome;
	CButtonST m_btnSetting;
	CButtonST m_btnSync;
	CButtonST m_btnBuy;
	CButtonST m_btnHelp;

	CButtonST m_btnMIN;
	CButtonST m_btnClose;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	HBRUSH m_hDialogBrush;

	void OnOutlookCmd(DWORD dwCmd,TCHAR *szData);

	int SetBtnColor(int iFocusIndex);

	BOOL PtInClientRect(CPoint p);
public:
	afx_msg void OnBnClickedHomeBtn();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedSyncBtn();
	afx_msg void OnBnClickedSettingBtn();
	afx_msg void OnBnClickedHelpBtn();
	afx_msg void OnBnClickedBuyBtn();
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnAaaShowmainui();
	afx_msg void OnAaaNewsms();

	int m_iAutoStart;
	
	afx_msg void OnNcPaint();
	afx_msg void OnAaaFilemanager();
	afx_msg void OnAaaPhotoviewer();
	afx_msg void OnAaaExit();

	afx_msg void OnAaaScandevice();
	afx_msg void OnBnClickedCloseBtn();
	afx_msg void OnBnClickedMinBtn();

	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
};

