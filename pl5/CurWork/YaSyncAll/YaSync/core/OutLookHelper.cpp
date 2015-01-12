
#include "stdafx.h"

#include "OutLookHelper.h"
#include "AppTools.h"
#include "PEInterface.h"
#include "mapiutil.h"
#include "FilterHelper.h"
#include "SettingHelper.h"
#include "PECore.h"

#include "CalendarHelper.h"
#include "ContactsHelper.h"
#include "SmsHelper.h"
#include "CallLogHelper.h"

extern HINSTANCE g_HInstance;
extern HWND g_hHomeUIWnd;
extern void ShowSyncInfo(TCHAR* szInfo);

CMAPIEx* COutLookHelper::m_pMAPIMgr;


void COutLookHelper::SBinary2TCHAR(SBinary &bin,TCHAR *pszEntry)
{

}



void COutLookHelper::TCHAR2SBinary(TCHAR *pszEntry,SBinary &bin)
{
	static byte binEntryID[255];
	memset(binEntryID,0,255);

	if (!pszEntry)
	{
		PEWriteLog(_T("pszEntry is null"));
		return;
	}

	int iLen = (int)_tcslen(pszEntry);

	memset(&bin,0,sizeof(bin));
	bin.cb = iLen/2;
	bin.lpb = binEntryID;

	for (int i = 0; i < iLen; i += 2)
	{
		TCHAR hex[3];
		memcpy(hex,pszEntry + i,2*sizeof(TCHAR));
		hex[2] = 0;
		binEntryID[i/2] = (byte)_tcstol(hex,NULL,16);
	}
}

void COutLookHelper::GenFolderName(TCHAR *szFolderName,TCHAR *szFolderType,TCHAR *szPhoneName)
{
	_stprintf(szFolderName,_T("%s-%s"),szPhoneName,szFolderType);
	m_sFolderName = szFolderName;
}

COutLookHelper::COutLookHelper(void)
{
	m_pFolder = NULL;
	m_iDefaultFolder = 0;
}

COutLookHelper::~COutLookHelper(void)
{
	RELEASE(m_pFolder);
}


BOOL COutLookHelper::LogInEx(TCHAR *szPst)
{
	LogOut();

	if (szPst && _tcslen(szPst) < 1)
	{
		szPst = NULL;
	}

	if(!CMAPIEx::Init())
	{
		PEWriteLog(_T("CMAPIEx::Init()  failed"));
		return FALSE;
	}

	m_pMAPIMgr = new CMAPIEx();
	if(!m_pMAPIMgr->Login() || !m_pMAPIMgr->OpenMessageStore(szPst))
	{
		PEWriteLog(_T("m_pMAPIMgr->Login() || m_pMAPIMgr->OpenMessageStore() failed"));
		return FALSE;
	}

	return TRUE;
}



BOOL COutLookHelper::LogIn()
{
	LogOut();

	if(!CMAPIEx::Init())
	{
		PEWriteLog(_T("CMAPIEx::Init() failed"));
		return FALSE;
	}

	TCHAR *szPst = NULL;
	PE_COMMON_SETTING *setting = new PE_COMMON_SETTING();
	CSettingHelper::GetCommonSetting(setting);
	if (!setting->dwUseDefaultPst)
	{
		PEWriteLog("Not use default pst");
		szPst = setting->szPst;
		if (szPst)
		{
			PEWriteLog(szPst);
		}
	}

	m_pMAPIMgr = new CMAPIEx();
	if(!m_pMAPIMgr->Login() || !m_pMAPIMgr->OpenMessageStore(szPst))
	{
		PEWriteLog(_T("m_pMAPIMgr->Login() || m_pMAPIMgr->OpenMessageStore() failed"));
		delete setting;
		return FALSE;
	}

	delete setting;

	return TRUE;
}


void COutLookHelper::LogOut(void)
{
	if (!m_pMAPIMgr)
	{
		return;
	}

	delete m_pMAPIMgr;
	m_pMAPIMgr = NULL;
}



BOOL COutLookHelper::openFolderByID(TCHAR *szFolderID)
{
	m_iDefaultFolder = 0;

	if (!szFolderID || _tcslen(szFolderID) < 1)
	{
		PEWriteLog(_T("szFolderID is null"));
		return FALSE;
	}

	SBinary bin;
	TCHAR2SBinary(szFolderID,bin);
	m_pFolder = m_pMAPIMgr->OpenFolderByEntryID(bin);
	if (m_pFolder != NULL)
	{
		return TRUE;
	}

	PEWriteLog(_T("Faild to create folder:"));
	PEWriteLog(szFolderID);

	return FALSE;
}


GUID MOBITNT_ITEM_ID_GUID				 =	{0x802648a0, 0x94e2, 0x4f91, 0x9c, 0x13, 0xb9, 0xab, 0x25, 0xd9, 0x13, 0xa2};
GUID MOBITNT_THREAD_ID_GUID				 =	{0xb063235d, 0xdeea, 0x4217, 0xa2, 0x58, 0x7f, 0xeb, 0x11, 0xc3, 0x5a, 0x1d};
GUID MOBITNT_SMS_PHONE_NO_GUID			 =	{0x62cf4353, 0xe044, 0x4544, 0xa8, 0x44, 0x4f, 0xea, 0xa7, 0x0a, 0x0a, 0xae};
GUID MOBITNT_CALENDAR_ID_GUID			 =	{0xbe2007d,  0x5411, 0x435f, 0x92, 0xe6, 0xd3, 0xe2, 0xa1, 0x04, 0xbe, 0x7b};
GUID MOBITNT_CALL_NUMBER_GUID			 =	{0xdb07b6a8, 0x2263, 0x49c8, 0xb0, 0x29, 0xc4, 0xa0, 0xb4, 0xfa, 0xa6, 0xea};
GUID MOBITNT_RECV_TYPE_GUID				 =	{0x94041076, 0x65dc, 0x4b85, 0x99, 0xdb, 0x37, 0x18, 0x33, 0x29, 0x16, 0x15};
GUID MOBITNT_MMS_TYPE_GUID				 =	{0x1d15409d, 0xb4d0, 0x4858, 0x86, 0x13, 0xb4, 0x29, 0xcc, 0x6a, 0xb1, 0x6d};
GUID MOBITNT_CONTACTS_TYPE_GUID			 =	{0xf7090720, 0x2b3,  0x4d5c, 0x8f, 0x70, 0xbe, 0xe7, 0x25, 0x9c, 0x38, 0xde};


LPMESSAGE COutLookHelper::FindItemByEntryID(SBinary &bin)
{
	LPMESSAGE pMsg = NULL;
	ULONG ulMesageType;

	HRESULT hr = m_pFolder->OpenEntry(bin.cb,(LPENTRYID)bin.lpb,NULL,MAPI_BEST_ACCESS,&ulMesageType,(LPUNKNOWN*)&pMsg);
	if ((FAILED(hr)))
	{
		pMsg->Release();
		return NULL;
	}

	return pMsg;
}

void COutLookHelper::DeleteItemByEntryID(SBinary &bin)
{
	ENTRYLIST list;
	list.cValues = 1;
	list.lpbin = &bin;
	HRESULT hRes = m_pFolder->DeleteMessages(&list,0,0,0);
	if (FAILED(hRes))
	{
		PEWriteLog(_T("Failed to delete item in outlook folder"));
	}
}


int COutLookHelper::GetLastSyncTime(FILETIME &ftLastSyncTime)
{
	HRESULT hr = S_OK;
	ULONG ulFolderID = 0;

	LPMAPITABLE  pTable = NULL;
	LPMAPITABLE  pTmpTable = NULL;
	LPSRowSet pRows = NULL;
	LPSRowSet pTmpRows = NULL;
	ULONG   ulNumCols = 1;
	//指定我们需要获取Entry ID属性
	SizedSPropTagArray(1, Columns) = 
	{ 
		ulNumCols,
		PR_ENTRYID  
	};

	hr = m_pFolder->GetContentsTable(0, &pTable);
	if ((FAILED(hr)))
	{
		PEWriteLog(_T("GetContentsTable failed"));
		return PE_RET_FAIL;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		PEWriteLog(_T("SetColumns failed"));
		return PE_RET_FAIL;
	}

	hr = S_OK;
	ULONG iCount = 0;
	pTable->GetRowCount(0,&iCount);

	for (ULONG i = 1; i <= iCount; ++i)
	{
		hr = pTable->QueryRows(1,0, &pRows);
		if (FAILED(hr))
		{
			continue;
		}

		if (pRows->cRows != 1)
		{
			pTable->Release();
			pTable = NULL;
			PEWriteLog(_T("QueryRows failed"));
			return PE_RET_FAIL;
		}

		//通过OpenEntry获取IMessage对象
		LPMESSAGE pMsg = NULL;
		ULONG ulMesageType;
		hr = m_pFolder->OpenEntry( pRows->aRow[0].lpProps[0].Value.bin.cb,
			(LPENTRYID)pRows->aRow[0].lpProps[0].Value.bin.lpb,
			NULL, 
			MAPI_BEST_ACCESS, 
			&ulMesageType,
			(LPUNKNOWN*)&pMsg);
		if ((FAILED(hr)))
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			PEWriteLog(_T("OpenEntry failed"));
			return PE_RET_FAIL;
		}

		FILETIME ftSyncTime;
		if (GetItemSyncTime(pMsg,ftSyncTime) == PE_RET_OK)
		{
			if (PECompareFileTime(ftSyncTime,ftLastSyncTime) == PE_RET_CMP_GREATER)
			{
				ftLastSyncTime = ftSyncTime;
			}
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	PEWriteLog(_T("GetLastSyncTime OK"));
	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}

list<TCHAR*> COutLookHelper::GetSubFolderList(int iFolderType)
{
	list<TCHAR*> folderList;
	folderList.clear();

	COutLookHelper *pHelper = NULL;
	if (iFolderType == FOLDER_TYPE_CONTACT)
	{
		pHelper = new ContactsHelper();
	}
	else if (iFolderType == FOLDER_TYPE_CALENDAR)
	{
		pHelper = new CalendarHelper();
	}
	else if (iFolderType == FOLDER_TYPE_SMS)
	{
		pHelper = new SmsHelper();
	}
	else if (iFolderType == FOLDER_TYPE_CALL_LOG)
	{
		pHelper = new CallLogHelper();
	}
	else
	{
		return folderList;
	}

	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("GetSubFolderList:open folder failed"));
		delete pHelper;
		return folderList;
	}

	list<TCHAR*> storeList = GetStoreList();
	list<TCHAR*>::iterator it;
	for (it = storeList.begin(); it != storeList.end(); ++it)
	{ 
		TCHAR *szName = (TCHAR*)*it;

		if (LogInEx(szName))
		{
			list<TCHAR*> subFolderList;
			subFolderList = pHelper->GetSubFolderList();
			list<TCHAR*>::iterator it1;
			for (it1 = subFolderList.begin(); it1 != subFolderList.end(); ++it1)
			{
				TCHAR* szFolder = (TCHAR*)*it1;

				TCHAR *szFolderPath = (TCHAR*)calloc(1,1024);
				_stprintf(szFolderPath,_T("%s//%s"),szName,szFolder);

				folderList.push_back(szFolderPath);
			}

			LogOut();
		}
	}

	FreeStringList(storeList);

	delete pHelper;
	return folderList;

}



list<CONTACT_ITEM*> COutLookHelper::GetContactList()
{
	list<CONTACT_ITEM*> l;
	l.clear();
	ContactsHelper *pHelper = new ContactsHelper();
	if (!pHelper->openFolder())
	{
		PEWriteLog(_T("open folder failed"));
		::SendMessage(g_hHomeUIWnd,WM_SHOW_ERROR_ON_UI,(WPARAM)_T("Get Contacts list failed:failed to open folder"),0);
		delete pHelper;
		return l;
	}

	l = pHelper->GetContactList();

	delete pHelper;
	return l;
}


TCHAR *COutLookHelper::GetDefaultProfile()
{
	CString sName = m_pMAPIMgr->GetProfileName();
	TCHAR *szName = (TCHAR*)calloc(1,sizeof(TCHAR)*255);

	_tcscpy(szName,sName.GetBuffer());
	sName.ReleaseBuffer();

	return szName;
}


list<TCHAR*> COutLookHelper::GetStoreList()
{
	return m_pMAPIMgr->GetMsgStoreList();
}




