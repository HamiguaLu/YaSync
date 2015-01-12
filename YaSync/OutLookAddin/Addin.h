// Addin.h : Declaration of the CSMSAddIn

#ifndef __ADDIN_H_
#define __ADDIN_H_

#include "resource.h"       // main symbols

#include <list>
using namespace std;

#import "C:\\Program Files\\Common Files\\Designer\\MSADDNDR.DLL" raw_interfaces_only, raw_native_types, no_namespace, named_guids 
/////////////////////////////////////////////////////////////////////////////
// CSMSAddIn
extern _ATL_FUNC_INFO OnClickBTN;
extern _ATL_FUNC_INFO OnNewExplorer;
extern _ATL_FUNC_INFO OnExplorerActivateOrClose;
extern _ATL_FUNC_INFO OnAppStart;

class ATL_NO_VTABLE CSMSAddIn : 
	public IDispEventSimpleImpl<1,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<2,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<3,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<4,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<5,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<6,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<7,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<8,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<9,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<10,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<11,CSMSAddIn,&__uuidof(Office::_CommandBarButtonEvents)>,

	public IDispEventSimpleImpl<15,CSMSAddIn,&__uuidof(Outlook::ExplorersEvents)>,
	public IDispEventSimpleImpl<16,CSMSAddIn,&__uuidof(Outlook::ExplorerEvents)>,

	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSMSAddIn, &CLSID_Addin>,
	public IDispatchImpl<IAddin, &IID_IAddin, &LIBID_MYADDINLib>,
	public IDispatchImpl<_IDTExtensibility2, &IID__IDTExtensibility2, &LIBID_AddInDesignerObjects>
{
public:
	CSMSAddIn()
	{
//		m_iExplorerCount = 0;
	}

	DECLARE_REGISTRY_RESOURCEID(IDR_ADDIN)

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CSMSAddIn)
		COM_INTERFACE_ENTRY(IAddin)
		//DEL 	COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY2(IDispatch, IAddin)
		COM_INTERFACE_ENTRY(_IDTExtensibility2)
	END_COM_MAP()

	BEGIN_SINK_MAP(CSMSAddIn)
		SINK_ENTRY_INFO(1,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNNewSMS,			&OnClickBTN)
		SINK_ENTRY_INFO(2,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNReplySMS,		&OnClickBTN)
		SINK_ENTRY_INFO(3,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNThread,			&OnClickBTN)
		SINK_ENTRY_INFO(4,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNDelete,			&OnClickBTN)
		SINK_ENTRY_INFO(5,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNSave2Contact,	&OnClickBTN)
		SINK_ENTRY_INFO(6,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNFilter,			&OnClickBTN)
		SINK_ENTRY_INFO(7,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNHelp,			&OnClickBTN)		
		SINK_ENTRY_INFO(8,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNBuy,			&OnClickBTN)
		SINK_ENTRY_INFO(9,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNRestore,		&OnClickBTN)
		SINK_ENTRY_INFO(10,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNFile,		&OnClickBTN)
		SINK_ENTRY_INFO(11,	__uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01,OnClickBTNStat,		&OnClickBTN)
		
		SINK_ENTRY_INFO(15,	__uuidof(Outlook::ExplorersEvents),/*dispid*/ 0x0000F001, OnNewExplorerEvent,		&OnNewExplorer)
		SINK_ENTRY_INFO(16,	__uuidof(Outlook::ExplorerEvents),/*dispid*/ 0x0000F008, OnExplorerCloseEvent,		&OnExplorerActivateOrClose)
	END_SINK_MAP()

public:
	// _IDTExtensibility2
	STDMETHODIMP OnConnection(IDispatch * Application, ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY * * custom);

	STDMETHODIMP OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom);

	STDMETHODIMP OnAddInsUpdate(SAFEARRAY * * custom);

	STDMETHODIMP OnStartupComplete(SAFEARRAY * * custom);

	STDMETHODIMP OnBeginShutdown(SAFEARRAY * * custom);

	HRESULT SetIconForBtn(int iPicID,CComPtr<Office::_CommandBarButton>  spBTN);
	
	HRESULT SetItemDeleteEvt();
	HRESULT CreateButton(CComPtr<Outlook::_Explorer> spExplorer,bool bUseBigIcon);

	_CommandBarButtonPtr CreateOneButton(TCHAR* tszCpation,TCHAR *tszTip,TCHAR *tszTag,CommandBarControlPtr spNewBar);

	int GetExpCount(TCHAR *sTip);

	CComPtr<Office::_CommandBarButton> m_spBTNNewSMS;
	CComPtr<Office::_CommandBarButton> m_spBTNReplySMS;
	CComPtr<Office::_CommandBarButton> m_spBTNThread;
	CComPtr<Office::_CommandBarButton> m_spBTNDelete;
	CComPtr<Office::_CommandBarButton> m_spBTNSave2Contact;
	CComPtr<Office::_CommandBarButton> m_spBTNFilter;
	CComPtr<Office::_CommandBarButton> m_spBTNBuy;
	CComPtr<Office::_CommandBarButton> m_spBTNHelp;
	CComPtr<Office::_CommandBarButton> m_spBTNRestore;
	CComPtr<Office::_CommandBarButton> m_spBTNFile;
	CComPtr<Office::_CommandBarButton> m_spBTNStat;

	//_Application
	VOID __stdcall OnClickBTNNewSMS(IDispatch * Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNThread(IDispatch * Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNDelete(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNExport(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNBuy(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNHelp(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);

	VOID __stdcall OnClickBTNReplySMS(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNSave2Contact(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNFilter(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNRestore(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNFile(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	VOID __stdcall OnClickBTNStat(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault);
	
	VOID __stdcall OnNewExplorerEvent(IDispatch* pdispItemsEvents);
	VOID __stdcall OnExplorerCloseEvent();

	DWORD GetContactInfo(TCHAR *szName,TCHAR *szPhone);
	
	int	 m_iOLVer;
	BOOL m_bIsBtnCreated;
	BOOL m_bIsAutomation;
	BOOL m_bIsSmsFolder;
	DWORD m_dwFolderType;

	HRESULT SetSmsItemEvt(MAPIFolderPtr& smsFolder);
	HRESULT CreateSubContactFolder(MAPIFolderPtr& contactFolder);
	HRESULT RemoveDuplicateContact(MAPIFolderPtr contactFolder);
	list<TCHAR*> getSelectedItemID();
	VOID DeAttachBtnEvent();
	int getFirstSelectedItemID(TCHAR *szEntryID);
	//HRESULT SetItemAddEvt(void);
};

typedef IDispEventSimpleImpl<1,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTNewSMS;
typedef IDispEventSimpleImpl<2,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTReplySMS;
typedef IDispEventSimpleImpl<3,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTThread;
typedef IDispEventSimpleImpl<4,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTDelete;
typedef IDispEventSimpleImpl<5,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTSave2Contact;
typedef IDispEventSimpleImpl<6,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTFilter;
typedef IDispEventSimpleImpl<7,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTHelp;
typedef IDispEventSimpleImpl<8,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTBuy;
typedef IDispEventSimpleImpl<9,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTRestore;
typedef IDispEventSimpleImpl<10,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTFile;
typedef IDispEventSimpleImpl<11,CSMSAddIn, &__uuidof(Office::_CommandBarButtonEvents)> CommandBtnEvTStat;


typedef IDispEventSimpleImpl<15,CSMSAddIn, &__uuidof(Outlook::ExplorersEvents)> ExplorerNewEvt;
typedef IDispEventSimpleImpl<16,CSMSAddIn, &__uuidof(Outlook::ExplorerEvents)> ExplorerCloseEvt;

//typedef IDispEventSimpleImpl<11,CSMSAddIn, &__uuidof(Outlook::ExplorerEvents)> ExplorerActivateEvt;
#endif //__ADDIN_H_
