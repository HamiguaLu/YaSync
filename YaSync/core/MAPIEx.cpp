////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// File: MAPIEx.cpp
// Description: Windows Extended MAPI class 
//
// Copyright (C) 2005-2006, Noel Dillabough
//
// This source code is free to use and modify provided this notice remains intact and that any enhancements
// or bug fixes are posted to the CodeProject page hosting this class for the community to benefit.
//
// Usage: see the Codeproject article at http://www.codeproject.com/internet/CMapiEx.asp
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#include "MAPIExPCH.h"
#include "MAPIEx.h"
//#include "MAPISink.h"
#include <MapiUtil.h>

#pragma comment (lib,"mapi32.lib")
#pragma comment(lib,"Ole32.lib")

/////////////////////////////////////////////////////////////
// CMAPIEx

#ifdef UNICODE
int CMAPIEx::cm_nMAPICode=MAPI_UNICODE;
#else
int CMAPIEx::cm_nMAPICode=0;
#endif

extern list<TCHAR*> splitString(TCHAR *srcStr,TCHAR szDiv);


CMAPIEx::CMAPIEx()
{
	m_pSession=NULL;
	m_pMsgStore=NULL;
	m_pFolder=NULL;	
	m_pContents=NULL;
	m_pHierarchy=NULL;
	m_sink=0;
}

CMAPIEx::~CMAPIEx()
{
	Logout();
}

BOOL CMAPIEx::Init(BOOL bMultiThreadedNotifcations)
{
	if(bMultiThreadedNotifcations) {
		MAPIINIT_0 MAPIInit={ 0, MAPI_MULTITHREAD_NOTIFICATIONS };
		if(MAPIInitialize(&MAPIInit)!=S_OK) return FALSE;
	} else {
		if(MAPIInitialize(NULL)!=S_OK) return FALSE;
	}
	return TRUE;
}

void CMAPIEx::Term()
{
	MAPIUninitialize();
}

BOOL CMAPIEx::Login(LPCTSTR szProfileName)
{
	return (MAPILogonEx(NULL,(LPTSTR)szProfileName,NULL,MAPI_EXTENDED | MAPI_USE_DEFAULT | MAPI_NEW_SESSION,&m_pSession)==S_OK);
}

void CMAPIEx::Logout()
{
	if(m_sink) {
		if(m_pMsgStore) m_pMsgStore->Unadvise(m_sink);
		m_sink=0;
	}

	RELEASE(m_pHierarchy);
	RELEASE(m_pContents);
	RELEASE(m_pFolder);
	RELEASE(m_pMsgStore);
	RELEASE(m_pSession);
}


list<TCHAR*> CMAPIEx::GetMsgStoreList()
{
	list<TCHAR*> serviceList;
	HRESULT hr;
	LPSRowSet ptrRowSet = NULL;
	LPSRow   ptrRow = NULL;
	unsigned long lRowCount;
	unsigned long lLoopCounter;
	LPSPropValue  ptrPropVal = NULL;

	serviceList.clear();

	LPMAPITABLE lpStoreTable = NULL;
	m_pSession->GetMsgStoresTable(	0,&lpStoreTable);
	if ( !lpStoreTable)
	{
		return serviceList;
	}

	hr = lpStoreTable->GetRowCount( 0, &lRowCount);
	if (FAILED(hr))
	{
		return serviceList;
	}

	hr = lpStoreTable->QueryRows( lRowCount , 0, &ptrRowSet );
	if (FAILED(hr))
	{
		return serviceList;
	}

	for ( lLoopCounter = 0  ; lLoopCounter < lRowCount ; lLoopCounter++)
	{
		ptrRow = &ptrRowSet->aRow[ lLoopCounter];

		for (int i = 0; i < (int)ptrRow->cValues; ++i)
		{
			ptrPropVal = &ptrRow->lpProps[i];
#if 0
			ULONG ulTag = PR_RESOURCE_FLAGS;
			if (ptrPropVal->ulPropTag == ulTag)
			{
				int abc = 0;
			}
#endif

			if (ptrPropVal->ulPropTag != PR_DISPLAY_NAME_A)
			{
				continue;
			}

			char *tmp = ptrPropVal->Value.lpszA;

			TCHAR *szName = (TCHAR*)calloc(1,1024);
			MultiByteToWideChar(CP_ACP,0,tmp,-1,szName,1024);

			serviceList.push_back(szName);
		}
	} 

	if (ptrRowSet)
	{
		FreeProws(ptrRowSet);
	}

	return serviceList;

}

list<TCHAR*> CMAPIEx::GetProfileList()
{
	HRESULT hrRet;
	LPPROFADMIN ptrProfAdmin = NULL;
	LPMAPITABLE ptrProfileTable = NULL;
	LPSRowSet ptrRowSet = NULL;
	LPSRow   ptrRow = NULL;
	LPSPropValue  ptrPropVal = NULL;
	LPSERVICEADMIN  ptrServAdmin = NULL;
	LPMAPITABLE ptrServiceTable = NULL;
	LPSRowSet ptrServRowSet = NULL;
	LPSRow   ptrServRow = NULL;
	LPSPropValue  ptrServPropVal = NULL;
	unsigned long lRowCount;
	unsigned long lLoopCounter;
	//unsigned long lServiceCount ;

	list<TCHAR*> profileList;
	profileList.clear();

	/* get admin profiles pointer */
	hrRet = MAPIAdminProfiles( 0, &ptrProfAdmin);

	hrRet = ptrProfAdmin->GetProfileTable( 0, &ptrProfileTable);
	hrRet = ptrProfileTable->GetRowCount( 0, &lRowCount);
	hrRet = ptrProfileTable->QueryRows( lRowCount , 0, &ptrRowSet );

	for ( lLoopCounter = 0  ; lLoopCounter < lRowCount ; lLoopCounter++)
	{
		ptrRow = &ptrRowSet->aRow[ lLoopCounter];
		ptrPropVal = &ptrRow->lpProps[0];
		char *tmp = ptrPropVal->Value.lpszA;

		TCHAR *szName = (TCHAR*)calloc(1,1024);
		MultiByteToWideChar(CP_ACP,0,tmp,-1,szName,1024);

		profileList.push_back(szName);
	} 

	if (ptrRowSet)
	{
		FreeProws(ptrRowSet);
	}

	if (ptrProfileTable)
	{
		ptrProfileTable->Release();
	}

	if (ptrProfAdmin)
	{
		ptrProfAdmin->Release();
	}

	return profileList;
}


// if I try to use MAPI_UNICODE when UNICODE is defined I get the MAPI_E_BAD_CHARWIDTH 
// error so I force narrow strings here
LPCTSTR CMAPIEx::GetProfileName()
{
	if(!m_pSession) return NULL;

	static CString strProfileName;
	LPSRowSet pRows=NULL;
	const int nProperties=2;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME_A, PR_RESOURCE_TYPE}};

	IMAPITable*	pStatusTable;
	if(m_pSession->GetStatusTable(0,&pStatusTable)==S_OK) {
		if(pStatusTable->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) {
			while(TRUE) {
				if(pStatusTable->QueryRows(1,0,&pRows)!=S_OK) MAPIFreeBuffer(pRows);
				else if(pRows->cRows!=1) FreeProws(pRows);
				else if(pRows->aRow[0].lpProps[1].Value.ul==MAPI_SUBSYSTEM) {
					strProfileName=(LPSTR)GetValidString(pRows->aRow[0].lpProps[0]);
					FreeProws(pRows);
				} else {
					FreeProws(pRows);
					continue;
				}
				break;
			}
		}
		RELEASE(pStatusTable);
	}
	return strProfileName;
}

BOOL CMAPIEx::OpenMessageStore(LPCTSTR szStore,ULONG ulFlags)
{
	if(!m_pSession) return FALSE;

	m_ulMDBFlags=ulFlags;

	LPSRowSet pRows=NULL;
	const int nProperties=3;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME, PR_ENTRYID, PR_DEFAULT_STORE}};

	BOOL bResult=FALSE;
	IMAPITable*	pMsgStoresTable;
	if(m_pSession->GetMsgStoresTable(0, &pMsgStoresTable)==S_OK) {
		if(pMsgStoresTable->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) {
			while(TRUE) {
				if(pMsgStoresTable->QueryRows(1,0,&pRows)!=S_OK) MAPIFreeBuffer(pRows);
				else if(pRows->cRows!=1) FreeProws(pRows);
				else {
					if(!szStore) { 
						if(pRows->aRow[0].lpProps[2].Value.b) bResult=TRUE;
					} else {
						CString strStore=GetValidString(pRows->aRow[0].lpProps[0]);
						if(strStore.Find(szStore)!=-1) bResult=TRUE;
					}
					if(!bResult) {
						FreeProws(pRows);
						continue;
					}
				}
				break;
			}
			if(bResult) {
				RELEASE(m_pMsgStore);
				bResult=(m_pSession->OpenMsgStore(NULL,pRows->aRow[0].lpProps[1].Value.bin.cb,(ENTRYID*)pRows->aRow[0].lpProps[1].Value.bin.lpb,NULL,MDB_NO_DIALOG | MAPI_BEST_ACCESS,&m_pMsgStore)==S_OK);
				FreeProws(pRows);
			}
		}
		RELEASE(pMsgStoresTable);
	}
	return bResult;
}

ULONG CMAPIEx::GetMessageStoreSupport()
{
	if(!m_pMsgStore) return FALSE;

	LPSPropValue props=NULL;
	ULONG cValues=0;
	ULONG rgTags[]={ 1, PR_STORE_SUPPORT_MASK };
	ULONG ulSupport=0;

	if(m_pMsgStore->GetProps((LPSPropTagArray) rgTags, CMAPIEx::cm_nMAPICode, &cValues, &props)==S_OK) {
		ulSupport=props->Value.ul;
		MAPIFreeBuffer(props);
	}
	return ulSupport;
}


LPMAPIFOLDER CMAPIEx::OpenFolderByEntryID(SBinary &bin)
{
	if(!m_pMsgStore) return NULL;

	LPSPropValue props=NULL;
	ULONG cValues=0;
	DWORD dwObjType;

	LPMAPIFOLDER pFolder = NULL;

	m_pMsgStore->OpenEntry(bin.cb,(LPENTRYID)bin.lpb, NULL, m_ulMDBFlags, &dwObjType,(LPUNKNOWN*)&pFolder);

	return pFolder;
}


LPMAPIFOLDER CMAPIEx::OpenFolderByPath(TCHAR *szPath)
{
	if(!m_pMsgStore) return NULL;

	LPMAPIFOLDER pFolder = NULL;
	LPMAPIFOLDER pSubFolder = NULL;

	list<TCHAR*> pathList = splitString(szPath,_T('\\'));
	if (pathList.size() < 1)
	{
		return NULL;
	}

	pFolder = OpenRootFolder(false);
	if (!pFolder)
	{
		return NULL;
	}

	list<TCHAR*>::iterator it;

	for (it = pathList.begin();it != pathList.end(); ++it)
	{ 
		TCHAR *szPath = (TCHAR*)*it;
		pSubFolder = OpenSubFolder(szPath,pFolder);
		free(szPath);
		if (!pSubFolder)
		{
			break;
		}

		pFolder = pSubFolder;
	}

	pathList.clear();

	return pSubFolder;
}

LPMAPIFOLDER CMAPIEx::OpenFolder(unsigned long ulFolderID,BOOL bInternal)
{
	if(!m_pMsgStore) return NULL;

	LPSPropValue props=NULL;
	ULONG cValues=0;
	DWORD dwObjType;
	ULONG rgTags[]={ 1, ulFolderID };
	LPMAPIFOLDER pFolder;

	if(m_pMsgStore->GetProps((LPSPropTagArray) rgTags, cm_nMAPICode, &cValues, &props)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb, NULL, m_ulMDBFlags, &dwObjType,(LPUNKNOWN*)&pFolder);
	MAPIFreeBuffer(props);

	if(pFolder && bInternal) {
		RELEASE(m_pFolder);
		m_pFolder=pFolder;
	}
	return pFolder;
}

LPMAPIFOLDER CMAPIEx::OpenSpecialFolder(unsigned long ulFolderID,BOOL bInternal)
{
	LPMAPIFOLDER pInbox=OpenInbox(FALSE);
	if(!pInbox || !m_pMsgStore) return FALSE;

	LPSPropValue props=NULL;
	ULONG cValues=0;
	DWORD dwObjType;
	ULONG rgTags[]={ 1, ulFolderID };
	LPMAPIFOLDER pFolder;

	if(pInbox->GetProps((LPSPropTagArray) rgTags, cm_nMAPICode, &cValues, &props)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb, NULL, m_ulMDBFlags, &dwObjType,(LPUNKNOWN*)&pFolder);
	MAPIFreeBuffer(props);
	RELEASE(pInbox);

	if(pFolder && bInternal) {
		RELEASE(m_pFolder);
		m_pFolder=pFolder;
	}
	return pFolder;
}

LPMAPIFOLDER CMAPIEx::OpenRootFolder(BOOL bInternal)
{
	return OpenFolder(PR_IPM_SUBTREE_ENTRYID,bInternal);
}

LPMAPIFOLDER CMAPIEx::OpenInbox(BOOL bInternal)
{
	if(!m_pMsgStore) return NULL;

	ULONG cbEntryID;
	LPENTRYID pEntryID;
	DWORD dwObjType;
	LPMAPIFOLDER pFolder;

	if(m_pMsgStore->GetReceiveFolder(NULL,0,&cbEntryID,&pEntryID,NULL)!=S_OK) return NULL;
	m_pMsgStore->OpenEntry(cbEntryID,pEntryID, NULL, m_ulMDBFlags,&dwObjType,(LPUNKNOWN*)&pFolder);
	MAPIFreeBuffer(pEntryID);

	if(pFolder && bInternal) {
		RELEASE(m_pFolder);
		m_pFolder=pFolder;
	}
	return pFolder;
}

int CMAPIEx::GetFolderName(TCHAR *szName,LPMAPIFOLDER pFolder) 
{
	if(!pFolder) 
	{
		PEWriteLog(_T("folder is null"));
		return PE_RET_FAIL;
	}

	//int	mcols = 1;
	SizedSPropTagArray(1,mcols)={1,{PR_DISPLAY_NAME}};

	ULONG ulCount;
	SPropValue *props=0;

	HRESULT hr = pFolder->GetProps((SPropTagArray*)&mcols,0,&ulCount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		PEWriteLog(_T("Failed to GetProps"));
		return PE_RET_FAIL;
	}

	if (ulCount != 1)
	{
		PEWriteLog(_T("prop count is wrong"));
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag != PR_DISPLAY_NAME)
	{
		PEWriteLog(_T("no display name found"));
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	_tcscpy(szName,props[0].Value.lpszW);

	MAPIFreeBuffer(props);

	return PE_RET_OK;
}


LPMAPIFOLDER CMAPIEx::OpenOutbox(BOOL bInternal)
{
	return OpenFolder(PR_IPM_OUTBOX_ENTRYID,bInternal);
}

LPMAPIFOLDER CMAPIEx::OpenSentItems(BOOL bInternal)
{
	return OpenFolder(PR_IPM_SENTMAIL_ENTRYID,bInternal);
}

LPMAPIFOLDER CMAPIEx::OpenDeletedItems(BOOL bInternal)
{
	return OpenFolder(PR_IPM_WASTEBASKET_ENTRYID,bInternal);
}

LPMAPIFOLDER CMAPIEx::OpenContacts(BOOL bInternal)
{
	return OpenSpecialFolder(PR_IPM_CONTACT_ENTRYID,bInternal);
}

LPMAPIFOLDER CMAPIEx::OpenDrafts(BOOL bInternal)
{
	return OpenSpecialFolder(PR_IPM_DRAFTS_ENTRYID,bInternal);
}

LPMAPITABLE CMAPIEx::GetHierarchy(LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return NULL;
	}
	RELEASE(m_pHierarchy);
	if(pFolder->GetHierarchyTable(0,&m_pHierarchy)!=S_OK) return NULL;

	const int nProperties=3;
	SizedSPropTagArray(nProperties,Columns)={nProperties,{PR_DISPLAY_NAME, PR_ENTRYID,PR_COMMENT}};
	if(m_pHierarchy->SetColumns((LPSPropTagArray)&Columns, 0)==S_OK) return m_pHierarchy;
	return NULL;
}

LPMAPIFOLDER CMAPIEx::GetNextSubFolder(CString& strFolderName,LPMAPIFOLDER pFolder)
{
	if(!m_pHierarchy)
	{
		return NULL;
	}
	
	if(!pFolder) 
	{
		pFolder = m_pFolder;
		if(!pFolder)
		{
			return FALSE;
		}
	}

	DWORD dwObjType;
	LPSRowSet pRows = NULL;

	LPMAPIFOLDER pSubFolder=NULL;
	HRESULT hr = m_pHierarchy->QueryRows(1,0,&pRows);
	if (FAILED(hr))
	{
		PEWriteLog(_T("QueryRows failed"));
		MAPIFreeBuffer(pRows);
		return NULL;
	}

	if ( !pRows || pRows->cRows < 1)
	{
		PEWriteLog(_T("GetNextSubFolder:pRows->cRows is zero"));
		return NULL;
	}

	if(pFolder->OpenEntry(pRows->aRow[0].lpProps[PROP_ENTRYID].Value.bin.cb,(LPENTRYID)pRows->aRow[0].lpProps[PROP_ENTRYID].Value.bin.lpb, NULL, MAPI_MODIFY, &dwObjType,(LPUNKNOWN*)&pSubFolder)==S_OK) 
	{
		strFolderName=GetValidString(pRows->aRow[0].lpProps[0]);
	}

	FreeProws(pRows);

	return pSubFolder;
}

// High Level function to open a sub folder by iterating recursively (DFS) over all folders 
// (use instead of manually calling GetHierarchy and GetNextSubFolder)
LPMAPIFOLDER CMAPIEx::OpenSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder)
{
	LPMAPIFOLDER pSubFolder=NULL;
	LPMAPITABLE pHierarchy;

	RELEASE(m_pHierarchy);
	pHierarchy=GetHierarchy(pFolder);
	if(pHierarchy) {
		CString strFolder;
		LPMAPIFOLDER pRecurse=NULL;
		do {
			RELEASE(pSubFolder);
			m_pHierarchy=pHierarchy;
			pSubFolder=GetNextSubFolder(strFolder,pFolder);
			if(pSubFolder) {
				if(!strFolder.CompareNoCase(szSubFolder)) break;
				m_pHierarchy=NULL; // so we don't release it in subsequent drilldown
				pRecurse=OpenSubFolder(szSubFolder,pSubFolder);
				if(pRecurse) {
					RELEASE(pSubFolder);
					pSubFolder=pRecurse;
					break;
				}
			}
		} while(pSubFolder);
		RELEASE(pHierarchy);
		m_pHierarchy=NULL;
	}

	//comment by lugang
#if 0
	// this may occur many times depending on how deep the recursion is; make sure we haven't already assigned m_pFolder
	if(pSubFolder && (m_pFolder!=pSubFolder) && (pFolder != m_pFolder) ) {
		RELEASE(m_pFolder);
		m_pFolder=pSubFolder;
	}
#endif

	return pSubFolder;
} 

/*
int CMAPIEx::GetRowCount()
{
ULONG ulCount;
if(!m_pContents || m_pContents->GetRowCount(0,&ulCount)!=S_OK) return -1;
return ulCount;
}
*/


// Creates a subfolder under pFolder, opens the folder if it already exists
LPMAPIFOLDER CMAPIEx::CreateSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return NULL;
	}

	LPMAPIFOLDER pSubFolder=NULL;
	ULONG ulFolderType=FOLDER_GENERIC;
	ULONG ulFlags=OPEN_IF_EXISTS | cm_nMAPICode;

	pFolder->CreateFolder(ulFolderType,(LPTSTR)szSubFolder,NULL,NULL,ulFlags,&pSubFolder);
	return pSubFolder;
}

// Deletes a sub folder and ALL sub folders/messages
BOOL CMAPIEx::DeleteSubFolder(LPCTSTR szSubFolder,LPMAPIFOLDER pFolder)
{
	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}

	LPMAPIFOLDER pSubFolder=NULL;
	if(GetHierarchy(pFolder)) {
		CString strFolder;
		do {
			RELEASE(pSubFolder);
			pSubFolder=GetNextSubFolder(strFolder,pFolder);
			if(pSubFolder && !strFolder.CompareNoCase(szSubFolder)) break;
		} while(pSubFolder);
	}
	return DeleteSubFolder(pSubFolder,pFolder);
}

// Deletes a sub folder and ALL sub folders/messages
BOOL CMAPIEx::DeleteSubFolder(LPMAPIFOLDER pSubFolder,LPMAPIFOLDER pFolder)
{
	if(!pSubFolder) return FALSE;

	if(!pFolder) {
		pFolder=m_pFolder;
		if(!pFolder) return FALSE;
	}

	LPSPropValue props=NULL;
	ULONG cValues=0;
	ULONG rgTags[]={ 1, PR_ENTRYID };

	if(pSubFolder->GetProps((LPSPropTagArray) rgTags, CMAPIEx::cm_nMAPICode, &cValues, &props)==S_OK) {
		HRESULT hr=pFolder->DeleteFolder(props[0].Value.bin.cb,(LPENTRYID)props[0].Value.bin.lpb,NULL,NULL,DEL_FOLDERS|DEL_MESSAGES);
		MAPIFreeBuffer(props);
		return (hr==S_OK);
	}
	return FALSE;
}

/*
// call with ulEventMask set to ALL notifications ORed together, only one Advise Sink is used.
BOOL CMAPIEx::Notify(LPNOTIFCALLBACK lpfnCallback,LPVOID lpvContext,ULONG ulEventMask)
{
if(GetMessageStoreSupport()&STORE_NOTIFY_OK) {
if(m_sink) m_pMsgStore->Unadvise(m_sink);
CMAPISink* pAdviseSink=new CMAPISink(lpfnCallback,lpvContext);
if(m_pMsgStore->Advise(0,NULL,ulEventMask,pAdviseSink,&m_sink)==S_OK) return TRUE;
delete pAdviseSink;
m_sink=0;
}
return FALSE;
}*/

// sometimes the string in prop is invalid, causing unexpected crashes
LPCTSTR CMAPIEx::GetValidString(SPropValue& prop)
{
	LPCTSTR s=prop.Value.LPSZ;
	if(s && !::IsBadStringPtr(s,(UINT_PTR)-1)) return s;
	return NULL;
}

// special case of GetValidString to take the narrow string in UNICODE
void CMAPIEx::GetNarrowString(SPropValue& prop,CString& strNarrow)
{
	LPCTSTR s=GetValidString(prop);
	if(!s) strNarrow=_T("");
	else {
#ifdef UNICODE
		// VS2005 can copy directly
		if(_MSC_VER>=1400) {
			strNarrow=(TCHAR*)s;
		} else {
			WCHAR wszWide[256];
			//			MultiByteToWideChar(CP_ACP,0,(LPCTSTR)s,-1,wszWide,255);
			strNarrow=wszWide;
		}
#else
		strNarrow=s;
#endif
	}
}


void CMAPIEx::GetSystemTime(SYSTEMTIME& tm,int wYear,int wMonth,int wDay,int wHour,int wMinute,int wSecond,int wMilliSeconds)
{
	tm.wYear=(WORD)wYear;
	tm.wMonth=(WORD)wMonth;
	tm.wDay=(WORD)wDay;
	tm.wHour=(WORD)wHour;
	tm.wMinute=(WORD)wMinute;
	tm.wSecond=(WORD)wSecond;
	tm.wMilliseconds=(WORD)wMilliSeconds;
	tm.wDayOfWeek=0;
}



list<TCHAR*> CMAPIEx::GetSubFolderList(LPMAPIFOLDER pFolder,TCHAR *szRoot,int iType)
{
	//enumrate all sub folders within contact folder
	LPMAPIFOLDER pSubFolder = NULL;
	LPMAPITABLE pHierarchyr = NULL;
	list<TCHAR*> subFolderList;

	subFolderList.clear();

	pHierarchyr = GetHierarchy(pFolder);
	if(!pHierarchyr) {
		return subFolderList;	
	}

	CString strFolder;
	do {
		RELEASE(pSubFolder);
		pSubFolder= GetNextSubFolder(strFolder,pFolder);

		if(!pSubFolder)
		{
			continue;
		}

		SizedSPropTagArray(1, mcols) = {1,{PR_CONTAINER_CLASS}};
		ULONG pcount;
		SPropValue *props=0;

		HRESULT hr = pSubFolder->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
		if (FAILED(hr) )
		{
			MAPIFreeBuffer(props);
			continue;
		}
			
		if (pcount != 1 || props[0].ulPropTag != PR_CONTAINER_CLASS)
		{
			MAPIFreeBuffer(props);
			continue;
		}

		TCHAR*szClass	= props[0].Value.lpszW;

		if (iType == FOLDER_TYPE_CONTACT)
		{
			if (_tcscmp(szClass,_T("IPF.Contact") ) != 0)
			{
				MAPIFreeBuffer(props);
				continue;

			}
		}
		else if (iType == FOLDER_TYPE_CALENDAR)
		{
			if (_tcscmp(szClass,_T("IPF.Appointment") ) != 0)
			{
				MAPIFreeBuffer(props);
				continue;
			}
		}

		MAPIFreeBuffer(props);

		TCHAR *szName = (TCHAR*)calloc(1,1024);
		if (szRoot)
		{
			_stprintf(szName,_T("%s\\%s"),szRoot,strFolder.GetBuffer());
		}
		else
		{
			_tcscpy(szName,strFolder.GetBuffer());
		}
		strFolder.ReleaseBuffer();
		subFolderList.push_back(szName);


	} while(pSubFolder);


	RELEASE(pSubFolder);
	return subFolderList;	
}

