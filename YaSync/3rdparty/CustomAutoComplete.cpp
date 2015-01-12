
#include "stdafx.h"

#include "CustomAutoComplete.h"

// Constructors/destructors

CCustomAutoComplete::CCustomAutoComplete()
{
	InternalInit();
}


CCustomAutoComplete::CCustomAutoComplete(const CSimpleArray<CString>& p_sItemList)
{
	InternalInit();
	SetList(p_sItemList);
}


CCustomAutoComplete::~CCustomAutoComplete()
{
	m_asList.RemoveAll();

	if (m_pac)
		m_pac.Release();

}

BOOL CCustomAutoComplete::SetList(const CSimpleArray<CString>& p_sItemList)
{
	ATLASSERT(p_sItemList.GetSize() != 0);

	Clear();
	m_asList = p_sItemList;

	return TRUE;
}


BOOL CCustomAutoComplete::Bind(HWND p_hWndEdit, DWORD p_dwOptions, LPCTSTR p_lpszFormatString)
{
	ATLASSERT(::IsWindow(p_hWndEdit));

	if ((m_fBound) || (m_pac))
		return FALSE;

	HRESULT hr = S_OK;

	hr = m_pac.CoCreateInstance(CLSID_AutoComplete);

	if (SUCCEEDED(hr))
	{
		if (p_dwOptions)
		{
			CComQIPtr<IAutoComplete2> pAC2(m_pac);

			ATLASSERT(pAC2);

			hr = pAC2->SetOptions(p_dwOptions);			// This never fails?
			pAC2.Release();

		}

		hr = m_pac->Init(p_hWndEdit, this, NULL, p_lpszFormatString);

		if (SUCCEEDED(hr))
		{
			m_fBound = TRUE;
			return TRUE;
		}

	}

	return FALSE;
}

VOID CCustomAutoComplete::Unbind()
{
	if (!m_fBound)
		return;

	ATLASSERT(m_pac);

	if (m_pac)
	{
		m_pac.Release();
		m_fBound = FALSE;
	}
}

BOOL CCustomAutoComplete::AddItem(CString& p_sItem,int abc)
{
	if (p_sItem.GetLength() != 0)
	{
		if (m_asList.Find(p_sItem) == -1)
		{
			m_asList.Add(p_sItem);

			return TRUE;
		}
	}

	return FALSE;
}

BOOL CCustomAutoComplete::AddItem(LPCTSTR p_lpszItem)
{
	return AddItem(CString(p_lpszItem),0);
}

INT CCustomAutoComplete::GetItemCount()
{
	return m_asList.GetSize();
}


BOOL CCustomAutoComplete::RemoveItem(CString& p_sItem,int abc)
{
	if (p_sItem.GetLength() != 0)
	{
		if (m_asList.Find(p_sItem) != -1)
		{
			m_asList.Remove(p_sItem);
			return TRUE;
		}

	}

	return FALSE;

}

BOOL CCustomAutoComplete::RemoveItem(LPCTSTR p_lpszItem)
{
	return RemoveItem(CString(p_lpszItem),0);
}


BOOL CCustomAutoComplete::Clear()
{
	if (m_asList.GetSize() != 0)
	{
		m_asList.RemoveAll();

		return TRUE;
	}

	return FALSE;

}

BOOL CCustomAutoComplete::Disable()
{
	if ((!m_pac) || (!m_fBound))
		return FALSE;

	return SUCCEEDED(EnDisable(FALSE));

}

BOOL CCustomAutoComplete::Enable(VOID)
{
	if ((!m_pac) || (m_fBound))
		return FALSE;

	return SUCCEEDED(EnDisable(TRUE));

}


STDMETHODIMP_(ULONG) CCustomAutoComplete::AddRef()
{
	return ::InterlockedIncrement(reinterpret_cast<LONG*>(&m_nRefCount));
}

STDMETHODIMP_(ULONG) CCustomAutoComplete::Release()
{
	ULONG nCount = 0;
	nCount = (ULONG) ::InterlockedDecrement(reinterpret_cast<LONG*>(&m_nRefCount));

	if (nCount == 0)
		delete this;

	return nCount;

}

STDMETHODIMP CCustomAutoComplete::QueryInterface(REFIID riid, void** ppvObject)
{
	HRESULT hr = E_NOINTERFACE;

	if (ppvObject != NULL)
	{
		*ppvObject = NULL;

		if (IID_IUnknown == riid)
			*ppvObject = static_cast<IUnknown*>(this);

		if (IID_IEnumString == riid)
			*ppvObject = static_cast<IEnumString*>(this);

		if (*ppvObject != NULL)
		{
			hr = S_OK;
			((LPUNKNOWN)*ppvObject)->AddRef();
		}

	}
	else
	{
		hr = E_POINTER;
	}

	return hr;

}


//
//	IEnumString implementation
//
STDMETHODIMP CCustomAutoComplete::Next(ULONG celt, LPOLESTR* rgelt, ULONG* pceltFetched)
{
	HRESULT hr = S_FALSE;

	if (!celt)
		celt = 1;

	ULONG i = 0;
	for (; i < celt; i++)
	{
		if (m_nCurrentElement == (ULONG)m_asList.GetSize())
			break;

		rgelt[i] = (LPWSTR)::CoTaskMemAlloc((ULONG) sizeof(WCHAR) * (m_asList[m_nCurrentElement].GetLength() + 1));
		lstrcpy(rgelt[i], m_asList[m_nCurrentElement]);

		if (pceltFetched)
			*pceltFetched++;

		m_nCurrentElement++;
	}

	if (i == celt)
		hr = S_OK;

	return hr;
}

STDMETHODIMP CCustomAutoComplete::Skip(ULONG celt)
{
	m_nCurrentElement += celt;

	if (m_nCurrentElement > (ULONG)m_asList.GetSize())
		m_nCurrentElement = 0;

	return S_OK;
}

STDMETHODIMP CCustomAutoComplete::Reset(void)
{
	m_nCurrentElement = 0;
	return S_OK;
}

STDMETHODIMP CCustomAutoComplete::Clone(IEnumString** ppenum)
{
	if (!ppenum)
		return E_POINTER;

	CCustomAutoComplete* pnew = new CCustomAutoComplete();

	pnew->AddRef();
	*ppenum = pnew;

	return S_OK;

}

// Internal implementation

void CCustomAutoComplete::InternalInit()
{
	m_nCurrentElement = 0;
	m_nRefCount = 0;
	m_fBound = FALSE;
}

HRESULT CCustomAutoComplete::EnDisable(BOOL p_fEnable)
{
	HRESULT hr = S_OK;

	ATLASSERT(m_pac);

	hr = m_pac->Enable(p_fEnable);

	if (SUCCEEDED(hr))
		m_fBound = p_fEnable;

	return hr;
}

