#pragma once

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

#include "resource.h"

// CUIContainer dialog

class CUIContainer : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CUIContainer)

public:
	CUIContainer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUIContainer();

// Dialog Data
	enum { IDD = IDD_HTML_UI, IDH = IDR_HTML_UICONTAINER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL PtInClientRect(CPoint p);

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void SetURL(TCHAR *szURL);
	void Go2Black();
	int m_width;
	int m_height;

private:
	TCHAR m_szURL[255];
		
	BOOL AdjustSize();

	HBRUSH m_hDialogBrush;
public:
	virtual BOOL OnInitDialog();
};
