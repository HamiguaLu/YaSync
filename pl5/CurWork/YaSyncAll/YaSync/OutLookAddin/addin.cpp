// Addin.cpp : Implementation of CSMSAddIn
#include "stdafx.h"
#include "MyAddin.h"
#include "Addin.h"
#include "DllHelper.h"
#include <windows.h>
#include <windowsx.h>
//#include "GetContactName.h"
//#include "PEProtocol.h"



/////////////////////////////////////////////////////////////////////////////
// CSMSAddIn

_ATL_FUNC_INFO OnClickBTN						=			{CC_STDCALL,VT_EMPTY,2,{VT_DISPATCH,VT_BYREF | VT_BOOL}};

_ATL_FUNC_INFO OnNewExplorer					=			{CC_STDCALL, VT_EMPTY, 1, {VT_DISPATCH} };
_ATL_FUNC_INFO OnExplorerActivateOrClose		=			{CC_STDCALL,VT_EMPTY,0};
_ATL_FUNC_INFO OnAppStart						=			{CC_STDCALL,VT_EMPTY,0};




HBITMAP CreateBmpMask(ULONG bmpID)
{
	// The bitmap should be stored as a resource in the exe file.
	// We pass the hInstance of the application, and the ID of the
	// bitmap to the LoadBitmap API function and it returns us an
	// HBITMAP to a DDB created from the resource data.
	HBITMAP hbmImage;
	HBITMAP hbmMask;
	HINSTANCE hInstance = _Module.GetResourceInstance();
	hbmImage = LoadBitmap(hInstance,MAKEINTRESOURCE(bmpID));

	BITMAP bm;
	GetObject(hbmImage,sizeof(bm),&bm);  

	// Now, we need to "correct" the image bitmap, and create a
	// monochrome bitmap. Step 1 - we create a monochrome bitmap
	// the same size as the original. then we trick GDI into
	// blitting the Image onto the mask in such a way as the
	// mask is created correctly.
	hbmMask = CreateBitmap(bm.bmWidth,bm.bmHeight,1,1,NULL);

	// We will need two DCs to work with. One to hold the Image
	// (the source), and one to hold the mask (destination).
	// When blitting onto a monochrome bitmap from a color, pixels
	// in the source color bitmap that are equal to the background
	// color are blitted as white. All the remaining pixels are
	// blitted as black.

	HDC hdcSrc = CreateCompatibleDC(NULL);
	HDC hdcDst = CreateCompatibleDC(NULL);

	HBITMAP hbmSrcT = SelectBitmap(hdcSrc,hbmImage);
	HBITMAP hbmDstT = SelectBitmap(hdcDst,hbmMask);

	COLORREF clrTopLeft = GetPixel(hdcSrc,0,0);
	COLORREF clrSaveBk = SetBkColor(hdcSrc,clrTopLeft);

	// This call sets up the mask bitmap.
	BitBlt(hdcDst,0,0,bm.bmWidth,bm.bmHeight,hdcSrc,0,0,SRCCOPY);

	// Now, we need to paint onto the original image, making
	// sure that the "transparent" area is set to black. What
	// we do is AND the monochrome image onto the color Image
	// first. When blitting from mono to color, the monochrome
	// pixel is first transformed as follows:
	// if it 1 (black) it is mapped to the color set by SetTextColor().
	// if is 0 (white) is is mapped to the color set by SetBkColor().
	// Only then is the raster operation performed.

	COLORREF clrSaveDstText = SetTextColor(hdcSrc,RGB(255,255,255));
	SetBkColor(hdcSrc,RGB(0,0,0));

	BitBlt(hdcSrc,0,0,bm.bmWidth,bm.bmHeight,hdcDst,0,0,SRCAND);

	// Clean up by deselecting any objects, and delete the 
	// DC's.
	SetTextColor(hdcDst,clrSaveDstText);

	SetBkColor(hdcSrc,clrSaveBk);
	SelectBitmap(hdcSrc,hbmSrcT);
	SelectBitmap(hdcDst,hbmDstT);

	DeleteDC(hdcSrc);
	DeleteDC(hdcDst);

	return hbmMask;
}


STDMETHODIMP CSMSAddIn ::OnConnection(IDispatch * Application, ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY * * custom)
{
	CComQIPtr <Outlook::_Application> spApp(Application); 
	CComPtr<Office::_CommandBars> spCmdBars; 
	CComPtr<Office::CommandBar> spCmdBar;
	if (spApp == NULL)
	{
		return E_FAIL;
	}

	BSTR bstrVersionCode = NULL;
	spApp->get_Version(&bstrVersionCode);
	TCHAR *tszInfo = bstrVersionCode;

	TCHAR tszMainVer[10] = {0};
	_tcsncpy(tszMainVer,tszInfo,2);

	m_iOLVer = PE_OL_VER_UNKONW;
	if (_tcsstr(tszMainVer,_T("15") ))
	{
		m_iOLVer = PE_OL_VER_2013;
	}
	if (_tcsstr(tszMainVer,_T("14") ))
	{
		m_iOLVer = PE_OL_VER_2010;
	}
	else if (_tcsstr(tszMainVer,_T("12") ))
	{
		m_iOLVer = PE_OL_VER_2007;
	}
	else if (_tcsstr(tszMainVer,_T("11") ))
	{
		m_iOLVer = PE_OL_VER_2003;
	}

	WriteLog(_T("Mobitnt add in start"));
	WriteLog(_T("Outlook ver:"));
	WriteLog(tszInfo);
	SysFreeString(bstrVersionCode);

	m_bIsAutomation = FALSE;
	m_bIsBtnCreated = FALSE;

	CComPtr<Outlook::_Explorers> m_Explorers;
	HRESULT hr = spApp->get_Explorers(&m_Explorers);
	if ( FAILED(hr))
	{
		WriteLog(_T("OnConnection:Explorers is new"));
		return hr;
	}

	hr = ExplorerNewEvt::DispEventAdvise( (IDispatch*)m_Explorers, &__uuidof(Outlook::ExplorersEvents) );
	if (FAILED(hr))
	{
		WriteLog(_T("OnConnection:Set new event filed"));
		return S_FALSE;
	}

	//get the ActiveExplorer Object
	CComPtr<Outlook::_Explorer> spExplorer;
	hr = spApp->ActiveExplorer(&spExplorer);
	if (FAILED(hr) || spExplorer == NULL)
	{
		//seems automation
		m_bIsAutomation = TRUE;
		WriteLog(_T("OnConnection:Automation start"));
		return S_OK;
	}

	ExplorerCloseEvt::DispEventAdvise((IDispatch*)spExplorer, &__uuidof(Outlook::ExplorerEvents));
	bool bUseBigIcon = false;
	if (m_iOLVer >= PE_OL_VER_2010 )
	{
		bUseBigIcon = true;
	}

	hr = CreateButton(spExplorer,bUseBigIcon);
	if ( hr != S_OK )
	{
		WriteLog(_T("OnConnection:Create button failed"));
		return hr;
	}

	return S_OK;
}

int CSMSAddIn::getFirstSelectedItemID(TCHAR *szEntryID)
{
	list<TCHAR*> idlist = getSelectedItemID();
	if (idlist.size() <= 0)
	{
		WriteLog(_T("getFirstSelectedItemID failed"));
		return PE_RET_FAIL;
	}

	list<TCHAR*>::iterator i = idlist.begin();
	_tcscpy(szEntryID,(TCHAR*)*i);

	for (i = idlist.begin(); i != idlist.end(); ++i)
	{ 
		TCHAR *pEntryID = (TCHAR*)*i;
		delete[] pEntryID;
	}
	idlist.clear();
	return PE_RET_OK;
}

list<TCHAR*> CSMSAddIn::getSelectedItemID()
{
	CComPtr <Outlook::_Explorer>  olExp;
	CComPtr <Outlook::Selection> olSel;
	LPDISPATCH olSelectedItem;
	BOOL bFound = FALSE;
	_ApplicationPtr pApp;
	HRESULT hr=pApp.CreateInstance(__uuidof(Application));
	list<TCHAR*> idList;
	if (FAILED(hr))
	{
		WriteLog(_T("Get app failed"));
		return idList;
	}

	// Get the ActiveExplorer so that you can get the selection;
	//CComQIPtr <Outlook::_Application> spApp(Application); 
	pApp->ActiveExplorer(&olExp);
	olExp->get_Selection(&olSel);

	long iCount = 0;
	olSel->get_Count(&iCount);

	for (long i = 1; i <= iCount; i++)
	{
		VARIANT covIndex;
		VariantInit(&covIndex);
		covIndex.vt = VT_I4;
		covIndex.lVal = i;

		olSel->Item(covIndex,&olSelectedItem);	// Get the selected item
		CComPtr <Outlook::_MailItem>   olMailItem;
		CComPtr <Outlook::_JournalItem>   olJournalItem;
		CComPtr <Outlook::_ContactItem>   olContactItem;
		olMailItem = (_MailItemPtr) olSelectedItem;
		olJournalItem = (_JournalItemPtr) olSelectedItem;
		olContactItem = (_ContactItemPtr) olSelectedItem;
		if (olMailItem == NULL && olJournalItem == NULL && olContactItem == NULL)
		{
			continue;
		}

		TCHAR *tTmp = new TCHAR[1024];
		memset(tTmp,0,1024*sizeof(TCHAR));
		BSTR bstrItemEntryID = NULL;

		if (olMailItem != NULL)
		{
			olMailItem->get_EntryID(&bstrItemEntryID);
		}
		else if (olJournalItem != NULL)
		{
			olJournalItem->get_EntryID(&bstrItemEntryID);
		}
		else if (olContactItem != NULL)
		{
			olContactItem->get_EntryID(&bstrItemEntryID);
		}

		BSTR bstrFolderEntryID = NULL;
		MAPIFolderPtr pFolder;
		if (olMailItem != NULL)
		{
			hr = olMailItem->get_Parent((IDispatch * *)&pFolder);
		}
		else if (olJournalItem != NULL)
		{
			hr = olJournalItem->get_Parent((IDispatch * *)&pFolder);
		}
		else if (olContactItem != NULL)
		{
			hr = olContactItem->get_Parent((IDispatch * *)&pFolder);
		}

		if (FAILED(hr) || pFolder == NULL)
		{
			SysFreeString(bstrItemEntryID);
			delete[] tTmp;
			continue;
		}

		pFolder->get_EntryID(&bstrFolderEntryID);
		if (_tcslen(bstrItemEntryID) < 1)
		{
			WriteLog(_T("getSelectedItemID:bstrItemEntryID is null"));
			delete tTmp;
		}
		else if (_tcslen(bstrFolderEntryID) < 1)
		{
			WriteLog(_T("getSelectedItemID:bstrFolderEntryID is null"));
			delete tTmp;
		}
		else
		{
			_stprintf(tTmp,_T("%s;%s"),bstrItemEntryID,bstrFolderEntryID);
			idList.push_back(tTmp);
		}

		SysFreeString(bstrItemEntryID);
		SysFreeString(bstrFolderEntryID);
	}

	return idList;
}

int CSMSAddIn::GetExpCount(TCHAR *sTip)
{
	CComPtr<Outlook::_Explorers> ExplorersSet;
	_ApplicationPtr pApp;
	HRESULT hr = pApp.CreateInstance(__uuidof(Application));
	if ( FAILED(hr))
	{
		WriteLog(_T("OnNewExplorerEvent:get app failed"));
		return -1;
	}

	hr = pApp->get_Explorers(&ExplorersSet);
	if ( FAILED(hr))
	{
		WriteLog(_T("OnNewExplorerEvent:exp is new"));
		return -1;
	}

	long lCount = 0;

	hr = ExplorersSet->get_Count(&lCount);
	if (FAILED(hr))
	{
		WriteLog(_T("OnNewExplorerEvent:get count failed"));
		return -1;
	}

	TCHAR sInfo[255];
	_stprintf(sInfo,_T("%s:Exp Count = %d"),sTip,lCount);
	WriteLog(sInfo);
	pApp = NULL;
	return lCount;
}

VOID __stdcall CSMSAddIn::OnNewExplorerEvent(IDispatch* pdispItemsEvents)
{
	int iCount = 0;
	iCount = GetExpCount(_T("OnNewExplorerEvent"));
	if (iCount < 0)
	{
		WriteLog(_T("OnNewExplorerEvent:GetExpCount failed"));
		return;
	}

	TCHAR sInfo[255];
	_stprintf(sInfo,_T("OnNewExplorerEvent:Count = %d"),iCount);
	WriteLog(sInfo);

	//if (m_bIsBtnCreated)
	if (iCount > 1)
	{
		WriteLog(_T("New explorer:no need to handle"));
		return;
	}

	if (m_bIsBtnCreated)
	{
		WriteLog(_T("OnNewExplorerEvent:Another instance already exist?Btn already created"));
		return;

	}

	WriteLog(_T("New explorer event fired"));

	//_ExplorerPtr(pdispItemsEvents)->Activate();

	CComPtr<Outlook::_Explorer> spExplorer = (_ExplorerPtr)pdispItemsEvents;
	HRESULT hr = spExplorer->Activate();
	if (FAILED(hr))
	{
		WriteLog(_T("New explorer:failed to active the exp"));
	}

	m_bIsBtnCreated = FALSE;
	bool bUseBigIcon = false;
	if (m_iOLVer >= PE_OL_VER_2010 )
	{
		bUseBigIcon = true;
	}

	hr = CreateButton(spExplorer,bUseBigIcon);
	if (hr != S_OK )
	{
		WriteLog(_T("OnExplorerActivateEvent:CreateButton failed"));
	}

	//ExplorerActivateEvt::DispEventAdvise((IDispatch*)m_spExplorer, &__uuidof(Outlook::ExplorerEvents));
	ExplorerCloseEvt::DispEventAdvise((IDispatch*)pdispItemsEvents, &__uuidof(Outlook::ExplorerEvents));

}

VOID CSMSAddIn::DeAttachBtnEvent()
{

	CommandBtnEvTNewSMS::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTReplySMS::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTThread::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTDelete::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTSave2Contact::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTFilter::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTBuy::DispEventUnadvise((IDispatch*)NULL);
	CommandBtnEvTHelp::DispEventUnadvise((IDispatch*)NULL);
}

VOID __stdcall CSMSAddIn::OnExplorerCloseEvent()
{
	GetExpCount(_T("OnExplorerCloseEvent"));

	WriteLog(_T("OnExplorerCloseEvent:m_spExplorer close"));

	//_ExplorerPtr m_spExplorer;
	ExplorerCloseEvt::DispEventUnadvise((IDispatch*)NULL);

	DeAttachBtnEvent();

	m_bIsBtnCreated = FALSE;
}

STDMETHODIMP CSMSAddIn::OnDisconnection(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
{
	if (m_bIsBtnCreated)
	{
		WriteLog(_T("OnDisconnection:DispEventUnadvise"));
		DeAttachBtnEvent();
		m_bIsBtnCreated = FALSE;
	}

	WriteLog(_T("OnDisconnection:stop device..."));

	WriteLog(_T("Mobitnt add in exit "));
	return S_OK;
}

STDMETHODIMP CSMSAddIn ::OnAddInsUpdate(SAFEARRAY * * custom)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSMSAddIn ::OnStartupComplete(SAFEARRAY * * custom)
{
	return E_NOTIMPL;
}

STDMETHODIMP CSMSAddIn ::OnBeginShutdown(SAFEARRAY * * custom)
{
	WriteLog(_T("Outlook going to shutdown "));
	return E_NOTIMPL;
}


_CommandBarButtonPtr CSMSAddIn ::CreateOneButton(TCHAR* tszCpation,TCHAR *tszTip,TCHAR *tszTag,CommandBarControlPtr spNewBar)
{
	_CommandBarButtonPtr spCmdButton(spNewBar);
	if (spCmdButton == NULL)
	{
		return (_CommandBarButtonPtr)NULL;
	}

	//assign the properties
	spCmdButton->PutVisible(VARIANT_TRUE); 
	spCmdButton->PutCaption(tszCpation); 
	spCmdButton->PutEnabled(VARIANT_TRUE);
	spCmdButton->PutTooltipText(tszTip); 
	spCmdButton->PutTag(tszTag); 
	return spCmdButton;
}

#define ADD_IN_BTN_TAG		_T("Btn Tag")

HRESULT CSMSAddIn ::CreateButton(CComPtr<Outlook::_Explorer> spExplorer,bool bUseBigIcon)
{
	if (m_bIsBtnCreated)
	{
		WriteLog(_T("CreateButton:Already created"));
		return S_OK;
	}

	HRESULT hr = E_FAIL;

	CComPtr<Office::_CommandBars> m_spCmdBars; 
	CComPtr<Office::CommandBar> m_spCmdBar;
	CComPtr <Office::CommandBar> m_spNewCmdBar;
	CComPtr < Office::CommandBarControls> m_spBarControls;

	hr = spExplorer->get_CommandBars(&m_spCmdBars);
	if(FAILED(hr) || m_spCmdBars == NULL)
	{
		WriteLog(_T("CreateButton:cmdbar is nul"));
		return hr;
	}

	CComVariant vName("MobiTNT PocketExport Toolbar");
	CComVariant vPos(1); 
	CComVariant vTemp(VARIANT_TRUE); 		
	CComVariant vEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);	

	//create New sms button
	//OnDisconnection
	////////////////////////////////////////////////////////////
	m_spNewCmdBar = m_spCmdBars->Add(vName, vPos, vEmpty, vTemp);
	if (m_spNewCmdBar == NULL)
	{
		WriteLog(_T("CreateButton:m_spNewCmdBar is nul"));
		return E_FAIL;
	}

	m_spBarControls = m_spNewCmdBar->GetControls();
	if (m_spBarControls == NULL)
	{
		WriteLog(_T("CreateButton:m_spBarControls is nul"));
		return E_FAIL;
	}

	CComVariant vToolBarType(1);
	CComVariant vShow(VARIANT_TRUE);
	CommandBarControlPtr spNewBar; 

	//////////////////////////////////////////////////////
	//create new sms button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	if (spNewBar == NULL)
	{
		WriteLog(_T("CreateButton:spNewBar is nul"));
		return E_FAIL;
	}

	m_spBTNNewSMS = CreateOneButton(_T("New SMS"),_T("Send SMS from Outlook"),_T("MobiTNT-Tag1"),spNewBar);
	hr = CommandBtnEvTNewSMS::DispEventAdvise((IDispatch*)m_spBTNNewSMS);
	if(FAILED(hr))
	{
		WriteLog(_T("CreateButton:DispEventAdvise failed"));
		return hr;
	}

	//////////////////////////////////////////////////////
	//create new sms button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	if (spNewBar == NULL)
	{
		WriteLog(_T("CreateButton:spNewBar is nul"));
		return E_FAIL;
	}


	m_spBTNReplySMS = CreateOneButton(_T("Reply SMS"),_T("Reply SMS from Outlook"),_T("MobiTNT-Tag-reply"),spNewBar);
	hr = CommandBtnEvTReplySMS::DispEventAdvise((IDispatch*)m_spBTNReplySMS);
	if(FAILED(hr))
	{
		WriteLog(_T("CreateButton:DispEventAdvise failed"));
		return hr;
	}

#if 1
	/////////////////////////////////////////////////////////////////////////////////
	//create thread sms button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	//ATLASSERT(spNewBar);
	if (spNewBar == NULL)
	{
		return E_FAIL;
	}

	m_spBTNThread = CreateOneButton(_T("SMS Thread"),_T("Read SMS by Thread"),_T("MobiTNT-TagSMSTHREAD"),spNewBar);
	//CComPtr<Office::_CommandBarButton> btnSetting = CreateOneButton("Settings","Settings for Mobitnt Outlook Add in","MobiTNT-Tag10",spNewBar);
	hr = CommandBtnEvTThread::DispEventAdvise((IDispatch*)m_spBTNThread);
	if(FAILED(hr))
	{
		return hr;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//create delete button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	//ATLASSERT(spNewBar);
	if (spNewBar == NULL)
	{
		return E_FAIL;
	}

	m_spBTNDelete = CreateOneButton(_T("Delete"),_T("Delete items both from outlook and phone"),_T("MobiTNT-TagDelete"),spNewBar);
	//CComPtr<Office::_CommandBarButton> btnSetting = CreateOneButton("Settings","Settings for Mobitnt Outlook Add in","MobiTNT-Tag10",spNewBar);
	hr = CommandBtnEvTDelete::DispEventAdvise((IDispatch*)m_spBTNDelete);
	if(FAILED(hr))
	{
		return hr;
	}
#endif

	//////////////////////////////////////////////////////
	//create save contact button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	if (spNewBar == NULL)
	{
		WriteLog(_T("CreateButton:spNewBar is nul"));
		return E_FAIL;
	}


	m_spBTNSave2Contact = CreateOneButton(_T("Add to contact"),_T("Add selected item's phone to contact"),_T("MobiTNT-Tag-Addcontact"),spNewBar);
	hr = CommandBtnEvTSave2Contact::DispEventAdvise((IDispatch*)m_spBTNSave2Contact);
	if(FAILED(hr))
	{
		WriteLog(_T("CreateButton:DispEventAdvise failed"));
		return hr;
	}

	//////////////////////////////////////////////////////
	//create filter button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	if (spNewBar == NULL)
	{
		WriteLog(_T("CreateButton:spNewBar is nul"));
		return E_FAIL;
	}


	m_spBTNFilter = CreateOneButton(_T("Add to blackList"),_T("black list when sync"),_T("MobiTNT-Tag-filter"),spNewBar);
	hr = CommandBtnEvTFilter::DispEventAdvise((IDispatch*)m_spBTNFilter);
	if(FAILED(hr))
	{
		WriteLog(_T("CreateButton:DispEventAdvise failed"));
		return hr;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//create restore button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	//ATLASSERT(spNewBar);
	if (spNewBar == NULL)
	{
		return E_FAIL;
	}

	m_spBTNRestore = CreateOneButton(_T("Restore"),_T("Restore data to device"),_T("MobiTNT-TagRestore"),spNewBar);
	//CComPtr<Office::_CommandBarButton> btnSetting = CreateOneButton("Settings","Settings for Mobitnt Outlook Add in","MobiTNT-Tag10",spNewBar);
	hr = CommandBtnEvTRestore::DispEventAdvise((IDispatch*)m_spBTNRestore);
	if(FAILED(hr))
	{
		return hr;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//create File button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	//ATLASSERT(spNewBar);
	if (spNewBar == NULL)
	{
		return E_FAIL;
	}

	m_spBTNFile = CreateOneButton(_T("File Transfer"),_T("Transfer file to android"),_T("MobiTNT-TagFile"),spNewBar);
	//CComPtr<Office::_CommandBarButton> btnSetting = CreateOneButton("Settings","Settings for Mobitnt Outlook Add in","MobiTNT-Tag10",spNewBar);
	hr = CommandBtnEvTFile::DispEventAdvise((IDispatch*)m_spBTNFile);
	if(FAILED(hr))
	{
		return hr;
	}


	/////////////////////////////////////////////////////////////////////////////////
	//create STAT button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	//ATLASSERT(spNewBar);
	if (spNewBar == NULL)
	{
		return E_FAIL;
	}

	m_spBTNStat = CreateOneButton(_T("Stat"),_T("SMS/Call Log Stat"),_T("MobiTNT-TagStat"),spNewBar);
	//CComPtr<Office::_CommandBarButton> btnSetting = CreateOneButton("Settings","Settings for Mobitnt Outlook Add in","MobiTNT-Tag10",spNewBar);
	hr = CommandBtnEvTStat::DispEventAdvise((IDispatch*)m_spBTNStat);
	if(FAILED(hr))
	{
		return hr;
	}

	/////////////////////////////////////////////////////////////////////////////////
	//create Help button
	spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
	//ATLASSERT(spNewBar);
	if (spNewBar == NULL)
	{
		return E_FAIL;
	}

	m_spBTNHelp = CreateOneButton(_T("Help"),_T("PocketExport user guide"),_T("MobiTNT-TagHelp"),spNewBar);
	//CComPtr<Office::_CommandBarButton> btnTools = CreateOneButton("Tools","Tools used to manage your device","MobiTNT-Tag9",spNewBar);
	hr = CommandBtnEvTHelp::DispEventAdvise((IDispatch*)m_spBTNHelp);
	if(FAILED(hr))
	{
		return hr;
	}


	if (PEIsRegistered() != PE_RET_OK)
	{
		/////////////////////////////////////////////////////////////////////////////////
		//create buy button
		spNewBar = m_spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
		//ATLASSERT(spNewBar);
		if (spNewBar == NULL)
		{
			return E_FAIL;
		}

		m_spBTNBuy = CreateOneButton(_T("Buy Now"),_T("YaSync needs your support"),_T("MobiTNT-Tag-Buy"),spNewBar);
		//CComPtr<Office::_CommandBarButton> btnSyncSim = CreateOneButton("Sync Sim","Sync Sim card contact to outlook","MobiTNT-Tag8",spNewBar);
		hr = CommandBtnEvTBuy::DispEventAdvise((IDispatch*)m_spBTNBuy);
		if(FAILED(hr))
		{
			return hr;
		}

		SetIconForBtn(IDB_BUY,m_spBTNBuy);
	}

	m_spNewCmdBar->PutVisible(VARIANT_TRUE);


	SetIconForBtn(IDB_NEW_SMS,m_spBTNNewSMS);
	SetIconForBtn(IDB_REPLY,m_spBTNReplySMS);
	SetIconForBtn(IDB_SMS_THREAD,m_spBTNThread);
	SetIconForBtn(IDB_DELETE,m_spBTNDelete);
	SetIconForBtn(IDB_CONTACT,m_spBTNSave2Contact);
	SetIconForBtn(IDB_FILTER,m_spBTNFilter);
	SetIconForBtn(IDB_RESTORE,m_spBTNRestore);
	SetIconForBtn(IDB_FILE,m_spBTNFile);
	SetIconForBtn(IDB_STAT,m_spBTNStat);
	SetIconForBtn(IDB_HELP,m_spBTNHelp);

	m_bIsBtnCreated = TRUE;

	WriteLog(_T("CreateButton:everything is done"));

	return S_OK;
}

HRESULT CSMSAddIn ::SetIconForBtn(int iPicID,CComPtr<Office::_CommandBarButton> spBTN)
{
	//HBITMAP hBmp = LoadBitmap(_Module.GetResourceInstance(),MAKEINTRESOURCE(iPicID));
	HANDLE hBmp		= LoadImage(_Module.GetResourceInstance(),MAKEINTRESOURCE(iPicID),IMAGE_BITMAP,0,0,0);

	HRESULT hr;
	CComPtr<IPicture> pPicture;
	PICTDESC pd;
	ZeroMemory(&pd,sizeof pd);
	pd.cbSizeofstruct = sizeof pd;
	pd.picType = PICTYPE_BITMAP;
	pd.bmp.hbitmap = (HBITMAP)hBmp;
	pd.bmp.hpal = 0;
	hr = OleCreatePictureIndirect( &pd,
		__uuidof(pPicture),
		FALSE,
		(LPVOID *)&pPicture );
	if (FAILED(hr))
		return hr;
	OLE_XSIZE_HIMETRIC nWidth = 0;
	OLE_YSIZE_HIMETRIC nHeight = 0;
	hr = pPicture->get_Width(&nWidth);
	hr = pPicture->get_Height(&nHeight);

	CComPtr<IPictureDisp> pPictureDisp;
	hr = pPicture->QueryInterface(&pPictureDisp);
	if (FAILED(hr))
		return hr;

	HANDLE hBmpMask = CreateBmpMask(iPicID);

	CComPtr<IPicture> pPictureMask;
	PICTDESC pdMask;
	ZeroMemory(&pdMask,sizeof(pdMask));
	pdMask.cbSizeofstruct = sizeof(pdMask);
	pdMask.picType = PICTYPE_BITMAP;
	pdMask.bmp.hbitmap = (HBITMAP)hBmpMask;
	pdMask.bmp.hpal = 0;
	hr = OleCreatePictureIndirect( &pdMask,	__uuidof(pPicture),	FALSE,(LPVOID *)&pPictureMask);
	if (FAILED(hr))
		return hr;
	nWidth = 0;
	nHeight = 0;
	hr = pPictureMask->get_Width(&nWidth);
	hr = pPictureMask->get_Height(&nHeight);

	CComPtr<IPictureDisp> pPictureDispMask;
	hr = pPictureMask->QueryInterface(&pPictureDispMask);
	if (FAILED(hr))
		return hr;

	spBTN->PutStyle(Office::msoButtonIconAndCaption);
	hr = spBTN->put_Picture(pPictureDisp);
	spBTN->put_Mask(pPictureDispMask);

	return hr;
}

VOID __stdcall CSMSAddIn::OnClickBTNNewSMS(IDispatch* /*Office::_CommandBarButton* */ Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	SendCmd2YaSync(PE_CMD_NEW_SMS,0,0);
}

VOID __stdcall CSMSAddIn::OnClickBTNThread(IDispatch* /*Office::_CommandBarButton* */ Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_THREAD,szEntry,sizeof(szEntry));
		return;
	}

	MessageBox(NULL,_T("Please select a SMS item first"),_T("Error"),MB_ICONSTOP|MB_OK);
}

VOID __stdcall CSMSAddIn::OnClickBTNDelete(IDispatch* /*Office::_CommandBarButton* */ Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	list<TCHAR*> idlist = getSelectedItemID();
	if (idlist.size() <= 0)
	{
		MessageBox(NULL,_T("Please select a SMS/Call Log/Contact item first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	list<TCHAR*>::iterator i;
	for (i = idlist.begin(); i != idlist.end(); ++i)
	{ 
		TCHAR *pEntryID = (TCHAR*)*i;
		SendCmd2YaSync(PE_CMD_DELETE,pEntryID,_tcslen(pEntryID)*sizeof(TCHAR));
		delete[] pEntryID;
	}

	idlist.clear();

}


VOID __stdcall CSMSAddIn::OnClickBTNBuy(IDispatch* /*Office::_CommandBarButton* */ Ctrl,VARIANT_BOOL * CancelDefault)
{
	ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/android-outlook-sync-buy.htm"), NULL, NULL, SW_SHOWMAXIMIZED);
}



VOID __stdcall CSMSAddIn::OnClickBTNHelp(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	SendCmd2YaSync(PE_CMD_HELP,0,0);
}

VOID __stdcall CSMSAddIn::OnClickBTNReplySMS(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_REPLY,szEntry,sizeof(szEntry));
		return;
	}

	MessageBox(NULL,_T("Please select a SMS/Call Log item first"),_T("Error"),MB_ICONSTOP|MB_OK);
}


VOID __stdcall CSMSAddIn::OnClickBTNSave2Contact(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_SAVE_CONTACT,szEntry,sizeof(szEntry));
	}
	else
	{
		SendCmd2YaSync(PE_CMD_SAVE_CONTACT,0,0);
	}
}

VOID __stdcall CSMSAddIn::OnClickBTNFilter(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_FILTER,szEntry,sizeof(szEntry));
	}
	else
	{
		SendCmd2YaSync(PE_CMD_FILTER,0,0);
	}

}

VOID __stdcall CSMSAddIn::OnClickBTNRestore(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}	

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_RESTORE,szEntry,sizeof(szEntry));
	}
	else
	{
		SendCmd2YaSync(PE_CMD_RESTORE,0,0);
	}
}


VOID __stdcall CSMSAddIn::OnClickBTNFile(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}	

	SendCmd2YaSync(PE_CMD_FILE_TRANSFER,0,0);
}


VOID __stdcall CSMSAddIn::OnClickBTNStat(IDispatch *  Ctrl,VARIANT_BOOL * CancelDefault)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return;
	}	

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_STAT,szEntry,sizeof(szEntry));
	}
	else
	{
		//SendCmd2YaSync(PE_CMD_STAT,0,0);
		MessageBox(NULL,_T("Please select a SMS/Call Log item first"),_T("Error"),MB_ICONSTOP|MB_OK);
	}

}




