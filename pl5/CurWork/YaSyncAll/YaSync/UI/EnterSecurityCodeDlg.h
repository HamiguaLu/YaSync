#pragma once
#include "peprotocol.h"
#include "resource.h"
#include "BtnST.h"

// CEnterSecurityCodeDlg dialog

class CEnterSecurityCodeDlg : public CDialog
{
	DECLARE_DYNAMIC(CEnterSecurityCodeDlg)

public:
	CEnterSecurityCodeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEnterSecurityCodeDlg();

// Dialog Data
	enum { IDD = IDD_ENTER_SECURITY_CODE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	HBRUSH m_hDialogBrush;

	CButtonST m_btnOK;
	CButtonST m_btnCancel;

	CButtonST m_btnWhatCode;

	BOOL PtInClientRect(CPoint p);
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedOk();

	CString m_sPhoneID;
	CString m_sPhoneName;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBtnWhatSecurityCode();
	afx_msg void OnBnClickedAddBlockDevice();

	static list<TCHAR *> m_filterList;
	static int IsItemInFilterList(TCHAR *contact);
	static int FreeFilterList();
};

