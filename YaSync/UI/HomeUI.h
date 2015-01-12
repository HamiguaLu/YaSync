#pragma once

#include "afxcmn.h"
#include "afxwin.h"
#include "BtnST.h"
#include "TextProgressCtrl.h"

#include "UIContainer.h"
#include "RestoreDlg.h"
#include "NewSmsDlg.h"


// CHomeUI dialog

class CHomeUI : public CDialog
{
	DECLARE_DYNAMIC(CHomeUI)

public:
	CHomeUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHomeUI();

// Dialog Data
	enum { IDD = IDD_MAIN_UI };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;

	CButtonST m_btnQuickStart;

	CButtonST m_btnConnect;
	CButtonST m_btnSwitch;

	CButtonST m_btnNewSMS;
	CButtonST m_btnRestore;
	CButtonST m_btnPhoto;
	CButtonST m_btnApp;
	CButtonST m_btnFile;
	
public:
	virtual BOOL OnInitDialog();

	void OnDeviceInfo(PE_DEV_INFO *pInfo);
	void OnDeviceConnectionChange(int iConnected);
	void OnDisableConnBtn(int iDisable);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnConnect();
	afx_msg void OnBnClickedSwitch2PhoneBtn();
	CIPAddressCtrl m_DevIP;
	CTextProgressCtrl m_batteryLevel;
	CTextProgressCtrl m_progSdCard;
	CComboBox m_ConnPhones;
protected:
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
public:
	afx_msg void OnCbnSelchangeComboConnectedPhones();
	afx_msg void OnBnClickedBtnNewSms();
	afx_msg void OnBnClickedBtnRestore();
	afx_msg void OnBnClickedBtnAppManager();
	afx_msg void OnBnClickedBtnFile();
	afx_msg void OnBnClickedBtnPhoto();

	void RestoreFromFolder(TCHAR *szFolderID);
	void StartUI(int iUIType);
	int GetToolURL(int iUIType,OUT TCHAR *szHost);

public:
	afx_msg void OnBnClickedBtnQuickStart();
};
