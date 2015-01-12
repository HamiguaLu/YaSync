#pragma once
#include "resource.h"
#include "peprotocol.h"

#include "BtnST.h"

// CTrayDlg dialog



class CTrayDlg : public CDialog
{
	DECLARE_DYNAMIC(CTrayDlg)

public:
	CTrayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrayDlg();

// Dialog Data
	enum { IDD = IDD_TRAY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

	void ShowMsg(TRAY_MSG *pMsg);
	list<TRAY_MSG*> m_MsgList;
	
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedNextMsgBtn();

	CButtonST m_btnReply;
	CButtonST m_btnNextMsg;
	CButtonST m_btnClose;

	HBRUSH m_hDialogBrush;

	TCHAR m_szReplyAddr[1024];
	
	void ShowOneMsg();

	void Move2Bottome();
		
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	BOOL PtInClientRect(CPoint p);
	afx_msg void OnBnClickedCloseBtn();
	afx_msg void OnBnClickedReplyBtn();

	void FreeAllMsg();
	void Free1stMsg();

public:
	afx_msg void OnClose();
};



