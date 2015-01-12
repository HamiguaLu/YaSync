
#include "stdafx.h"

#include "MmsHelper.h"
#include "PECore.h"
#include "AppTools.h"
#include "mapiutil.h"
#include "PEInterface.h"
#include <initguid.h>


int MmsHelper::SaveMMS2OutLook(MMS_ITEM* pMmsMsg)
{
	PEWriteLog(_T("Save mms"));

	SMS_ITEM *smsMsg = (SMS_ITEM*)calloc(1,sizeof(SMS_ITEM));
	GetSmsFromMms(pMmsMsg,smsMsg);

	LPMESSAGE pMsg = NULL;
	HRESULT hr = m_pFolder->CreateMessage(NULL,0,&pMsg);
	if (hr != S_OK)
	{
		PEWriteLog(_T("SaveSMS2OutLook:CreateMessage failed"));
		free(smsMsg);
		return PE_RET_FAIL;
	}

	if (PE_RET_OK != GenSmsProperty(pMsg))
	{
		pMsg->Release();
		PEWriteLog(_T("SaveSMS2OutLook:SetMessageProp failed"));
		free(smsMsg);
		return PE_RET_FAIL; 
	}


	GenMmsTypeProperty(pMsg);
	SetMmsTypeProperty(pMsg);

	if (!SetSmsProperty(pMsg,*smsMsg))
	{
		pMsg->Release();
		PEWriteLog(_T("SaveSMS2OutLook:SetMessageProp failed"));
		free(smsMsg);
		return PE_RET_FAIL; 
	}


	hr = SetSmsPropertyEx(pMsg,*smsMsg);
	if (FAILED(hr))
	{
		PEWriteLog(_T("SaveSMS2OutLook:SetMessageProp failed"));
		free(smsMsg);
		pMsg->Release();
		return PE_RET_FAIL; 
	}

	free(smsMsg);

	SaveMimeImage(pMsg,pMmsMsg);
	SaveMimeText(pMsg,pMmsMsg);

	if (pMmsMsg->ulReadFlag == SMS_READ_FLAG_UNREAD)
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

void MmsHelper::GetSmsFromMms(MMS_ITEM *pMms,SMS_ITEM *pSms)
{
	pSms->ulThreadID = 	pMms->ulThreadID;
	pSms->ulMsgID = 	pMms->ulMsgID;
	pSms->ulReadFlag = pMms->ulReadFlag;
	_tcscpy(pSms->tszName,pMms->tszName);
	_tcscpy(pSms->tszPhone,pMms->tszPhone);

	if (pMms->pszBody)
	{
		pSms->pszBody = (TCHAR*)calloc(_tcslen(pMms->pszBody) + 1,sizeof(TCHAR));
		_tcscpy(pSms->pszBody,pMms->pszBody);
	}
	else
	{
		pSms->pszBody = NULL;
	}
	pSms->ulMsgType = pMms->ulMsgType;
	pSms->ftRecvTime = pMms->ftRecvTime;

	if (pMms->ulMsgType == SMS_TYPE_SENT && pMms->pszBody && _tcslen(pMms->pszBody) > 0)
	{
		return;
	}

	CString sTxt = _T("MMS");
	list<MIME_ITEM*>::iterator it1;
	for (it1 = pMms->pMimeData->begin(); it1 != pMms->pMimeData->end(); ++it1)
	{ 
		MIME_ITEM *pItem = (MIME_ITEM*)(*it1);

		if (!_tcsstr(pItem->szMimeType,_T("text/plain")))
		{
			continue;
		}

		sTxt = pItem->szText;
		break;
	}

	if (sTxt.GetLength() > 30)
	{
		sTxt = sTxt.Left(30);
		sTxt += _T("...");
	}

	pSms->pszBody = (TCHAR*)calloc(1024,sizeof(TCHAR));
	_tcscpy(pSms->pszBody,sTxt.GetBuffer());
	sTxt.ReleaseBuffer();

}

void MmsHelper::FreeMimeItem(MMS_ITEM *pMms)
{
	free(pMms->pszBody);
	pMms->pszBody = NULL;
	if (!pMms->pMimeData)
	{
		return;
	}

	list<MIME_ITEM*>::iterator it1;
	for (it1 = pMms->pMimeData->begin(); it1 != pMms->pMimeData->end(); ++it1)
	{ 
		free ((*it1)->MimeData);
		free ((*it1)->szText);
		free(*it1);
	}

	pMms->pMimeData->clear();
}



int MmsHelper::SaveMimeText(LPMESSAGE lpMessage,MMS_ITEM *pMms)
{
	if (!pMms->pMimeData)
	{
		return PE_RET_FAIL;
	}

	CString sTxt = _T("<div>");

	list<MIME_ITEM*>::iterator it1;
	for (it1 = pMms->pMimeData->begin(); it1 != pMms->pMimeData->end(); ++it1)
	{ 
		MIME_ITEM *pItem = (MIME_ITEM*)(*it1);

		if (!_tcsstr(pItem->szMimeType,_T("text/plain")))
		{
			continue;
		}

		sTxt += _T("<p>");
		sTxt += pItem->szText;
		sTxt += _T("</p>");
	}
	sTxt += _T("</div>");

	TCHAR *szHdrFmt = _T("<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/></head><body>%s</body></html>");
	CString sBody;
	sBody.Format(szHdrFmt,sTxt.GetBuffer());
	sTxt.ReleaseBuffer();

	int iLen = WideCharToMultiByte(CP_UTF8,0,sBody.GetBuffer(),sBody.GetLength(),NULL,0,NULL,NULL) + 1;
	sBody.ReleaseBuffer();
	BYTE *pHtmlData = (BYTE*)calloc(iLen,1);
	WideCharToMultiByte(CP_UTF8,0,sBody.GetBuffer(),sBody.GetLength(),(LPSTR)pHtmlData,iLen,NULL,NULL);
	sBody.ReleaseBuffer();

	SPropValue spvProps[1] = {0};

	spvProps[0].ulPropTag = PR_BODY_HTML;
	spvProps[0].Value.bin.lpb = (LPBYTE)pHtmlData;
	spvProps[0].Value.bin.cb = iLen;

	HRESULT hr = lpMessage->SetProps(1,spvProps,NULL);
	if (FAILED(hr))
	{
		PEWriteLog(_T("SaveMimeText:SetProps failed"));
	}

	free(pHtmlData);

	return hr;

}


int MmsHelper::SaveMimeImage(LPMESSAGE lpMessage,MMS_ITEM *pMms)
{
	if (!pMms->pMimeData)
	{
		return PE_RET_FAIL;
	}

	ULONG attachNum;
	list<MIME_ITEM*>::iterator it1;
	int i = 0;
	for (it1 = pMms->pMimeData->begin(); it1 != pMms->pMimeData->end(); ++it1)
	{ 
		MIME_ITEM *pItem = (MIME_ITEM*)(*it1);

		if (!_tcsstr(pItem->szMimeType,_T("image")))
		{
			continue;
		}

		LPATTACH lpAttach;
		HRESULT hr = lpMessage->CreateAttach(NULL, NULL, &attachNum, &lpAttach);
		if(!SUCCEEDED(hr))
		{
			return PE_RET_FAIL;
		}

		// Open the destination stream in the attachment object
		LPSTREAM lpStream;
		hr = lpAttach->OpenProperty (PR_ATTACH_DATA_BIN,&IID_IStream,0,MAPI_MODIFY |MAPI_CREATE,(LPUNKNOWN *)&lpStream);
		if (HR_SUCCEEDED(hr))
		{
			ULONG ulCount = 0;
			lpStream->Write(pItem->MimeData,pItem->iMimeDataLen,&ulCount);
			lpStream->Commit(STGC_DEFAULT);
		}

		//Stream copied, Set the filename properties
		TCHAR szName[255];
		_stprintf(szName,_T("image%d.png"),i++);

		SPropValue spvAttach[3]; 
		spvAttach[0].ulPropTag = PR_ATTACH_METHOD; 
		spvAttach[0].Value.l = ATTACH_BY_VALUE; 
		spvAttach[1].ulPropTag = PR_RENDERING_POSITION; 
		spvAttach[1].Value.l = -1; 
		spvAttach[2].ulPropTag = PR_DISPLAY_NAME; 
		spvAttach[2].Value.lpszW = szName;
		hr = lpAttach->SetProps(3,(LPSPropValue)&spvAttach, NULL);

		lpAttach->SaveChanges(NULL);
	}

	return PE_RET_OK;
}


int MmsHelper::GetMaxMmsID(ULONG &ulMaxMsgID)
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

		GenMmsTypeProperty(pMsg);
		if (!IsMmsType(pMsg))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
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


int MmsHelper::GenMmsTypeProperty(LPMESSAGE lpMessage)
{
	MAPINAMEID	rgnmid[1] = {0};
	LPMAPINAMEID rgpnmid[1] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&MOBITNT_MMS_TYPE_GUID;
	rgnmid[0].ulKind = MNID_STRING;
	rgnmid[0].Kind.lpwstrName = _T("MOBITNT_MMS_TYPE");
	rgpnmid[0] = &rgnmid[0];

	HRESULT hRes = lpMessage->GetIDsFromNames(1,(LPMAPINAMEID*) &rgpnmid,MAPI_CREATE,&lpNamedPropTags);
	if (FAILED(hRes))
	{
		return PE_RET_FAIL;
	}

	m_lTag4MmsType = CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[0],PT_LONG);

	MAPIFreeBuffer(lpNamedPropTags);
	return PE_RET_OK;

}

int MmsHelper::SetMmsTypeProperty(LPMESSAGE lpMessage)
{
	SPropValue spvProps[1] = {0};

	spvProps[0].ulPropTag = m_lTag4MmsType;
	spvProps[0].Value.l = 1;

	HRESULT hRes = lpMessage->SetProps(1, spvProps, NULL);
	if (FAILED(hRes))
	{
		return false;
	}

	return true;

}


int MmsHelper::IsMmsType(LPMESSAGE lpMessage)
{
	SizedSPropTagArray(1, mcols) = {1,{m_lTag4MmsType}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	if (pcount != 1)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	if (props[0].ulPropTag != m_lTag4MmsType)
	{
		MAPIFreeBuffer(props);
		return false;
	}

	int iRet = props[0].Value.l;

	MAPIFreeBuffer(props);

	return iRet;

}


int MmsHelper::GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft)
{
	GenMmsTypeProperty(pMsg);
	if (!IsMmsType(pMsg))
	{
		return PE_RET_FAIL;;
	}

	SMS_ITEM sms;
	memset(&sms,0,sizeof(sms));

	GenSmsProperty(pMsg);

	if (!GetSmsProperty(pMsg,sms))
	{
		return PE_RET_FAIL;
	}

	if (!GetSmsPropertyEx(pMsg,sms))
	{
		return PE_RET_FAIL;
	}

	memcpy(&ft,&sms.ftRecvTime,sizeof(ft));
	return PE_RET_OK;
}





