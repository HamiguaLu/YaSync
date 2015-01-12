#pragma once

#include "resource.h"
#include "BtnST.h"



// CSaveContactsDlg dialog

class CSaveContactsDlg : public CDialog
{
	DECLARE_DYNAMIC(CSaveContactsDlg)

public:
	CSaveContactsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSaveContactsDlg();

// Dialog Data
	enum { IDD = IDD_ADD_CONTACT_DLG };

	HBRUSH m_hDialogBrush;

	CButtonST m_btnSave;
	CButtonST m_btnCancel;

	void SetContacts(TCHAR *szName,TCHAR* szPhone);

	TCHAR *m_szName;
	TCHAR *m_szPhone;

	BOOL PtInClientRect(CPoint p);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnSaveContacts();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedBtnCancel();
};
