//--------------------------------------------------------------------------------------------
//  Name:           CCustomAutoComplete (CCUSTOMAUTOCOMPLETE.H)
//  Type:           Wrapper class
//  Description:    Matches IAutoComplete, IEnumString and the registry (optional) to provide
//					custom auto-complete functionality for EDIT controls - including those in
//					combo boxes - in WTL projects.
//
//  Author:         Klaus H. Probst [kprobst@vbbox.com]
//  URL:            http://www.vbbox.com/
//  Copyright:      This work is copyright ?2002, Klaus H. Probst
//  Usage:          You may use this code as you see fit, provided that you assume all
//                  responsibilities for doing so.
//  Distribution:   Distribute freely as long as you maintain this notice as part of the
//					file header.
//
//
//  Updates:        
//
//
//  Notes:			
//
//
//  Dependencies:
//
//					The usual ATL/WTL headers for a normal EXE, plus <atlmisc.h>
//
//--------------------------------------------------------------------------------------------

#if !defined(CCustomAutoComplete_INCLUDED)
#define CCustomAutoComplete_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Bring in the GUID hack and the shell stuff
#include <initguid.h>
#include <shldisp.h>
#include <shlguid.h>

class CCustomAutoComplete : public IEnumString
{

private:

	CSimpleArray<CString> m_asList;
	CComPtr<IAutoComplete> m_pac;

	ULONG m_nCurrentElement;
	ULONG m_nRefCount;
	BOOL m_fBound;

public:

	// Constructors/destructors

	CCustomAutoComplete();
	CCustomAutoComplete(const CSimpleArray<CString>& p_sItemList);
	~CCustomAutoComplete();

public:

	// Implementation
	BOOL SetList(const CSimpleArray<CString>& p_sItemList);
	BOOL Bind(HWND p_hWndEdit, DWORD p_dwOptions = 0, LPCTSTR p_lpszFormatString = NULL);
	VOID Unbind();
	BOOL AddItem(CString& p_sItem,int abc);
	BOOL AddItem(LPCTSTR p_lpszItem);
	INT GetItemCount();
	BOOL RemoveItem(CString& p_sItem,int abc);
	BOOL RemoveItem(LPCTSTR p_lpszItem);
	BOOL Clear();
	BOOL Disable();
	BOOL Enable(VOID);
public:

	//
	//	IUnknown implementation
	//
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject);

public:

	//
	//	IEnumString implementation
	//
	STDMETHODIMP Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched);
	STDMETHODIMP Skip(ULONG celt);
	STDMETHODIMP Reset(void);
	STDMETHODIMP Clone(IEnumString** ppenum);

private:

	// Internal implementation
	void InternalInit();
	HRESULT EnDisable(BOOL p_fEnable);
};

#endif // !defined(CCustomAutoComplete_INCLUDED)