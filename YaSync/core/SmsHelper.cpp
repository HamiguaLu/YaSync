
#include "stdafx.h"

#include "SmsHelper.h"
#include "PECore.h"
#include "AppTools.h"
#include "mapiutil.h"
#include "PEInterface.h"
#include <initguid.h>

#include "TrayHelper.h"
#include "SettingHelper.h"

extern int g_iCanSync;

#define HTML_TEMPLATE_HEADER	_T("<html><style type=\"text/css\"><!--.Msg {border:solid 1px blue;padding:10px;margin:10px;}--></style><body><div class=\"Msg\">")
#define HTML_TEMPLATE_TAIL		_T("</div></body></html>")


int SmsHelper::SaveSMS2OutLook(SMS_ITEM& smsMsg)
{
	LPMESSAGE pMsg = NULL;
	HRESULT hr = m_pFolder->CreateMessage(NULL,0,&pMsg);
	if (hr != S_OK)
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("SaveSMS2OutLook:CreateMessage failed: %x"),hr);
		PEWriteLog(szInfo);
		return PE_RET_FAIL;
	}

	if (PE_RET_OK != GenSmsProperty(pMsg))
	{
		pMsg->Release();
		PEWriteLog(_T("SaveSMS2OutLook:GenSmsProperty failed"));
		return PE_RET_FAIL; 
	}

	if (!SetSmsProperty(pMsg,smsMsg))
	{
		pMsg->Release();
		PEWriteLog(_T("SaveSMS2OutLook:SetSmsProperty failed"));
		return PE_RET_FAIL; 
	}

	hr = SetSmsPropertyEx(pMsg,smsMsg);
	if (FAILED(hr))
	{
		PEWriteLog(_T("SaveSMS2OutLook:SetSmsPropertyEx failed"));
		pMsg->Release();
		return PE_RET_FAIL; 
	}

	if (smsMsg.ulReadFlag == SMS_READ_FLAG_UNREAD)
	{
		pMsg->SetReadFlag(CLEAR_READ_FLAG);
	}

	hr = pMsg->SaveChanges(FORCE_SAVE);
	pMsg->Release();

	if (FAILED(hr))
	{
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}

int SmsHelper::GenSmsProperty(LPMESSAGE lpMessage)
{
	MAPINAMEID  rgnmid[4] = {0};
	LPMAPINAMEID rgpnmid[4] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&MOBITNT_ITEM_ID_GUID;
	rgnmid[0].ulKind = MNID_STRING;
	rgnmid[0].Kind.lpwstrName = _T("MOBITNT_ITEM_ID");
	rgpnmid[0] = &rgnmid[0];

	rgnmid[1].lpguid = (LPGUID)&MOBITNT_THREAD_ID_GUID;
	rgnmid[1].ulKind = MNID_STRING;
	rgnmid[1].Kind.lpwstrName = _T("MOBITNT_THREAD_ID");
	rgpnmid[1] = &rgnmid[1];

	rgnmid[2].lpguid = (LPGUID)&MOBITNT_RECV_TYPE_GUID;
	rgnmid[2].ulKind = MNID_STRING;
	rgnmid[2].Kind.lpwstrName = _T("MOBITNT_RECV_TYPE");
	rgpnmid[2] = &rgnmid[2];

	rgnmid[3].lpguid = (LPGUID)&MOBITNT_SMS_PHONE_NO_GUID;
	rgnmid[3].ulKind = MNID_STRING;
	rgnmid[3].Kind.lpwstrName = _T("MOBITNT_SMS_PHONE_NO_GUID");
	rgpnmid[3] = &rgnmid[3];

	HRESULT hRes = lpMessage->GetIDsFromNames(4,(LPMAPINAMEID*) &rgpnmid,MAPI_CREATE,&lpNamedPropTags);
	if (FAILED(hRes))
	{
		return PE_RET_FAIL;
	}

	m_lTag4ItemID		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[0],PT_LONG);
	m_lTag4ThreadID		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[1],PT_LONG);
	m_lTag4RecvType		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[2],PT_LONG);
	m_lTag4PhoneNo		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[3],PT_UNICODE);
	MAPIFreeBuffer(lpNamedPropTags);
	return PE_RET_OK;
}





HRESULT SmsHelper::SetSmsPropertyEx(LPMESSAGE lpMessage,SMS_ITEM& sms)
{
	HRESULT hr = S_OK;
	int iLen = 0;
	CString sSubject = _T("");
	if (sms.pszBody)
	{
		sSubject = sms.pszBody;
	}

	if (sSubject.GetLength() > 30)
	{
		sSubject = sSubject.Left(30);
		sSubject += _T("...");
	}

	SPropValue spvProps[11] = {0};
#if 1
	spvProps[0].ulPropTag = PR_SUBJECT_W;
	spvProps[0].Value.lpszW = sSubject.GetBuffer();
#else
	iLen = sSubject.GetLength();
	char *szSubject = (char *)calloc(iLen*3, 1);
	WideCharToMultiByte(CP_UTF8,0,sSubject.GetBuffer(),-1,szSubject,iLen*3,0,0);
	sSubject.ReleaseBuffer();
	spvProps[0].ulPropTag = PR_SUBJECT_A;
	spvProps[0].Value.lpszA = szSubject;
#endif

	spvProps[1].ulPropTag = PR_MESSAGE_CLASS_W;
	spvProps[1].Value.lpszW = L"IPM.Note";

	spvProps[2].ulPropTag = PR_BODY_W;
	spvProps[2].Value.lpszW = sms.pszBody;
	
	CString sBody = HTML_TEMPLATE_HEADER;
	sBody += sms.pszBody;
	sBody += HTML_TEMPLATE_TAIL;

	iLen = WideCharToMultiByte(CP_UTF8,0,sBody.GetBuffer(),-1,0,0,0,0);
	char *szData = (char *)calloc(iLen, 1);
	WideCharToMultiByte(CP_UTF8,0,sBody.GetBuffer(),-1,szData,iLen,0,0);
	sBody.ReleaseBuffer();

	spvProps[3].ulPropTag = PR_HTML;
	spvProps[3].Value.bin.cb = (ULONG)iLen;
	spvProps[3].Value.bin.lpb = (LPBYTE)szData;
	
	spvProps[4].ulPropTag = PR_MESSAGE_DELIVERY_TIME;
	spvProps[4].Value.ft = sms.ftRecvTime;
	spvProps[5].ulPropTag = PR_CLIENT_SUBMIT_TIME;
	spvProps[5].Value.ft = sms.ftRecvTime;

	CString sTo = _T("Me");
	if (sms.ulMsgType == SMS_TYPE_SENT)
	{
		if (_tcscmp(sms.tszName,sms.tszPhone) == 0)
		{
			sTo.Format(_T("To <%s>"),sms.tszPhone);
		}
		else if (_tcslen(sms.tszName) && _tcslen(sms.tszPhone))
		{
			sTo.Format(_T("To %s<%s>"),sms.tszName,sms.tszPhone);
		}
		else if (_tcslen(sms.tszName))
		{
			sTo.Format(_T("To %s"),sms.tszName);
		}
		else if (_tcslen(sms.tszPhone))
		{
			sTo.Format(_T("To <%s>"),sms.tszPhone);
		}

		spvProps[6].ulPropTag = PR_RCVD_REPRESENTING_NAME_W;
		spvProps[6].Value.lpszW = sms.tszName;
	}
	else
	{
		if (_tcscmp(sms.tszName,sms.tszPhone) == 0)
		{
			sTo.Format(_T("<%s>"),sms.tszPhone);
		}
		else if (_tcslen(sms.tszName) && _tcslen(sms.tszPhone))
		{
			sTo.Format(_T("%s<%s>"),sms.tszName,sms.tszPhone);
		}
		else if (_tcslen(sms.tszName))
		{
			sTo.Format(_T("%s"),sms.tszName);
		}
		else if (_tcslen(sms.tszPhone))
		{
			sTo.Format(_T("<%s>"),sms.tszPhone);
		}
		
		spvProps[6].ulPropTag = PR_RCVD_REPRESENTING_NAME_W;
		spvProps[6].Value.lpszW = _T("Me");
	}

	spvProps[7].ulPropTag = PR_SENDER_EMAIL_ADDRESS_W;
	spvProps[7].Value.lpszW =(LPWSTR)(LPCWSTR)sTo;
	spvProps[8].ulPropTag = PR_SENDER_NAME_W;
	spvProps[8].Value.lpszW = (LPWSTR)(LPCWSTR)sTo;
	spvProps[9].ulPropTag = PR_SENT_REPRESENTING_NAME_W;
	spvProps[9].Value.lpszW = (LPWSTR)(LPCWSTR)sTo;
	spvProps[10].ulPropTag =  PR_INTERNET_CPID;
	spvProps[10].Value.l = CP_UTF8;

	hr = lpMessage->SetProps(11,spvProps,NULL);
	if (FAILED(hr))
	{
		PEWriteLog(_T("SetMessageProp:SetProps failed"));
	}

	free(szData);
	sSubject.ReleaseBuffer();
	return hr;
}


bool SmsHelper::GetSmsPropertyEx(LPMESSAGE lpMessage,SMS_ITEM& sms)
{
	SizedSPropTagArray(4, mcols) = {4,{PR_BODY_W,PR_SENT_REPRESENTING_NAME_W,PR_SENDER_EMAIL_ADDRESS_W,PR_MESSAGE_DELIVERY_TIME}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	if (pcount != 4)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	if (props[0].ulPropTag != PR_BODY_W
		||props[1].ulPropTag != PR_SENT_REPRESENTING_NAME_W
		||props[2].ulPropTag != PR_SENDER_EMAIL_ADDRESS_W
		||props[3].ulPropTag != PR_MESSAGE_DELIVERY_TIME
		)
	{
		MAPIFreeBuffer(props);
		return false;
	}

#if 1
	if (props[0].Value.lpszW && _tcslen(props[0].Value.lpszW))
	{
		sms.pszBody = (TCHAR*)calloc( _tcslen(props[0].Value.lpszW) + 1,sizeof(TCHAR));
		_tcscpy(sms.pszBody, props[0].Value.lpszW);
	}
	else
	{
		sms.pszBody = NULL;
	}
#else
	int iHtmlLen = props[0].Value.bin.cb;
	if (iHtmlLen)
	{
		char *szData = (char*)props[0].Value.bin.lpb;
		int iLen = MultiByteToWideChar(CP_UTF8,0,szData,-1,0,0);
		if (iLen > 0)
		{
			sms.pszBody = (TCHAR*)calloc(iLen,sizeof(TCHAR));
			MultiByteToWideChar(CP_UTF8,0,szData,-1,sms.pszBody,iLen);
			CString sData = sms.pszBody;
			sData.Replace(HTML_TEMPLATE_HEADER,_T(""));
			sData.Replace(HTML_TEMPLATE_TAIL,_T(""));
			_tcscpy(sms.pszBody,sData.GetBuffer());
			sData.ReleaseBuffer();
		}
	}
#endif

	_tcscpy(sms.tszName,  props[1].Value.lpszW);
	//_tcscpy(sms.tszPhone, props[2].Value.lpszW);
	sms.ftRecvTime = props[3].Value.ft;
	//_tcscpy(sms.tszsBody,(TCHAR*)props[2].Value.bin.lpb);

	MAPIFreeBuffer(props);
	return true;
}

bool SmsHelper::SetSmsProperty(LPMESSAGE lpMessage,SMS_ITEM& sms)
{
	SPropValue spvProps[4] = {0};

	spvProps[0].ulPropTag = m_lTag4ItemID;
	spvProps[0].Value.l = sms.ulMsgID;

	spvProps[1].ulPropTag = m_lTag4ThreadID;
	spvProps[1].Value.l = sms.ulThreadID;

	spvProps[2].ulPropTag = m_lTag4RecvType;
	spvProps[2].Value.l = sms.ulMsgType;

	spvProps[3].ulPropTag = m_lTag4PhoneNo;
	spvProps[3].Value.lpszW = sms.tszPhone;

	HRESULT hRes = lpMessage->SetProps(4, spvProps, NULL);
	if (FAILED(hRes))
	{
		return false;
	}

	return true;
}

bool SmsHelper::GetSmsProperty(LPMESSAGE lpMessage,SMS_ITEM& sms)
{
	SizedSPropTagArray(4, mcols) = {4,{m_lTag4ItemID,m_lTag4ThreadID,m_lTag4RecvType,m_lTag4PhoneNo}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	if (pcount != 4)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	if (props[0].ulPropTag != m_lTag4ItemID
		||props[1].ulPropTag != m_lTag4ThreadID
		||props[2].ulPropTag != m_lTag4RecvType
		)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	sms.ulMsgID = props[0].Value.l;
	sms.ulThreadID = props[1].Value.l;
	sms.ulMsgType = props[2].Value.l;

	if (props[3].ulPropTag == m_lTag4PhoneNo)
	{
		_tcscpy(sms.tszPhone, props[3].Value.lpszW);
	}

	MAPIFreeBuffer(props);

	return true;
}

int SmsHelper::GetMaxMsgID(ULONG &ulMaxMsgID)
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

	SMS_ITEM sms;

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
			return PE_RET_FAIL;
		}

		if (PE_RET_OK != GenSmsProperty(pMsg) )
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			pMsg->Release();
			return PE_RET_FAIL;
		}

		memset(&sms,0,sizeof(sms));

		if (!GetSmsProperty(pMsg,sms))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (sms.ulMsgID > ulMaxMsgID)
		{
			ulMaxMsgID = sms.ulMsgID;
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}

int SmsHelper::Export()
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

	PECore::SendRestoreState2UI(PE_ACT_SAVE_SMS_2_PHONE,(float)iCount,0);

	SMS_ITEM sms;

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
			PECore::SendRestoreState2UI(PE_ACT_SAVE_SMS_2_PHONE,100,100);
			return PE_RET_FAIL;
		}

		if (!g_iCanSync)
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			PECore::SendRestoreState2UI(PE_ACT_SAVE_SMS_2_PHONE,100,100);
			break;
		}

		PECore::SendRestoreState2UI(PE_ACT_SAVE_SMS_2_PHONE,(float)iCount,(float)i);

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

		if (PE_RET_OK != GenSmsProperty(pMsg) )
		{
			pMsg->Release();
			FreeProws(pRows);
			pTable->Release();
			return PE_RET_FAIL;
		}

		memset(&sms,0,sizeof(sms));
		if (!GetSmsProperty(pMsg,sms))
		{
			pMsg->Release();
			FreeProws(pRows);
			free(sms.pszBody);
			continue;
		}

		if (GetSmsPropertyEx(pMsg,sms))
		{
			DeviceAgent::RestoreSMS(sms);
		}

		free(sms.pszBody);

		pMsg->Release();
		FreeProws(pRows);
	}

	PECore::SendRestoreState2UI(PE_ACT_SAVE_SMS_2_PHONE,100,100);

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}

list<SMS_ITEM*> SmsHelper::GetSmsListByThreadID(TCHAR *tszFolderID,ULONG ulThreadID,TCHAR *szPhoneID)
{
	list<SMS_ITEM*> smsList;

	SBinary bin;
	TCHAR2SBinary(tszFolderID,bin);
	LPMAPIFOLDER lpFolder = m_pMAPIMgr->OpenFolderByEntryID(bin);
	if (lpFolder == NULL)
	{
		return smsList;
	}

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

	hr = lpFolder->GetContentsTable(0, &pTable);
	if ((FAILED(hr)))
	{
		lpFolder->Release();
		return smsList;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		lpFolder->Release();
		return smsList;
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
			return smsList;
		}

		//通过OpenEntry获取IMessage对象
		LPMESSAGE pMsg = NULL;
		ULONG ulMesageType;
		hr = lpFolder->OpenEntry( pRows->aRow[0].lpProps[0].Value.bin.cb,
			(LPENTRYID)pRows->aRow[0].lpProps[0].Value.bin.lpb,
			NULL, 
			MAPI_BEST_ACCESS, 
			&ulMesageType,
			(LPUNKNOWN*)&pMsg);
		if ((FAILED(hr)))
		{
			FreeProws(pRows);
			pTable->Release();
			lpFolder->Release();
			return smsList;
		}

		if (PE_RET_OK != GenSmsProperty(pMsg) )
		{
			pMsg->Release();
			FreeProws(pRows);
			pTable->Release();
			lpFolder->Release();
			return smsList;
		}

		SMS_ITEM *pSms = (SMS_ITEM *)malloc(sizeof(SMS_ITEM));

		memset(pSms,0,sizeof(SMS_ITEM));
		if (!GetSmsProperty(pMsg,*pSms))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (pSms->ulThreadID != ulThreadID )
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (GetSmsPropertyEx(pMsg,*pSms))
		{
			smsList.push_back(pSms);
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	lpFolder->Release();

	return smsList;
}

void SmsHelper::ApplyFilter()
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

	SMS_ITEM sms;
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

		if (PE_RET_OK != GenSmsProperty(pMsg) )
		{
			pMsg->Release();
			FreeProws(pRows);
			pTable->Release();
			return;
		}

		memset(&sms,0,sizeof(sms));
		if (!GetSmsProperty(pMsg,sms))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (!GetSmsPropertyEx(pMsg,sms))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if ( PE_RET_OK == FilterHelper::IsItemInFilterList(sms.tszPhone) )
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


BOOL SmsHelper::openFolder()
{
	m_pFolder = NULL;
	m_iDefaultFolder = 1;
		
	PE_PHONE_SETTING *s = PECore::GetCurPhone();
	if (s && _tcslen(s->szSMSFolderName) > 0)
	{
		PEWriteLog(_T("SMS Folder Name:"));
		PEWriteLog(s->szSMSFolderName);
		m_pFolder = m_pMAPIMgr->OpenFolderByPath(s->szSMSFolderName);
		if (m_pFolder)
		{
			return TRUE;			
		}

		PEWriteLog(_T("Can not open folder for SMS"));
		return FALSE;
	}

	TCHAR szPhoneName[255];
	if (PECore::GetCurPhoneName(szPhoneName) != PE_RET_OK)
	{
		return FALSE;
	}

	TCHAR szFolderName[1024];
	GenFolderName(szFolderName,_T("SMS"),szPhoneName);
	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenRootFolder(FALSE);
	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("can not open root foler for SMS"));
		return FALSE;
	}

	LPMAPIFOLDER pWMSMSFolder = NULL;
	HRESULT hr = pRootFolder->CreateFolder(	FOLDER_GENERIC,
		szFolderName,
		MOBITNT_SMS_FOLDER_COMMENT,
		NULL,
		MAPI_UNICODE|OPEN_IF_EXISTS,
		&pWMSMSFolder);
	pRootFolder->Release();
	if (FAILED(hr))
	{
		return FALSE;
	}

	hr = pWMSMSFolder->SaveChanges(0);
	if (FAILED(hr))
	{
		return FALSE;
	}

	m_pFolder = pWMSMSFolder;
	return TRUE;
}



int SmsHelper::FindSmsIemByEntryID(TCHAR *tszEntryID,SMS_ITEM& sms)
{
	SBinary bin;
	TCHAR2SBinary(tszEntryID,bin);
	LPMESSAGE pMsg = FindItemByEntryID(bin);
	if (pMsg == NULL)
	{
		return PE_RET_FAIL;
	}

	GenSmsProperty(pMsg);
	memset(&sms,0,sizeof(sms));
	if (!GetSmsProperty(pMsg,sms))
	{
		pMsg->Release();
		return PE_RET_FAIL;
	}

	GetSmsPropertyEx(pMsg,sms);

	pMsg->Release();
	return PE_RET_OK;

}


void SmsHelper::RemoveDupliacte()
{

}

int SmsHelper::GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft)
{
	SMS_ITEM sms;
	memset(&sms,0,sizeof(sms));

#if 0
	GenSmsProperty(pMsg);

	if (!GetSmsProperty(pMsg,sms))
	{
		return PE_RET_FAIL;
	}
#endif

	if (!GetSmsPropertyEx(pMsg,sms))
	{
		return PE_RET_FAIL;
	}

	memcpy(&ft,&sms.ftRecvTime,sizeof(ft));
	return PE_RET_OK;
}


list<TCHAR*> SmsHelper::GetSubFolderList()
{
	PEWriteLog(_T("Enter"));

	list<TCHAR*> subFolderList;
	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenRootFolder(0);
	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("OpenContacts failed!"));
		return subFolderList;
	}
#if 0
	TCHAR *szRoot = (TCHAR*)calloc(255,sizeof(TCHAR));
	m_pMAPIMgr->GetFolderName(szRoot,pRootFolder);
#endif

	subFolderList = m_pMAPIMgr->GetSubFolderList(pRootFolder,NULL);
	//subFolderList.push_back(szRoot);

	pRootFolder->Release();

	return subFolderList;

}

int SmsHelper::GenStat(FILETIME &ftStart,FILETIME &ftEnd,SMS_STAT *pStat)
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

	SMS_ITEM sms;
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
			return PE_RET_FAIL;
		}

		if (PE_RET_OK != GenSmsProperty(pMsg) )
		{
			pMsg->Release();
			FreeProws(pRows);
			pTable->Release();
			return PE_RET_FAIL;
		}

		memset(&sms,0,sizeof(sms));
		if (!GetSmsProperty(pMsg,sms))
		{
			pMsg->Release();
			FreeProws(pRows);
			free(sms.pszBody);
			continue;
		}

		if (!GetSmsPropertyEx(pMsg,sms))
		{
			pMsg->Release();
			FreeProws(pRows);
			free(sms.pszBody);
			continue;
		}

		if (_tcscmp(pStat->szPhone,sms.tszPhone) == 0)
		{
			switch(sms.ulMsgType)
			{
			case SMS_TYPE_RECEIVE:
				{
					++(pStat->ulInCount);
					break;
				}
			case SMS_TYPE_SENT:
				{
					++(pStat->ulOutCount);
					break;
				}
			}
		}

		free(sms.pszBody);

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;


}




