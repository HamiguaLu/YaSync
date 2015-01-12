#include "stdafx.h"
#include "CallLogHelper.h"
#include "PECore.h"
#include "AppTools.h"
#include "mapiutil.h"
#include "PEInterface.h"
#include <initguid.h>

#include "TrayHelper.h"
#include "SettingHelper.h"

extern int g_iCanSync;

DEFINE_OLEGUID(PSETID_Log, MAKELONG(0x2000+(0x0a),0x0006),0,0);
DEFINE_OLEGUID(PS_PUBLIC_STRINGS, MAKELONG(0x0329+(0x00),0x0002),0,0);
DEFINE_OLEGUID(PSETID_Common1, MAKELONG(0x2000+(0x8),0x0006),0,0);



ULONG aulPhoneCallProps[] = {
	PidLidPhoneCallStartTime,
	PidLidPhoneCallEndTime	,
	PidLidPhoneCallDuration,
	PidLidPhoneCallTypeDescription,
	PidLidPhoneCallType,
};

#define ulPhoneCallProps (sizeof(aulPhoneCallProps)/sizeof(aulPhoneCallProps[0]))


BOOL CallLogHelper::openFolder()
{
	m_pFolder = NULL;
	m_iDefaultFolder = 1;
	
	PE_PHONE_SETTING *s = PECore::GetCurPhone();
	if (s && _tcslen(s->szCallFolderName) > 0)
	{
		PEWriteLog("CALL Folder Name:");
		PEWriteLog(s->szCallFolderName);
		m_pFolder = m_pMAPIMgr->OpenFolderByPath(s->szCallFolderName);
		if (m_pFolder)
		{
			return TRUE;			
		}

		PEWriteLog(_T("Can not open folder for Call Log,will use default"));
		return FALSE;
	}

	TCHAR szPhoneName[255];
	if (PECore::GetCurPhoneName(szPhoneName) != PE_RET_OK)
	{
		PEWriteLog(_T("GetCurPhoneName failed"));
		return FALSE;
	}

	TCHAR szFolderName[1024];
	GenFolderName(szFolderName,_T("CallLog"),szPhoneName);

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenSpecialFolder(PR_IPM_JOURNAL_ENTRYID,FALSE);
	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("open call log root folder failed!"));
		return FALSE;
	}

	HRESULT hr = pRootFolder->CreateFolder(	FOLDER_GENERIC,
		szFolderName,
		MOBITNT_CALL_FOLDER_COMMENT,
		NULL,
		MAPI_UNICODE|OPEN_IF_EXISTS,
		&m_pFolder);
	
	if (FAILED(hr))
	{
		PEWriteLog(_T("Can not create sub folder for call log"));
		m_pFolder = pRootFolder;
		return TRUE;
	}

	pRootFolder->Release();

	PEWriteLog(_T("CallLog Folder:"));
	PEWriteLog(szFolderName);

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag =  PR_CONTAINER_CLASS;
	spvProps[0].Value.lpszW = _T("IPF.Journal");

	hr = m_pFolder->SetProps(1,spvProps,NULL);
	if (FAILED(hr))
	{
		PEWriteLog(_T("Faild to set call folder property"));
	}

	hr = m_pFolder->SaveChanges(0);
	if (FAILED(hr))
	{
		PEWriteLog(_T("Can not SetProps SaveChanges for call log"));
		m_pFolder->Release();
		return FALSE;
	}

	return TRUE;
}



void CallLogHelper::FormatCallLog(CALL_LOG_ITEM& call,CString &szBody,CString &szSubject)
{
	TCHAR *pszNumber = call.szNumberW;
	TCHAR *pszName = call.szNameW;

	int iRet = PE_RET_OK;

	szSubject = _T("");
	
	switch(call.ulCallType)
	{
	case CALL_TYPE_MISSED_CALL:
		{
			szBody += _T("Missed call from ");
			//szSubject = _T("[Missed]:");
			break;
		}
	case CALL_TYPE_INCOMING_CALL:
		{
			szBody += _T("Incoming call from ");
			//szSubject = _T("[  In  ]:");
			break;
		}
	case CALL_TYPE_OUTGOING_CALL:
		{
			szBody += _T("Outgoing call to ");
			//szSubject = _T("[  Out ]:");
			break;
		}
	case CALL_TYPE_VOICE_MAIL:
		{
			szBody += _T("Voice Mail ");
			//szSubject = _T("[  VoiceMail ]:");
			break;
		}
	case CALL_TYPE_REJECTED:
		{
			szBody += _T("Rejected call ");
			//szSubject = _T("[  Rejected ]:");
			break;
		}
	case CALL_TYPE_REFUSED:
		{
			szBody += _T("Refused call  ");
			//szSubject = _T("[  Refused ]:");
			break;
		}
	default:
		{
			szBody += _T("Private  ");
			//szSubject = _T("[  Private ]:");
			break;
		}
	}

	

	if (_tcslen(pszName) > 0)
	{
		szBody += pszName;
		szSubject += pszName;
	}

	if (_tcslen(pszNumber) > 0)
	{
		szBody += _T("<");
		szBody += pszNumber;
		szBody += _T(">");

		szSubject += _T("<");
		szSubject += pszNumber;
		szSubject += _T(">");

	}

	//*szSubject += " at time ";
	TCHAR szStartTime[255];
	TCHAR szSDate[100];
	TCHAR szSTime[100];

	TCHAR szEndTime[255];
	TCHAR szEDate[100];
	TCHAR szETime[100];


	FILETIME lftStart,lftEnd;
	FileTimeToLocalFileTime(&call.ftStartTime,&lftStart);
	FileTimeToLocalFileTime(&call.ftEndTime,&lftEnd);

	SYSTEMTIME stStart,stEnd;
	FileTimeToSystemTime(&lftStart,&stStart);
	FileTimeToSystemTime(&lftEnd,&stEnd);

	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stStart, NULL,szSTime, 255);
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &stEnd, NULL,szETime, 255);
	GetDateFormat(LOCALE_USER_DEFAULT,0,&stStart,NULL,szSDate,255);
	GetDateFormat(LOCALE_USER_DEFAULT,0,&stStart,NULL,szEDate,255);

	_stprintf(szEndTime,_T("%s %s"),szEDate,szETime);
	_stprintf(szStartTime,_T("%s %s"),szSDate,szSTime);

	szBody += _T("at time ");
	szBody += szStartTime;

	szBody += _T(",End time:");
	szBody += szEndTime;


	long lDuration = GetFileTimeIntervalBySeconds(call.ftEndTime,call.ftStartTime);

	CString sDuration;
	if (lDuration > 1)
	{
		FormatTimeString(lDuration,sDuration.GetBuffer(255));
		sDuration.ReleaseBuffer(-1);
	}
	else
	{
		sDuration.Format(_T(": %d sec"),lDuration);
	}

	szBody += _T(",Duration");
	szBody += sDuration;
	szBody += _T("\r\n");

	szSubject += sDuration;
}


HRESULT CallLogHelper::SetCallCategories(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData)
{
	if (!m_pFolder)
	{
		PEWriteLog(_T("SetCallCategories:m_pFolder is null"));
		return MAPI_E_INVALID_PARAMETER;
	}

	HRESULT hRes = S_OK;

	MAPINAMEID  rgnmid[1] = {0};
	LPMAPINAMEID rgpnmid[1] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&PS_PUBLIC_STRINGS;
	rgnmid[0].ulKind = MNID_STRING;
	rgnmid[0].Kind.lpwstrName = _T("Keywords");
	rgpnmid[0] = &rgnmid[0];


	hRes = m_pFolder->GetIDsFromNames(1,(LPMAPINAMEID*) &rgpnmid,NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("SetCallCategories:GetIDsFromNames failed"));
		return E_FAIL;
	}

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag           = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_MV_STRING8);
	spvProps[0].Value.MVszA.cValues = 1;
	spvProps[0].Value.MVszA.lppszA = new CHAR*[1];

	switch(callData.ulCallType)
	{
	case CALL_TYPE_MISSED_CALL:
		{
			spvProps[0].Value.MVszA.lppszA[0] ="Missed Calls";
			break;
		}
	case CALL_TYPE_INCOMING_CALL:
		{
			spvProps[0].Value.MVszA.lppszA[0] = "InComing Calls";
			break;
		}
	case CALL_TYPE_OUTGOING_CALL:
		{
			spvProps[0].Value.MVszA.lppszA[0] = "OutGoing Calls";
			break;
		}
	case CALL_TYPE_VOICE_MAIL:
		{
			spvProps[0].Value.MVszA.lppszA[0] = "VoiceMail";
			break;
		}
	case CALL_TYPE_REJECTED:
		{
			spvProps[0].Value.MVszA.lppszA[0] = "Rejected Calls";
			break;
		}
	case CALL_TYPE_REFUSED:
		{
			spvProps[0].Value.MVszA.lppszA[0] = "Refused Calls";
			break;
		}
	default:
		{
			spvProps[0].Value.MVszA.lppszA[0] = "Private Calls";
			break;
		}
	}

	hRes = lpMessage->SetProps(1, spvProps, NULL);
	MAPIFreeBuffer(lpNamedPropTags);
	if (FAILED(hRes))
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("SetCallCategories failed hRes is %d"),hRes);
		PEWriteLog(szInfo);
	}

	return hRes;
}




HRESULT CallLogHelper::SetCallContact(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData)
{
	if (!m_pFolder)
	{
		PEWriteLog(_T("SetCallContact:m_pFolder is null"));
		return MAPI_E_INVALID_PARAMETER;
	}

	if (_tcscmp(callData.szNameW,callData.szNumberW) == 0 )
	{
		return MAPI_E_INVALID_PARAMETER;
	}

	HRESULT hRes = S_OK;

	MAPINAMEID  rgnmid[1] = {0};
	LPMAPINAMEID rgpnmid[1] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&PSETID_Common1;
	rgnmid[0].ulKind = MNID_ID;
	rgnmid[0].Kind.lID = PidLidContacts;
	rgpnmid[0] = &rgnmid[0];


	hRes = m_pFolder->GetIDsFromNames(1,(LPMAPINAMEID*) &rgpnmid,NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("SetCallContact:GetIDsFromNames failed"));
		return E_FAIL;
	}

	SPropValue spvProps[1] = {0};

	spvProps[0].ulPropTag           = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_MV_UNICODE);//PR_JOURNAL_START_TIME;
	spvProps[0].Value.MVszW.cValues = 1;
	spvProps[0].Value.MVszW.lppszW = new TCHAR*[1];
	spvProps[0].Value.MVszW.lppszW[0] = callData.szNameW;

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(1, spvProps, NULL);
	if (!SUCCEEDED(hRes))
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("SetCallContact:SetProps hRes is %d"),hRes);
		PEWriteLog(szInfo);
		PEWriteLog(_T("SetCallContact:SetProps failed"));
	}

	return hRes;
}





HRESULT CallLogHelper::SetCallPropertyEx(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData)
{
	if (!m_pFolder)
	{
		PEWriteLog(_T("AddDiary:m_pFolder is null"));
		return MAPI_E_INVALID_PARAMETER;
	}

	SetCallCategories(lpMessage,callData);

	HRESULT hRes = S_OK;

	MAPINAMEID  rgnmid[ulPhoneCallProps] = {0};
	LPMAPINAMEID rgpnmid[ulPhoneCallProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulPhoneCallProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Log;
		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulPhoneCallProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(
		ulPhoneCallProps,
		(LPMAPINAMEID*) &rgpnmid,
		NULL,
		&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return E_FAIL;
	}


	SYSTEMTIME stTime;
	DATE startTime,endTime;
	FileTimeToSystemTime(&callData.ftStartTime,&stTime);
	SystemTimeToVariantTime(&stTime,&startTime);

	FileTimeToSystemTime(&callData.ftEndTime,&stTime);
	SystemTimeToVariantTime(&stTime,&endTime);

	ULONG ulDuration = GetFileTimeIntervalBySeconds(callData.ftEndTime,callData.ftStartTime);
	ulDuration /= 60;
	CString szBody,szSubject;
	FormatCallLog(callData,szBody,szSubject);

	SPropValue spvProps[7] = {0};
	SPropValue spvPropsExt[3] = {0};
	spvProps[0].ulPropTag           = PR_SUBJECT;
	spvProps[1].ulPropTag           = PR_BODY;
	spvProps[2].ulPropTag           = PR_MESSAGE_CLASS;
	spvProps[3].ulPropTag           = PR_SENDER_NAME;
	spvProps[4].ulPropTag           = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_SYSTIME);//PR_JOURNAL_START_TIME;
	spvProps[5].ulPropTag           = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[1],PT_SYSTIME);//PR_JOURNAL_END_TIME;
	spvProps[6].ulPropTag           = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[2],PT_LONG);//PR_JOURNAL_CALL_DURATION;

	spvPropsExt[0].ulPropTag        = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[3],PT_UNICODE);//PR_JOURNAL_TYPE_DES;//type
	spvPropsExt[1].ulPropTag        = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[4],PT_UNICODE);//PR_JOURNAL_TYPE;//type

	spvPropsExt[2].ulPropTag		= PR_ICON_INDEX;
	//spvProps[9].ulPropTag			= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[5],PT_MV_UNICODE);//PR_JOURNAL_CONTACTS;

	spvProps[0].Value.lpszW				= szSubject.GetBuffer();
	spvProps[1].Value.lpszW				= szBody.GetBuffer();
	spvProps[2].Value.lpszW				= _T("IPM.Activity");
	spvProps[3].Value.lpszW				= callData.szNameW;

	spvProps[4].Value.ft				= callData.ftStartTime;
	spvProps[5].Value.ft				= callData.ftEndTime;
	spvProps[6].Value.ul		        = ulDuration;

	spvPropsExt[0].Value.lpszW				= _T("Phone Call");
	spvPropsExt[1].Value.lpszW				= _T("Phone call");

	spvPropsExt[2].Value.l			        = 1546;

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(7, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		hRes = lpMessage->SetProps(3, spvPropsExt, NULL);
		if (FAILED(hRes))
		{
			TCHAR szInfo[255];
			_stprintf(szInfo,_T("AddDiary:SetProps failed 1 hRes is %d"),hRes);
			PEWriteLog(szInfo);
		}

		hRes = lpMessage->SaveChanges(FORCE_SAVE);
		if (FAILED(hRes))
		{
			PEWriteLog(_T("AddDiary:SaveChanges failed"));
			TCHAR szInfo[255];
			_stprintf(szInfo,_T("AddDiary:SaveChanges hRes is %d"),hRes);
			PEWriteLog(szInfo);
		}
	}
	else
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("AddDiary:SetProps hRes is %d"),hRes);
		PEWriteLog(szInfo);
		PEWriteLog(_T("AddDiary:SetProps failed"));
	}

	return hRes;
}


int CallLogHelper::GetCallPropertyEx(LPMESSAGE lpMessage,CALL_LOG_ITEM& callData)
{
	HRESULT hRes = S_OK;

	MAPINAMEID  rgnmid[ulPhoneCallProps];
	LPMAPINAMEID rgpnmid[ulPhoneCallProps];
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulPhoneCallProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Log;
		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulPhoneCallProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(
		ulPhoneCallProps,
		(LPMAPINAMEID*) &rgpnmid,
		NULL,
		&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return PE_RET_FAIL;
	}

	ULONG ulStartTImeTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_SYSTIME);//PR_JOURNAL_START_TIME;
	ULONG ulEndTImeTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[1],PT_SYSTIME);//PR_JOURNAL_END_TIME;
	ULONG ulDurationTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[2],PT_LONG);//PR_JOURNAL_CALL_DURATION;

	MAPIFreeBuffer(lpNamedPropTags);

	SizedSPropTagArray(6, mcols) = {6,{PR_SUBJECT,PR_BODY,PR_SENDER_NAME,ulStartTImeTag,ulEndTImeTag,ulDurationTag}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (pcount != 6)
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag != PR_SUBJECT
		||props[1].ulPropTag != PR_BODY
		||props[2].ulPropTag != PR_SENDER_NAME
		||props[3].ulPropTag != ulStartTImeTag
		||props[4].ulPropTag != ulEndTImeTag
		||props[5].ulPropTag != ulDurationTag)
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	_tcscpy(callData.szNameW,props[2].Value.lpszW);
	callData.ftStartTime = props[3].Value.ft;
	callData.ftEndTime = props[4].Value.ft;

	MAPIFreeBuffer(props);
	return PE_RET_OK;;
}


int CallLogHelper::GenCallProperty(LPMESSAGE lpMessage)
{
	MAPINAMEID  rgnmid[3] = {0};
	LPMAPINAMEID rgpnmid[3] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&MOBITNT_ITEM_ID_GUID;
	rgnmid[0].ulKind = MNID_STRING;
	rgnmid[0].Kind.lpwstrName = _T("MOBITNT_ITEM_ID");
	rgpnmid[0] = &rgnmid[0];

	rgnmid[1].lpguid = (LPGUID)&MOBITNT_CALL_NUMBER_GUID;
	rgnmid[1].ulKind = MNID_STRING;
	rgnmid[1].Kind.lpwstrName = _T("MOBITNT_CALL_NUMBERDIARY");
	rgpnmid[1] = &rgnmid[1];

	rgnmid[2].lpguid = (LPGUID)&MOBITNT_RECV_TYPE_GUID;
	rgnmid[2].ulKind = MNID_STRING;
	rgnmid[2].Kind.lpwstrName = _T("MOBITNT_RECV_TYPE");
	rgpnmid[2] = &rgnmid[2];
	HRESULT hRes = lpMessage->GetIDsFromNames(3,(LPMAPINAMEID*) &rgpnmid,MAPI_CREATE,&lpNamedPropTags);
	if (FAILED(hRes))
	{
		return PE_RET_FAIL;
	}

	m_lTag4ItemID		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[0],PT_LONG);
	m_lTag4CallNumber	= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[1],PT_UNICODE);
	m_lTag4RecvType		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[2],PT_LONG);

	MAPIFreeBuffer(lpNamedPropTags);
	return PE_RET_OK;
}


bool CallLogHelper::GetCallProperty(LPMESSAGE lpMessage,CALL_LOG_ITEM& call)
{
	SizedSPropTagArray(3, mcols) = {3,{m_lTag4ItemID,m_lTag4CallNumber, m_lTag4RecvType}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	if (pcount != 3)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	if (props[0].ulPropTag != m_lTag4ItemID 
		|| props[1].ulPropTag != m_lTag4CallNumber
		|| props[2].ulPropTag != m_lTag4RecvType
		)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	call.ulCallID = props[0].Value.l;
	_tcscpy(call.szNumberW,props[1].Value.lpszW);
	call.ulCallType = props[2].Value.l;


	MAPIFreeBuffer(props);

	return true;
}


bool CallLogHelper::SetCallProperty(LPMESSAGE lpMessage,CALL_LOG_ITEM& call)
{
	SPropValue spvProps[3] = {0};

	spvProps[0].ulPropTag = m_lTag4ItemID;
	spvProps[1].ulPropTag = m_lTag4CallNumber;
	spvProps[2].ulPropTag = m_lTag4RecvType;

	spvProps[0].Value.l = call.ulCallID;
	spvProps[1].Value.lpszW = call.szNumberW;
	spvProps[2].Value.l = call.ulCallType;

	HRESULT hRes = lpMessage->SetProps(3, spvProps, NULL);
	if (FAILED(hRes))
	{
		return false;
	}

	return true;
}



int CallLogHelper::Export()
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
		return PE_RET_FAIL;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		return PE_RET_FAIL;
	}

	hr = S_OK;
	ULONG iCount = 0;
	pTable->GetRowCount(0,&iCount);

	PECore::SendRestoreState2UI(PE_ACT_SAVE_CALL_LOG_2_PHONE,(float)iCount,0);
	CALL_LOG_ITEM call;
	//int iTotal = 1;
	for (ULONG i = 1; i <= iCount; ++i)
	{
		hr = pTable->QueryRows(1,0, &pRows);
		if (FAILED(hr))
		{
			continue;
		}

		PECore::SendRestoreState2UI(PE_ACT_SAVE_CALL_LOG_2_PHONE,(float)iCount,(float)i);
		if (pRows->cRows != 1)
		{
			pTable->Release();
			pTable = NULL;
			return PE_RET_FAIL;
		}

		if (!g_iCanSync)
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			PECore::SendRestoreState2UI(PE_ACT_SAVE_CALL_LOG_2_PHONE,100,100);
			break;
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
			return PE_RET_FAIL;
		}

		if (PE_RET_OK != GenCallProperty(pMsg) )
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			pMsg->Release();
			return PE_RET_FAIL;
		}

		memset(&call,0,sizeof(call));
		if (!GetCallProperty(pMsg,call))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (PE_RET_OK == GetCallPropertyEx(pMsg,call))
		{
			DeviceAgent::RestoreCall(call);
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	PECore::SendRestoreState2UI(PE_ACT_SAVE_CALL_LOG_2_PHONE,100,100);

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}

int CallLogHelper::SaveCallLog2OutLook(CALL_LOG_ITEM& call)
{
	LPMESSAGE lpMessage = 0;
	HRESULT hRes = m_pFolder->CreateMessage(0,0,&lpMessage);
	if (FAILED(hRes))
	{
		PEWriteLog(_T("AddDiary:createMessage failed"));
		return PE_RET_FAIL;
	}

	GenCallProperty(lpMessage);
	SetCallProperty(lpMessage,call);

	SetCallContact(lpMessage,call);

	SetCallPropertyEx(lpMessage,call);

	lpMessage->Release();

	return PE_RET_OK;
}


void CallLogHelper::ApplyFilter()
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
		return;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		return;
	}

	hr = S_OK;
	ULONG iCount = 0;
	pTable->GetRowCount(0,&iCount);

	CALL_LOG_ITEM call;
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
			return;
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
			return;
		}

		if (PE_RET_OK != GenCallProperty(pMsg) )
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			pMsg->Release();
			return;
		}

		memset(&call,0,sizeof(call));
		if (!GetCallProperty(pMsg,call))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if ( PE_RET_OK == FilterHelper::IsItemInFilterList(call.szNumberW) )
		{
			DeleteItemByEntryID(pRows->aRow[0].lpProps[0].Value.bin);
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return;
}


int CallLogHelper::FindCallItemByEntryID(TCHAR *tszEntryID,CALL_LOG_ITEM& call)
{
	SBinary bin;
	TCHAR2SBinary(tszEntryID,bin);
	LPMESSAGE pMsg = FindItemByEntryID(bin);
	if (pMsg == NULL)
	{
		return PE_RET_FAIL;
	}

	GenCallProperty(pMsg);
	memset(&call,0,sizeof(call));
	if (!GetCallProperty(pMsg,call))
	{
		pMsg->Release();
		return PE_RET_FAIL;
	}

	if (PE_RET_OK != GetCallPropertyEx(pMsg,call))
	{
		pMsg->Release();
		return PE_RET_FAIL;
	}

	pMsg->Release();
	return PE_RET_OK;
}


void CallLogHelper::RemoveDupliacte()
{

}


int CallLogHelper::GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft)
{
	CALL_LOG_ITEM call;

#if 1
	memset(&call,0,sizeof(call));
	GenCallProperty(pMsg);

	if (!GetCallProperty(pMsg,call))
	{
		return PE_RET_FAIL;
	}

#endif

	if (GetCallPropertyEx(pMsg,call) != PE_RET_OK)
	{
		return PE_RET_FAIL;
	}

	memcpy(&ft,&(call.ftStartTime),sizeof(ft));
	return PE_RET_OK;
}



list<TCHAR*> CallLogHelper::GetSubFolderList()
{
	PEWriteLog(_T("Enter"));

	list<TCHAR*> subFolderList;
	subFolderList.clear();

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenSpecialFolder(PR_IPM_JOURNAL_ENTRYID,FALSE);
	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("OpenContacts failed!"));
		return subFolderList;
	}

	TCHAR *szRoot = (TCHAR*)calloc(255,sizeof(TCHAR));
	m_pMAPIMgr->GetFolderName(szRoot,pRootFolder);

	subFolderList = m_pMAPIMgr->GetSubFolderList(pRootFolder,szRoot);
	subFolderList.push_back(szRoot);

	pRootFolder->Release();

	return subFolderList;

}


int CallLogHelper::GenStat(FILETIME &ftStart,FILETIME &ftEnd,CALL_LOG_STAT *pStat)
{

	HRESULT hr = S_OK;
	ULONG ulFolderID = 0;

	LPMAPITABLE  pTable = NULL;
	LPMAPITABLE  pTmpTable = NULL;
	LPSRowSet pRows = NULL;
	LPSRowSet pTmpRows = NULL;
	ULONG	ulNumCols = 1;
	//指定我们需要获取Entry ID属性
	SizedSPropTagArray(1, Columns) = 
	{ 
		ulNumCols,
		PR_ENTRYID	
	};

	hr = m_pFolder->GetContentsTable(0, &pTable);
	if ((FAILED(hr)))
	{
		return PE_RET_FAIL;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		return PE_RET_FAIL;
	}

	hr = S_OK;
	ULONG iCount = 0;
	pTable->GetRowCount(0,&iCount);

	CALL_LOG_ITEM call;
	for (ULONG i = 1; i <= iCount; ++i)
	{
		hr = pTable->QueryRows(1,0, &pRows);
		if (FAILED(hr))
		{
			continue;
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
			return PE_RET_FAIL;
		}

		if (PE_RET_OK != GenCallProperty(pMsg) )
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			pMsg->Release();
			return PE_RET_FAIL;
		}

		memset(&call,0,sizeof(call));
		if (!GetCallProperty(pMsg,call))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (GetCallPropertyEx(pMsg,call))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (_tcscmp(pStat->szPhone,call.szNumberW) == 0)
		{
			switch(call.ulCallType)
			{
			case CALL_TYPE_MISSED_CALL:
				{
					++(pStat->ulMissedCount);
					break;
				}
			case CALL_TYPE_INCOMING_CALL:
				{
					++(pStat->ulInCount);
					pStat->ulInTime += GetFileTimeIntervalBySeconds(call.ftEndTime,call.ftStartTime);
					break;
				}
			case CALL_TYPE_OUTGOING_CALL:
				{
					++(pStat->ulOutCount);
					pStat->ulOutTime += GetFileTimeIntervalBySeconds(call.ftEndTime,call.ftStartTime);
					break;
				}
			}
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}




