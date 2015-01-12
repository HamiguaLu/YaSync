// Connect.h : Declaration of the CConnect

#pragma once
#include "resource.h"       // main symbols
#include "AddIn.h"

#include "DllHelper.h"
#include <list>
using namespace std;

#import "C:\\Program Files\\Common Files\\Designer\\MSADDNDR.DLL" raw_interfaces_only, raw_native_types, no_namespace, named_guids 

// CConnect

HRESULT HrGetResource(int nId, LPCTSTR lpType,  LPVOID* ppvResourceData, DWORD* pdwSizeInBytes)
{
	HMODULE hModule = _AtlBaseModule.GetModuleInstance();
	if (!hModule)
		return E_UNEXPECTED;
	HRSRC hRsrc = FindResource(hModule, MAKEINTRESOURCE(nId), lpType);
	if (!hRsrc)
		return HRESULT_FROM_WIN32(GetLastError());
	HGLOBAL hGlobal = LoadResource(hModule, hRsrc);
	if (!hGlobal)
		return HRESULT_FROM_WIN32(GetLastError());
	*pdwSizeInBytes = SizeofResource(hModule, hRsrc);
	*ppvResourceData = LockResource(hGlobal);
	return S_OK;
}

BSTR GetXMLResource(int nId)
{
	LPVOID pResourceData = NULL;
	DWORD dwSizeInBytes = 0;
	HRESULT hr = HrGetResource(nId, TEXT("XML"), 
		&pResourceData, &dwSizeInBytes);
	if (FAILED(hr))
		return NULL;
	// Assumes that the data is not stored in Unicode.
	CComBSTR cbstr(dwSizeInBytes, reinterpret_cast<LPCSTR>(pResourceData));
	return cbstr.Detach();
}

SAFEARRAY* GetOFSResource(int nId)
{
	LPVOID pResourceData = NULL;
	DWORD dwSizeInBytes = 0;
	if (FAILED(HrGetResource(nId, TEXT("OFS"), 
		&pResourceData, &dwSizeInBytes)))
		return NULL;
	SAFEARRAY* psa;
	SAFEARRAYBOUND dim = {dwSizeInBytes, 0};
	psa = SafeArrayCreate(VT_UI1, 1, &dim);
	if (psa == NULL)
		return NULL;
	BYTE* pSafeArrayData;
	SafeArrayAccessData(psa, (void**)&pSafeArrayData);
	memcpy((void*)pSafeArrayData, pResourceData, dwSizeInBytes);
	SafeArrayUnaccessData(psa);
	return psa;
}

class ATL_NO_VTABLE CConnect : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CConnect, &CLSID_Connect>,
	public IDispatchImpl<AddInDesignerObjects::_IDTExtensibility2, &AddInDesignerObjects::IID__IDTExtensibility2, &AddInDesignerObjects::LIBID_AddInDesignerObjects, 1, 0>,
	public IDispatchImpl<IRibbonExtensibility, &__uuidof(IRibbonExtensibility), &LIBID_Office, /* wMajor = */ 2, /* wMinor = */ 4>,
	public IDispatchImpl<IButtonCallBack, &__uuidof(IButtonCallBack), &LIBID_AndroidAddInLib, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
	CConnect()
	{
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_ADDIN)
	DECLARE_NOT_AGGREGATABLE(CConnect)

	BEGIN_COM_MAP(CConnect)
		COM_INTERFACE_ENTRY2(IDispatch, IButtonCallBack)
		COM_INTERFACE_ENTRY(AddInDesignerObjects::IDTExtensibility2)
		COM_INTERFACE_ENTRY(IRibbonExtensibility)
		COM_INTERFACE_ENTRY(IButtonCallBack)
	END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease() 
	{
	}

public:
	//IDTExtensibility2 implementation:
	STDMETHOD(OnConnection)(IDispatch * Application, AddInDesignerObjects::ext_ConnectMode ConnectMode, IDispatch *AddInInst, SAFEARRAY **custom);
	STDMETHOD(OnDisconnection)(AddInDesignerObjects::ext_DisconnectMode RemoveMode, SAFEARRAY **custom );
	STDMETHOD(OnAddInsUpdate)(SAFEARRAY **custom );
	STDMETHOD(OnStartupComplete)(SAFEARRAY **custom );
	STDMETHOD(OnBeginShutdown)(SAFEARRAY **custom );

	CComPtr<IDispatch> m_pApplication;
	CComPtr<IDispatch> m_pAddInInstance;

	// IRibbonExtensibility Methods
public:
	STDMETHOD(GetCustomUI)(BSTR RibbonID, BSTR * RibbonXml)
	{
		if (!RibbonXml)
			return E_POINTER;

		if (_tcscmp(RibbonID, _T("Microsoft.Outlook.Explorer")))
		{
			return S_FALSE;
		}

		*RibbonXml = GetXMLResource(IDR_XML1);

		return (*RibbonXml ? S_OK : E_OUTOFMEMORY);  return S_OK;
	}

	// IButtonCallBack Methods
public:
	STDMETHOD(NewSmsBtnClicked)(IDispatch* RibbonControl);
	STDMETHOD(ReplySmsBtnClicked)(IDispatch* RibbonControl);
	STDMETHOD(StatBtnClicked)(IDispatch* RibbonControl);
	STDMETHOD(HelpBtnClicked)(IDispatch* btn);
	STDMETHOD(SyncBtnClicked)(IDispatch* RibbonControl);
	STDMETHOD(ThreadBtnClicked)(IDispatch* btn);
	STDMETHOD(SaveContactBtnClicked)(IDispatch* RibbonControl);
	STDMETHOD(FilterBtnClicked)(IDispatch* btn);
	STDMETHOD(DeleteBtnClicked)(IDispatch* btn);
	STDMETHOD(FileBtnClicked)(IDispatch* btn);
	STDMETHOD(BuyBtnClicked)(IDispatch* btn);
	

public:
	int getFirstSelectedItemID(TCHAR *szEntryID);
	list<TCHAR*> getSelectedItemID();
	
};

OBJECT_ENTRY_AUTO(__uuidof(Connect), CConnect)
