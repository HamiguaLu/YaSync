// MyTabCtrl.cpp : implementation file
//

#include "stdafx.h"

#include "MyTabCtrl.h"
#include "PEProtocol.h"

// CTabCtrlEx

IMPLEMENT_DYNAMIC(CTabCtrlEx, CTabCtrl)

CTabCtrlEx::CTabCtrlEx()
{

}

CTabCtrlEx::~CTabCtrlEx()
{
}


BEGIN_MESSAGE_MAP(CTabCtrlEx, CTabCtrl)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CTabCtrlEx message handlers



BOOL CTabCtrlEx::OnEraseBkgnd(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	pDC->FillSolidRect(rcClient, SUB_DLG_BK_COLOR);
	// Bkg Color for your Tab Control
	return TRUE; 
}

void CTabCtrlEx::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	CRect rc(lpDIS->rcItem);
	TCHAR szTabText[255];
	TC_ITEM tci;
	tci.cchTextMax = sizeof(szTabText)-1;
	tci.pszText = szTabText;
	tci.mask = TCIF_TEXT;
	GetItem(lpDIS->itemID, &tci);
	pDC->SetBkColor(SUB_DLG_BK_COLOR);
	pDC->FillSolidRect(&lpDIS->rcItem, SUB_DLG_BK_COLOR);

	if (lpDIS->itemState & ODS_SELECTED)
	{
		pDC->SetTextColor(RGB(0,0,255));
		pDC->DrawText(szTabText, &lpDIS->rcItem , DT_CENTER | DT_VCENTER |
			DT_SINGLELINE);
	}
	else
	{
		pDC->SetTextColor(RGB(0,0,0));
		pDC->DrawText(szTabText, &lpDIS->rcItem , DT_CENTER | DT_BOTTOM |
			DT_SINGLELINE);
	} 
}
