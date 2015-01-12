// Connect.cpp : Implementation of CConnect
#include "stdafx.h"
#include "AddIn.h"
#include "Connect.h"

#include "PEProtocol.h"

extern CAddInModule _AtlModule;

// When run, the Add-in wizard prepared the registry for the Add-in.
// At a later time, if the Add-in becomes unavailable for reasons such as:
//   1) You moved this project to a computer other than which is was originally created on.
//   2) You chose 'Yes' when presented with a message asking if you wish to remove the Add-in.
//   3) Registry corruption.
// you will need to re-register the Add-in by building the AndroidAddInSetup project, 
// right click the project in the Solution Explorer, then choose install.


// CConnect
STDMETHODIMP CConnect::OnConnection(IDispatch *pApplication, AddInDesignerObjects::ext_ConnectMode /*ConnectMode*/, IDispatch *pAddInInst, SAFEARRAY ** /*custom*/ )
{
	int i = 0;
	pApplication->QueryInterface(__uuidof(IDispatch), (LPVOID*)&m_pApplication);
	pAddInInst->QueryInterface(__uuidof(IDispatch), (LPVOID*)&m_pAddInInstance);
	return S_OK;
}

STDMETHODIMP CConnect::OnDisconnection(AddInDesignerObjects::ext_DisconnectMode /*RemoveMode*/, SAFEARRAY ** /*custom*/ )
{
	m_pApplication = NULL;
	m_pAddInInstance = NULL;
	return S_OK;
}

STDMETHODIMP CConnect::OnAddInsUpdate (SAFEARRAY ** /*custom*/ )
{
	return S_OK;
}

STDMETHODIMP CConnect::OnStartupComplete (SAFEARRAY ** /*custom*/ )
{
	return S_OK;
}

STDMETHODIMP CConnect::OnBeginShutdown (SAFEARRAY ** /*custom*/ )
{
	return S_OK;
}


STDMETHODIMP CConnect::NewSmsBtnClicked(IDispatch* btn)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	SendCmd2YaSync(PE_CMD_NEW_SMS,0,0);

	return S_OK;
}


STDMETHODIMP CConnect::ReplySmsBtnClicked(IDispatch* RibbonControl)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_REPLY,szEntry,sizeof(szEntry));
	}
	else
	{
		MessageBox(NULL,_T("Please select a SMS/Call Log item first"),_T("Error"),MB_ICONSTOP|MB_OK);
	}

	return S_OK;
}
STDMETHODIMP CConnect::StatBtnClicked(IDispatch* RibbonControl)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
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

	return S_OK;
}

STDMETHODIMP CConnect::HelpBtnClicked(IDispatch* btn)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	SendCmd2YaSync(PE_CMD_HELP,0,0);
	return S_OK;
}

STDMETHODIMP CConnect::SyncBtnClicked(IDispatch* RibbonControl)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	SendCmd2YaSync(PE_CMD_SYNC,0,0);
	return S_OK;
}

STDMETHODIMP CConnect::ThreadBtnClicked(IDispatch* btn)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_THREAD,szEntry,sizeof(szEntry));
	}
	else
	{
		MessageBox(NULL,_T("Please select a SMS item first"),_T("Error"),MB_ICONSTOP|MB_OK);
	}

	return S_OK;
}

STDMETHODIMP CConnect::SaveContactBtnClicked(IDispatch* RibbonControl)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
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


	return S_OK;
}

STDMETHODIMP CConnect::FilterBtnClicked(IDispatch* btn)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	TCHAR szEntry[MAX_ENTRYID_LEN];
	if (PE_RET_OK == getFirstSelectedItemID(szEntry))
	{
		SendCmd2YaSync(PE_CMD_FILTER,szEntry,sizeof(szEntry));
	}
	else
	{
		//SendCmd2YaSync(PE_CMD_FILTER,0,0);
		MessageBox(NULL,_T("Please select a SMS/Call Log item first"),_T("Error"),MB_ICONSTOP|MB_OK);
	}

	return S_OK;
}

STDMETHODIMP CConnect::DeleteBtnClicked(IDispatch* btn)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	list<TCHAR*> idlist = getSelectedItemID();
	if (idlist.size() <= 0)
	{
		MessageBox(NULL,_T("Please select a SMS/Call Log/Contact item first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}

	list<TCHAR*>::iterator i;
	for (i = idlist.begin(); i != idlist.end(); ++i)
	{ 
		TCHAR *pEntryID = (TCHAR*)*i;
		SendCmd2YaSync(PE_CMD_DELETE,pEntryID,(int)_tcslen(pEntryID)*sizeof(TCHAR));
		delete[] pEntryID;
	}

	idlist.clear();

	return S_OK;
}

STDMETHODIMP CConnect::FileBtnClicked(IDispatch* btn)
{
	if (IsInstanceExist() != PE_RET_OK)
	{
		//MessageBox(NULL,_T("Please Start YaSync first"),_T("Error"),MB_ICONSTOP|MB_OK);
		return S_OK;
	}	

	SendCmd2YaSync(PE_CMD_FILE_TRANSFER,0,0);


	return S_OK;
}

STDMETHODIMP CConnect::BuyBtnClicked(IDispatch* btn)
{
	ShellExecute(0, _T("open"), _T("http://www.mobitnt.com/android-outlook-sync-buy.htm"), NULL, NULL, SW_SHOWMAXIMIZED);
	return  S_OK;
}



int CConnect::getFirstSelectedItemID(TCHAR *szEntryID)
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

list<TCHAR*> CConnect::getSelectedItemID()
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


