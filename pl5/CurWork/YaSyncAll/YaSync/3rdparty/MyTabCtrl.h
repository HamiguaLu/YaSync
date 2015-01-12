#pragma once


// CTabCtrlEx

class CTabCtrlEx : public CTabCtrl
{
	DECLARE_DYNAMIC(CTabCtrlEx)

public:
	CTabCtrlEx();
	virtual ~CTabCtrlEx();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
};


