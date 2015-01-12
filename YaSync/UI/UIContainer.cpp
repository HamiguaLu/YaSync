// UIContainer.cpp : implementation file
//

#include "stdafx.h"
#include "YaSync.h"
#include "PEProtocol.h"
#include "UIContainer.h"
#include "AppTools.h"


// CUIContainer dialog

IMPLEMENT_DYNCREATE(CUIContainer, CDHtmlDialog)

CUIContainer::CUIContainer(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CUIContainer::IDD, CUIContainer::IDH, pParent)
{
	m_hDialogBrush = CreateSolidBrush(SUB_DLG_BK_COLOR);
}

CUIContainer::~CUIContainer()
{
}

void CUIContainer::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CUIContainer, CDHtmlDialog)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CUIContainer)

END_DHTML_EVENT_MAP()



// CUIContainer message handlers

BOOL CUIContainer::PtInClientRect(CPoint p)
{
	CRect rc;
	GetClientRect(&rc);
	ClientToScreen(&rc);
	return rc.PtInRect(p);
}

LRESULT CUIContainer::OnNcHitTest(CPoint point)
{
	return PtInClientRect(point) ? HTCAPTION : CDialog::OnNcHitTest(point);
}

void CUIContainer::SetURL(TCHAR *szURL)
{
	_tcscpy(m_szURL,szURL);
}

BOOL CUIContainer::AdjustSize()
{
	RECT r;
	this->GetParent()->GetWindowRect(&r);

#if 0
	int iParentWidth = r.right - r.left;
	int iParentHeight = r.bottom - r.top;

	int x = (iParentWidth - m_width) / 2;
	int y = (iParentHeight - m_height) / 2;
#else
	int x = r.left;
	int y = r.top;
#endif
	if (x < 0)
	{
		x = 0;
	}

	if (y < 0)
	{
		y = 0;
	}

	MoveWindow( x,y,m_width,m_height);

	Navigate(m_szURL);

	return TRUE;
}


BOOL CUIContainer::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	AdjustSize();

	SetWinOnOutlook(m_hWnd);
	return TRUE;
}



