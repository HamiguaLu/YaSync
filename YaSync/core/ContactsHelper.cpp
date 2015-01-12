#include "stdafx.h"
#include "ContactsHelper.h"
#include "PECore.h"
#include "AppTools.h"
#include "mapiutil.h"
#include "PEInterface.h"
#include <initguid.h>

#include "TrayHelper.h"
#include "SettingHelper.h"


//DEFINE_OLEGUID(PSETID_Address, MAKELONG(0x2004+(0xC),0x0046),0,0);
DEFINE_OLEGUID(PSETID_Address, MAKELONG(0x2000+(0x04),0x0006),0,0);


ULONG aulWorkAddrProps[] = {
	PidLidWorkAddressStreet,
	PidLidWorkAddressCity, 		
	PidLidWorkAddressState,		
	PidLidWorkAddressPostalCode,
	PidLidWorkAddressCountry,
};
#define ulWorkAddrProps (sizeof(aulWorkAddrProps)/sizeof(aulWorkAddrProps [0]))


ULONG aulFileUnderProps[] = {
	PidLidFileUnder,
};
#define ulFileUnderProps (sizeof(aulFileUnderProps)/sizeof(aulFileUnderProps[0]))


enum _WORK_ADDR_PROPS{
	p_PidLidWorkAddressStreet,
	p_PidLidWorkAddressCity, 		
	p_PidLidWorkAddressState,		
	p_PidLidWorkAddressPostalCode,
	p_PidLidWorkAddressCountry,

	NUM_WORK_ADDR_PROPS
};


ULONG aulEmailProps[] = {
	PidLidEmail1EmailAddress,
	PidLidEmail2EmailAddress, 		
	PidLidEmail3EmailAddress,		
};
#define ulEmailProps (sizeof(aulEmailProps)/sizeof(aulEmailProps [0]))

ULONG aulImProps[] = {
	PidLidInstantMessagingAddress,
};
#define ulImProps (sizeof(aulImProps)/sizeof(aulImProps [0]))


ULONG aulSiteProps[] = {
	PidLidWebSite,
};
#define ulSiteProps (sizeof(aulSiteProps)/sizeof(aulSiteProps [0]))


extern list<CONTACT_ITEM*> 	g_contactList;
void FreeContactList()
{
	list<CONTACT_ITEM*>::iterator it;
	for (it = g_contactList.begin(); it != g_contactList.end(); ++it)
	{ 
		CONTACT_ITEM *pContact = (CONTACT_ITEM*)*it;

		ContactsHelper::FreeContactItem(pContact);
		free(pContact);
	}

	g_contactList.clear();
}

int ContactsHelper::SetContactFileUnder(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	MAPINAMEID	rgnmid[ulFileUnderProps] = {0};
	LPMAPINAMEID rgpnmid[ulFileUnderProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulFileUnderProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulFileUnderProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulFileUnderProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		MAPIFreeBuffer(lpNamedPropTags);
		return PE_RET_OK;
	}

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_UNICODE);
	if (_tcslen(pContact->szDisplayName) > 0)
	{
		spvProps[0].Value.lpszW = pContact->szDisplayName;	
	}
	else
	{
		spvProps[0].Value.lpszW = _T(" ");			
	}

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(1, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactFileUnder,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}


int ContactsHelper::SetContactAddress(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (!pContact->pAddrs)
	{
		return PE_RET_OK;
	}

	MAPINAMEID	rgnmid[ulWorkAddrProps] = {0};
	LPMAPINAMEID rgpnmid[ulWorkAddrProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulWorkAddrProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulWorkAddrProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulWorkAddrProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		MAPIFreeBuffer(lpNamedPropTags);
		return PE_RET_OK;
	}

	SPropValue spvProps[16] = {0};

	list<AddrItem*>::iterator it;
	int iCount = 0;
	for (it = pContact->pAddrs->begin(); it != pContact->pAddrs->end(); ++it)
	{ 
		if (iCount >= 3 * ADDR_PROP_COUNT)
		{
			PEWriteLog(_T("SetContactAddress:only support 3 addrs"));
			iCount = 3 * ADDR_PROP_COUNT;
			break;
		}

		AddrItem*p = (AddrItem*)*it;
		bool bSupported = true;
		switch (p->iAddrType)
		{
		case ADDR_TYPE_HOME:
			{
				spvProps[iCount].ulPropTag = PR_HOME_ADDRESS_STREET;
				spvProps[iCount + 1].ulPropTag = PR_HOME_ADDRESS_POSTAL_CODE;
				//spvProps[iCount].ulPropTag = PR_HOME_ADDRESS_POST_OFFICE_BOX;
				spvProps[iCount + 2].ulPropTag = PR_HOME_ADDRESS_CITY;
				spvProps[iCount + 3].ulPropTag = PR_HOME_ADDRESS_STATE_OR_PROVINCE;
				spvProps[iCount + 4].ulPropTag = PR_HOME_ADDRESS_COUNTRY;
				break;
			}
		case ADDR_TYPE_WORK:
			{
				spvProps[iCount].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressStreet],PT_UNICODE);
				spvProps[iCount + 1].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressPostalCode],PT_UNICODE);
				spvProps[iCount + 2].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressCity],PT_UNICODE);
				spvProps[iCount + 3].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressState],PT_UNICODE);
				spvProps[iCount + 4].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressCountry],PT_UNICODE);
				//spvProps[iCount].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidTimeZoneDescription],PT_UNICODE);
				break;
			}
		case ADDR_TYPE_OTHER:
			{
				spvProps[iCount].ulPropTag = PR_OTHER_ADDRESS_STREET;
				spvProps[iCount + 1].ulPropTag = PR_OTHER_ADDRESS_POSTAL_CODE;
				//spvProps[iCount].ulPropTag = PR_OTHER_ADDRESS_POST_OFFICE_BOX;
				spvProps[iCount + 2].ulPropTag = PR_OTHER_ADDRESS_CITY;
				spvProps[iCount + 3].ulPropTag = PR_OTHER_ADDRESS_STATE_OR_PROVINCE;
				spvProps[iCount + 4].ulPropTag = PR_OTHER_ADDRESS_COUNTRY;
				break;
			}
		default:
			{
				CString sInfo;
				sInfo.Format(_T("ADDR Type %d is not supported"),p->iAddrType);
				PEWriteLog(sInfo.GetBuffer());
				sInfo.ReleaseBuffer();
				bSupported = false;
				break;
			}
		}

		if (bSupported)
		{
			spvProps[iCount].Value.lpszW = p->szAddrStreet;
			spvProps[iCount + 1].Value.lpszW = p->szAddrPostCode;
			spvProps[iCount + 2].Value.lpszW = p->szAddrCity;
			spvProps[iCount + 3].Value.lpszW = p->szAddrRegion;
			spvProps[iCount + 4].Value.lpszW = p->szCountry;

			iCount += ADDR_PROP_COUNT;
		}
	}

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(iCount, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactAddress,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}

int ContactsHelper::IsPhoneTypeSupported(int iPhoneType)
{
	if (iPhoneType > PHONE_TYPE_MAX_VALUE)
	{
		return PE_RET_FAIL;
	}

	int iSupported = PE_RET_FAIL;
	switch (iPhoneType)
	{
	case  PHONE_TYPE_FAX_HOME:
	case  PHONE_TYPE_COMPANY_MAIN:
	case  PHONE_TYPE_MAIN:
	case  PHONE_TYPE_OTHER_FAX:
	case  PHONE_TYPE_WORK_MOBILE:
	case  PHONE_TYPE_WORK_PAGER:
	case  PHONE_TYPE_MMS:
		{
			CString sInfo;
			sInfo.Format(_T("Phone Type %d is not supported"),iPhoneType);
			PEWriteLog(sInfo.GetBuffer());
			sInfo.ReleaseBuffer();
			break;
		}
	default:
		{
			iSupported = PE_RET_OK;
			break;
		}
	}

	return iSupported;
}



int ContactsHelper::SetContactPhone(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (!pContact->pPhones)
	{
		return PE_RET_OK;
	}

	int iCount = 0;

	//now support 20 phones
	SPropValue spvProps[20*2 + 1] = {0};
	list<PhoneItem*>::iterator it;
	for (it = pContact->pPhones->begin(); it != pContact->pPhones->end(); ++it)
	{ 
		if (iCount >= 20 * 2)
		{
			PEWriteLog(_T("SetContactPhone:only support 20 phones"));
			iCount = 20 * 2;
			break;
		}

		bool bSupported = true;

		PhoneItem*p = (PhoneItem*)*it;
		switch (p->iPhoneType)
		{
		case  PHONE_TYPE_HOME:
			{
				spvProps[iCount].ulPropTag = PR_HOME_TELEPHONE_NUMBER;
				break;
			}
		case  PHONE_TYPE_MOBILE:
			{
				spvProps[iCount].ulPropTag = PR_MOBILE_TELEPHONE_NUMBER;
				break;
			}
		case  PHONE_TYPE_WORK:
			{
				spvProps[iCount].ulPropTag = PR_BUSINESS_TELEPHONE_NUMBER;
				break;
			}

		case  PHONE_TYPE_FAX_WORK:
			{
				spvProps[iCount].ulPropTag = PR_BUSINESS_FAX_NUMBER;
				break;
			}
#if 0		
		case  PHONE_TYPE_FAX_HOME:
			{
				break;
			}
#endif		
		case  PHONE_TYPE_PAGER:
			{
				spvProps[iCount].ulPropTag = PR_PAGER_TELEPHONE_NUMBER;
				break;
			}
		case  PHONE_TYPE_OTHER:
			{
				spvProps[iCount].ulPropTag = PR_OTHER_TELEPHONE_NUMBER;
				break;
			}
		case  PHONE_TYPE_CALLBACK:
			{
				spvProps[iCount].ulPropTag = PR_CALLBACK_TELEPHONE_NUMBER;
				break;
			}
		case  PHONE_TYPE_CAR:
			{
				spvProps[iCount].ulPropTag = PR_CAR_TELEPHONE_NUMBER;
				break;
			}
#if 0		
		case  PHONE_TYPE_COMPANY_MAIN:
			{
				break;
			}
#endif		
		case  PHONE_TYPE_ISDN:
			{
				spvProps[iCount].ulPropTag = PR_ISDN_NUMBER;
				break;
			}
#if 0		
		case  PHONE_TYPE_MAIN:
			{
				break;
			}
		case  PHONE_TYPE_OTHER_FAX:
			{
				break;
			}
#endif		
		case  PHONE_TYPE_RADIO:
			{
				spvProps[iCount].ulPropTag = PR_RADIO_TELEPHONE_NUMBER;
				break;
			}
		case  PHONE_TYPE_TELEX:
			{
				spvProps[iCount].ulPropTag = PR_TELEX_NUMBER;
				break;
			}
		case  PHONE_TYPE_TTY_TDD:
			{
				spvProps[iCount].ulPropTag = PR_TTYTDD_PHONE_NUMBER;
				break;
			}
#if 0		
		case  PHONE_TYPE_WORK_MOBILE:
			{
				break;
			}
		case  PHONE_TYPE_WORK_PAGER:
			{
				break;
			}
#endif		
		case  PHONE_TYPE_ASSISTANT:
			{
				spvProps[iCount].ulPropTag = PR_ASSISTANT_TELEPHONE_NUMBER;
				break;
			}
#if 0		
		case  PHONE_TYPE_MMS:
			{
				break;
			}
#endif		
		default:
			{
				CString sInfo;
				sInfo.Format(_T("Phone Type %d is not supported"),p->iPhoneType);
				PEWriteLog(sInfo.GetBuffer());
				sInfo.ReleaseBuffer();

				bSupported = false;
				break;
			}
		}

		if (bSupported)
		{
			spvProps[iCount].Value.lpszW = p->szPhoneNo;

			iCount += 1;
		}
	}

	HRESULT	hRes = lpMessage->SetProps(iCount, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactPhone,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;

}



int ContactsHelper::SetContactOrg(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	int iCount = 0;
	SPropValue spvProps[2 * 2] = {0};

	if (_tcslen(pContact->szOrgCompany) > 0)
	{
		spvProps[iCount].ulPropTag = PR_COMPANY_NAME;
		spvProps[iCount].Value.lpszW = pContact->szOrgCompany;
		++iCount;
	}

	if (_tcslen(pContact->szOrgTitle) > 0)
	{
		spvProps[iCount].ulPropTag = PR_TITLE;
		spvProps[iCount].Value.lpszW = pContact->szOrgTitle;
		++iCount;
	}	

	if (iCount == 0)
	{
		return PE_RET_OK;
	}

	HRESULT	hRes = lpMessage->SetProps(iCount, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactOrg,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}


int ContactsHelper::SetContactIM(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (!pContact->pIMs)
	{
		return PE_RET_OK;
	}

	MAPINAMEID	rgnmid[ulImProps] = {0};
	LPMAPINAMEID rgpnmid[ulImProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulImProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulImProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulImProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		MAPIFreeBuffer(lpNamedPropTags);
		return PE_RET_OK;
	}

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_UNICODE);

	MAPIFreeBuffer(lpNamedPropTags);

	list<ImItem*>::iterator it;

	CString sIMList = _T("");

	for (it = pContact->pIMs->begin(); it != pContact->pIMs->end(); ++it)
	{ 
		ImItem *p = (ImItem*)*it;
		if (it != pContact->pIMs->begin())
		{
			sIMList += _T(";");
		}

		switch (p->iIMType)
		{
		case IM_PROTOCOL_CUSTOM:
			{
				sIMList += _T("Custom:");
				break;
			}
		case IM_PROTOCOL_AIM:
			{
				sIMList += _T("AIM:");
				break;
			}
		case IM_PROTOCOL_MSN:
			{
				sIMList += _T("MSN:");
				break;
			}
		case IM_PROTOCOL_YAHOO:
			{
				sIMList += _T("YAHOO:");
				break;
			}
		case IM_PROTOCOL_SKYPE:
			{
				sIMList += _T("SKYPE:");
				break;
			}
		case IM_PROTOCOL_QQ:
			{
				sIMList += _T("QQ:");
				break;
			}
		case IM_PROTOCOL_GOOGLE_TALK:
			{
				sIMList += _T("GOOGLE_TALK:");
				break;
			}
		case IM_PROTOCOL_ICQ:
			{
				sIMList += _T("ICQ:");
				break;
			}
		case IM_PROTOCOL_JABBER:
			{
				sIMList += _T("JABBER:");
				break;
			}
		case IM_PROTOCOL_NETMEETING:
			{
				sIMList += _T("NETMEETING:");
				break;
			}
		default:
			{
				break;
			}
		}

		sIMList += p->szIM;

	}

	spvProps[0].Value.lpszW = sIMList.GetBuffer();
	hRes = lpMessage->SetProps(1, spvProps, NULL);
	sIMList.ReleaseBuffer();
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactEMail,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}



int ContactsHelper::SetContactWebSite(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (!pContact->pSites)
	{
		return PE_RET_OK;
	}

	MAPINAMEID	rgnmid[ulSiteProps] = {0};
	LPMAPINAMEID rgpnmid[ulSiteProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulSiteProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulSiteProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulSiteProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		MAPIFreeBuffer(lpNamedPropTags);
		return PE_RET_OK;
	}

	SPropValue spvProps[1] = {0};

	list<WebSiteItem*>::iterator it;
	int iCount = 0;
	
	for (it = pContact->pSites->begin(); it != pContact->pSites->end(); ++it)
	{ 
		WebSiteItem*p = (WebSiteItem*)*it;
		spvProps[0].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_UNICODE);
		spvProps[0].Value.lpszW = p->szURL;
		iCount = 1;
	}

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(iCount, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactWebSite,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}




int ContactsHelper::SetContactEMail(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (!pContact->pEMails)
	{
		return PE_RET_OK;
	}

	MAPINAMEID	rgnmid[ulEmailProps] = {0};
	LPMAPINAMEID rgpnmid[ulEmailProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulEmailProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulEmailProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulEmailProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		MAPIFreeBuffer(lpNamedPropTags);
		return PE_RET_OK;
	}

	SPropValue spvProps[3] = {0};

	list<EMailItem*>::iterator it;
	int iCount = 0;
	for (it = pContact->pEMails->begin(); it != pContact->pEMails->end(); ++it)
	{ 
		if (iCount >= 3)
		{
			PEWriteLog(_T("SetContactEMail:only support 3 emails"));
			iCount = 3;
			break;
		}

		EMailItem*p = (EMailItem*)*it;
		bool bSupported = true;
		switch (p->iType)
		{
		case EMAIL_TYPE_HOME:
			{
				spvProps[iCount].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_UNICODE);
				break;
			}
		case EMAIL_TYPE_WORK:
			{
				spvProps[iCount].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[1],PT_UNICODE);
				break;
			}
		case EMAIL_TYPE_OTHER:
			{
				spvProps[iCount].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[2],PT_UNICODE);
				break;
			}
		case EMAIL_TYPE_MOBILE:
		default:
			{
				CString sInfo;
				sInfo.Format(_T("Mail Type %d is not supported"),p->iType);
				PEWriteLog(sInfo.GetBuffer());
				sInfo.ReleaseBuffer();
				bSupported = false;
				break;
			}
		}

		if (bSupported)
		{
			spvProps[iCount].Value.lpszW = p->szEmail;
			++iCount;
		}
	}

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(iCount, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactEMail,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}


int ContactsHelper::SetContactNotes(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	HRESULT hRes = S_OK;
	if (!pContact->szNotes || _tcslen(pContact->szNotes) < 1)
	{
		return PE_RET_OK;
	}

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag   = PR_BODY_W;
	spvProps[0].Value.lpszW = pContact->szNotes;

	hRes = lpMessage->SetProps(1, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactNotes,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}


int ContactsHelper::SetContactBirthDay(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (pContact->ftBirthDay.dwHighDateTime < 100)
	{
		//sounds like a invalid birthday,just skip it
		return PE_RET_OK;
	}

	HRESULT hRes = S_OK;

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag   = PR_BIRTHDAY;

	memcpy(&spvProps[0].Value.ft,&pContact->ftBirthDay,sizeof(FILETIME));

	hRes = lpMessage->SetProps(6, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactName,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}



int ContactsHelper::SetContactName(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	HRESULT hRes = S_OK;

	// Since we know in advance which props we'll be setting, we can statically declare most of the structures involved and save expensive MAPIAllocateBuffer calls
	SPropValue spvProps[6] = {0};
	spvProps[0].ulPropTag   = PR_MESSAGE_CLASS_W;
	spvProps[1].ulPropTag	= PR_SURNAME_W;
	spvProps[2].ulPropTag   = PR_MIDDLE_NAME_W;
	spvProps[3].ulPropTag   = PR_GIVEN_NAME_W;
	spvProps[4].ulPropTag   = PR_DISPLAY_NAME_W;
	spvProps[5].ulPropTag   = PR_SUBJECT_W;

	// These two must be the same
	spvProps[0].Value.lpszW = L"IPM.Contact";
	spvProps[1].Value.lpszW = pContact->szFamilyName;
	spvProps[2].Value.lpszW = pContact->szMiddleName;
	spvProps[3].Value.lpszW = pContact->szGivenName;
	spvProps[4].Value.lpszW = pContact->szDisplayName;
	spvProps[5].Value.lpszW = pContact->szDisplayName;

	hRes = lpMessage->SetProps(6, spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		return PE_RET_OK;
	}

	CString sInfo;
	sInfo.Format(_T("SetContactName,SetProps failed code:%d"),hRes);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();

	return PE_RET_FAIL;
}

int ContactsHelper::PhoneListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM *pContact2)
{
	if (!pContact1->pPhones && !pContact2->pPhones)
	{
		return PE_SAME_ITEM;
	}

	if (!pContact1->pPhones || !pContact2->pPhones)
	{
		return PE_NEED_MERGE_ITEM;
	}

	list<PhoneItem*>::iterator it1,it2;
	int iRet = PE_NEED_MERGE_ITEM;
	for ( it1 = pContact1->pPhones->begin(); it1 != pContact1->pPhones->end(); ++it1)
	{
		PhoneItem*p1 = (PhoneItem*)*it1;
		for (it2 = pContact2->pPhones->begin() ;it2 != pContact2->pPhones->end();++it2)
		{ 
			PhoneItem*p2 = (PhoneItem*)*it2;

			if (_tcscmp(p1->szPhoneNo,p2->szPhoneNo) != 0)
			{
				continue;
			} 

			if (p1->iPhoneType != p2->iPhoneType)
			{
				return PE_CONFLICT_ITEM;
			}

		}
	}

	return PE_NEED_MERGE_ITEM;

}

int ContactsHelper::AddrListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM *pContact2)
{
	if (!pContact1->pAddrs && !pContact2->pAddrs)
	{
		return PE_SAME_ITEM;
	}

	if (!pContact1->pAddrs || !pContact2->pAddrs)
	{
		return PE_NEED_MERGE_ITEM;
	}

	list<AddrItem*>::iterator it1,it2;
	int iRet = PE_NEED_MERGE_ITEM;
	for ( it1 = pContact1->pAddrs->begin(); it1 != pContact1->pAddrs->end(); ++it1)
	{
		AddrItem*p1 = (AddrItem*)*it1;
		for (it2 = pContact2->pAddrs->begin() ;it2 != pContact2->pAddrs->end();++it2)
		{ 
			AddrItem*p2 = (AddrItem*)*it2;

			if (_tcscmp(p1->szCountry,p2->szCountry) != 0)
			{
				continue;
			}

			if (_tcscmp(p1->szAddrCity,p2->szCountry) != 0)
			{
				continue;
			} 

			if (_tcscmp(p1->szAddrFormatAddress,p2->szCountry) != 0)
			{
				continue;
			} 

			if (_tcscmp(p1->szAddrPostCode,p2->szCountry) != 0)
			{
				continue;
			} 

			if (_tcscmp(p1->szAddrRegion,p2->szCountry) != 0)
			{
				continue;
			} 

			if (_tcscmp(p1->szAddrStreet,p2->szCountry) != 0)
			{
				continue;
			} 

			if (p1->iAddrType != p2->iAddrType)
			{
				return PE_CONFLICT_ITEM;
			}

		}
	}

	return PE_NEED_MERGE_ITEM;
}

int ContactsHelper::EMailListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM *pContact2)
{
	if (!pContact1->pEMails && !pContact2->pEMails)
	{
		return PE_SAME_ITEM;
	}

	if (!pContact1->pEMails || !pContact2->pEMails)
	{
		return PE_NEED_MERGE_ITEM;
	}

	list<EMailItem*>::iterator it1,it2;
	int iRet = PE_NEED_MERGE_ITEM;
	for ( it1 = pContact1->pEMails->begin(); it1 != pContact1->pEMails->end(); ++it1)
	{
		EMailItem*p1 = (EMailItem*)*it1;
		for (it2 = pContact2->pEMails->begin() ;it2 != pContact2->pEMails->end();++it2)
		{ 
			EMailItem*p2 = (EMailItem*)*it2;

			if (_tcscmp(p1->szEmail,p2->szEmail) != 0)
			{
				continue;
			} 

			if (p1->iType != p2->iType)
			{
				return PE_CONFLICT_ITEM;
			}
		}
	}

	return PE_NEED_MERGE_ITEM;
}

int ContactsHelper::IMListCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM *pContact2)
{
	if (!pContact1->pIMs && !pContact2->pIMs)
	{
		return PE_SAME_ITEM;
	}

	if (!pContact1->pIMs || !pContact2->pIMs)
	{
		return PE_NEED_MERGE_ITEM;
	}

	list<ImItem*>::iterator it1,it2;
	int iRet = PE_NEED_MERGE_ITEM;
	for ( it1 = pContact1->pIMs->begin(); it1 != pContact1->pIMs->end(); ++it1)
	{
		ImItem*p1 = (ImItem*)*it1;
		for (it2 = pContact2->pIMs->begin() ;it2 != pContact2->pIMs->end();++it2)
		{ 
			ImItem*p2 = (ImItem*)*it2;

			if (_tcscmp(p1->szIM,p2->szIM) != 0)
			{
				continue;
			} 

			if (p1->iIMType != p2->iIMType)
			{
				return PE_CONFLICT_ITEM;
			}
		}
	}

	return PE_NEED_MERGE_ITEM;
}

int ContactsHelper::ContactCompare(CONTACT_ITEM  *pContact1,CONTACT_ITEM *pContact2)
{
	if (_tcscmp(pContact1->szDisplayName,pContact2->szDisplayName) != 0)
	{
		return PE_DIFFERENT_ITEM;
	}

	if (_tcscmp(pContact1->szGivenName,pContact2->szGivenName) != 0)
	{
		return PE_DIFFERENT_ITEM;
	}

	if (_tcscmp(pContact1->szMiddleName,pContact2->szMiddleName) != 0)
	{
		return PE_DIFFERENT_ITEM;
	}

	if (_tcscmp(pContact1->szFamilyName,pContact2->szFamilyName) != 0)
	{
		return PE_DIFFERENT_ITEM;
	}

	return PE_SAME_ITEM;

#if 0
	if (_tcscmp(pContact1->szDisplayName,pContact2->szDisplayName) != 0)
	{
		return PE_RET_FAIL;
	}
#endif

	int iCMP1 = PhoneListCompare(pContact1,pContact2);
	int iCMP2 = AddrListCompare(pContact1,pContact2);
	int iCMP3 = EMailListCompare(pContact1,pContact2);
	int iCMP4 = IMListCompare(pContact1,pContact2);

	if (iCMP1 == PE_CONFLICT_ITEM ||iCMP2 == PE_CONFLICT_ITEM ||iCMP2 == PE_CONFLICT_ITEM ||iCMP2 == PE_CONFLICT_ITEM)
	{
		return PE_CONFLICT_ITEM;
	}

	if (iCMP1 == PE_NEED_MERGE_ITEM && iCMP2 == PE_NEED_MERGE_ITEM && iCMP2 == PE_NEED_MERGE_ITEM && iCMP2 == PE_NEED_MERGE_ITEM)
	{
		return PE_CONFLICT_ITEM;
	}

	if (iCMP1 == PE_SAME_ITEM && iCMP2 == PE_SAME_ITEM && iCMP2 == PE_SAME_ITEM && iCMP2 == PE_SAME_ITEM)
	{
		return PE_SAME_ITEM;
	}

	return PE_DIFFERENT_ITEM;
}




int ContactsHelper::GetContactName(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	SizedSPropTagArray(4, mcols) = {4,{PR_SURNAME_W,PR_MIDDLE_NAME_W,PR_GIVEN_NAME_W,PR_DISPLAY_NAME_W}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	for (int i = 0; i < (int)pcount; ++i)
	{
		switch (props[i].ulPropTag)
		{
		case PR_SURNAME_W:
			{
				_tcscpy(pContact->szFamilyName,props[i].Value.lpszW);
				break;
			}
		case PR_MIDDLE_NAME_W:
			{
				_tcscpy(pContact->szMiddleName,props[i].Value.lpszW);
				break;
			}
		case PR_GIVEN_NAME_W:
			{
				_tcscpy(pContact->szGivenName,props[i].Value.lpszW);
				break;
			}
		case PR_DISPLAY_NAME_W:
			{
				_tcscpy(pContact->szDisplayName,props[i].Value.lpszW);
				break;
			}
		}
	}

	MAPIFreeBuffer(props);
	return PE_RET_OK;
}


int ContactsHelper::GetContactOrg(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	SizedSPropTagArray(2, mcols) = {2,{PR_COMPANY_NAME,PR_TITLE}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return PE_RET_FAIL;
	}

	if (pcount != 2)
	{
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag == PR_COMPANY_NAME)
	{
		_tcscpy(pContact->szOrgCompany,props[0].Value.lpszW);
	}

	if (props[1].ulPropTag == PR_TITLE)
	{
		_tcscpy(pContact->szOrgTitle,props[1].Value.lpszW);
	}

	MAPIFreeBuffer(props);
	return PE_RET_OK;;
}


int ContactsHelper::GetContactAddress(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	MAPINAMEID	rgnmid[ulWorkAddrProps] = {0};
	LPMAPINAMEID rgpnmid[ulWorkAddrProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulWorkAddrProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulWorkAddrProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulWorkAddrProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		MAPIFreeBuffer(lpNamedPropTags);
		return PE_RET_OK;
	}

	ULONG ulWorkStreetTag =	CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressStreet],PT_UNICODE);
	ULONG ulWorkCityTag =	CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressCity],PT_UNICODE);
	ULONG ulWorkStateTag =	CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressState],PT_UNICODE);
	ULONG ulWorkPostalCodeTag =	CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressPostalCode],PT_UNICODE);
	ULONG ulWorkCountryTag =	CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressCountry],PT_UNICODE);

	MAPIFreeBuffer(lpNamedPropTags);


	SizedSPropTagArray(15, mcols) = {15,
	{
		ulWorkStreetTag,ulWorkCityTag,ulWorkStateTag,ulWorkPostalCodeTag,ulWorkCountryTag,
			PR_HOME_ADDRESS_STREET,PR_HOME_ADDRESS_POSTAL_CODE,PR_HOME_ADDRESS_CITY,PR_HOME_ADDRESS_STATE_OR_PROVINCE,PR_HOME_ADDRESS_COUNTRY,
			PR_OTHER_ADDRESS_STREET,PR_OTHER_ADDRESS_POSTAL_CODE,PR_OTHER_ADDRESS_CITY,PR_OTHER_ADDRESS_STATE_OR_PROVINCE,PR_OTHER_ADDRESS_COUNTRY
	}	

	};

	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	AddrItem *pWork = (AddrItem *)calloc(1,sizeof(AddrItem));
	AddrItem *pHome = (AddrItem *)calloc(1,sizeof(AddrItem));
	AddrItem *pOther = (AddrItem *)calloc(1,sizeof(AddrItem));

	for (int i = 0; i < (int)pcount; ++i)
	{
		if  (props[i].ulPropTag == ulWorkStreetTag)
		{
			pWork->iAddrType = ADDR_TYPE_WORK;
			_tcscpy(pWork->szAddrStreet,props[i].Value.lpszW);
		}
		else if  (props[i].ulPropTag == ulWorkCityTag)
		{
			pWork->iAddrType = ADDR_TYPE_WORK;
			_tcscpy(pWork->szAddrCity,props[i].Value.lpszW);	
		}
		else if (props[i].ulPropTag ==  ulWorkStateTag)
		{
			pWork->iAddrType = ADDR_TYPE_WORK;
			_tcscpy(pWork->szAddrRegion,props[i].Value.lpszW);
		}
		else if (props[i].ulPropTag ==  ulWorkPostalCodeTag)
		{
			pWork->iAddrType = ADDR_TYPE_WORK;
			_tcscpy(pWork->szAddrPostCode,props[i].Value.lpszW);
		}
		else if (props[i].ulPropTag ==  ulWorkCountryTag)
		{
			pWork->iAddrType = ADDR_TYPE_WORK;
			_tcscpy(pWork->szCountry,props[i].Value.lpszW);
		}
		else
		{
			switch (props[i].ulPropTag)
			{
			case PR_HOME_ADDRESS_STREET:
				{
					pHome->iAddrType = ADDR_TYPE_HOME;
					_tcscpy(pHome->szAddrStreet,props[i].Value.lpszW);
					break;
				}
			case PR_HOME_ADDRESS_POSTAL_CODE:
				{
					pHome->iAddrType = ADDR_TYPE_HOME;
					_tcscpy(pHome->szAddrPostCode,props[i].Value.lpszW);
					break;
				}
			case PR_HOME_ADDRESS_CITY:
				{
					pHome->iAddrType = ADDR_TYPE_HOME;
					_tcscpy(pHome->szAddrCity,props[i].Value.lpszW);	
					break;
				}
			case PR_HOME_ADDRESS_STATE_OR_PROVINCE:
				{
					pHome->iAddrType = ADDR_TYPE_HOME;
					_tcscpy(pHome->szAddrRegion,props[i].Value.lpszW);
					break;
				}
			case PR_HOME_ADDRESS_COUNTRY:
				{
					pHome->iAddrType = ADDR_TYPE_HOME;
					_tcscpy(pHome->szCountry,props[i].Value.lpszW);
					break;
				}
			case PR_OTHER_ADDRESS_STREET:
				{
					pOther->iAddrType = ADDR_TYPE_OTHER;
					_tcscpy(pOther->szAddrStreet,props[i].Value.lpszW);
					break;
				}
			case PR_OTHER_ADDRESS_POSTAL_CODE:
				{
					pOther->iAddrType = ADDR_TYPE_OTHER;
					_tcscpy(pOther->szAddrPostCode,props[i].Value.lpszW);
					break;
				}
			case PR_OTHER_ADDRESS_CITY:
				{
					pOther->iAddrType = ADDR_TYPE_OTHER;
					_tcscpy(pOther->szAddrCity,props[i].Value.lpszW);
					break;
				}
			case PR_OTHER_ADDRESS_STATE_OR_PROVINCE:
				{
					pOther->iAddrType = ADDR_TYPE_OTHER;
					_tcscpy(pOther->szAddrRegion,props[i].Value.lpszW);
					break;
				}
			case PR_OTHER_ADDRESS_COUNTRY:
				{
					pOther->iAddrType = ADDR_TYPE_OTHER;
					_tcscpy(pOther->szCountry,props[i].Value.lpszW);
					break;
				}
			}
		}
	}

	MAPIFreeBuffer(props);

	if (pWork->iAddrType == ADDR_TYPE_UNKNOW && pHome->iAddrType == ADDR_TYPE_UNKNOW && pOther->iAddrType == ADDR_TYPE_UNKNOW )
	{
		free(pWork);
		free(pHome);
		free(pOther);
		return PE_RET_OK;
	}

	if (pContact->pAddrs == NULL)
	{
		pContact->pAddrs = new list<AddrItem*>;
	}

	if (pWork->iAddrType != ADDR_TYPE_UNKNOW )
	{
		pContact->pAddrs->push_back(pWork);
	}
	else
	{
		free(pWork);
	}

	if (pHome->iAddrType != ADDR_TYPE_UNKNOW)
	{
		pContact->pAddrs->push_back(pHome);
	}
	else
	{
		free(pHome);
	}

	if (pOther->iAddrType != ADDR_TYPE_UNKNOW )
	{
		pContact->pAddrs->push_back(pOther);
	}
	else
	{
		free(pOther);
	}


	return PE_RET_OK;
}


int ContactsHelper::GetContactWebSite(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	MAPINAMEID	rgnmid[ulSiteProps] = {0};
	LPMAPINAMEID rgpnmid[ulSiteProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulSiteProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulSiteProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulSiteProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		MAPIFreeBuffer(lpNamedPropTags);
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return PE_RET_OK;
	}

	ULONG ulSiteTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_UNICODE);
	MAPIFreeBuffer(lpNamedPropTags);

	SizedSPropTagArray(1, mcols) = {1,{ulSiteTag}	};

	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (pcount != 1)
	{
		return PE_RET_FAIL;
	}
	
	if (props[0].ulPropTag == ulSiteTag)
	{
		WebSiteItem  *p = (WebSiteItem *)calloc(1,sizeof(WebSiteItem));
		p->iType = WEBSITE_TYPE_OTHER;
		_tcscpy(p->szURL,props[0].Value.lpszW);
		
		if (pContact->pSites == NULL)
		{
			pContact->pSites = new list<WebSiteItem*>;
		}
		pContact->pSites->push_back(p);
	}

	MAPIFreeBuffer(props);
	return PE_RET_OK;
}



int ContactsHelper::GetContactEMail(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	MAPINAMEID	rgnmid[ulEmailProps] = {0};
	LPMAPINAMEID rgpnmid[ulEmailProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulEmailProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Address;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulEmailProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	HRESULT hRes = m_pFolder->GetIDsFromNames(ulEmailProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		MAPIFreeBuffer(lpNamedPropTags);
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return PE_RET_OK;
	}

	ULONG ulEmailTag1 = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[0],PT_UNICODE);
	ULONG ulEmailTag2 = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[1],PT_UNICODE);
	ULONG ulEmailTag3 = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[2],PT_UNICODE);
	MAPIFreeBuffer(lpNamedPropTags);

	SizedSPropTagArray(3, mcols) = {3,{ulEmailTag1,ulEmailTag2,ulEmailTag3}	};

	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	for (int i = 0; i < (int)pcount; ++i)
	{
		int iFound = 0;
		EMailItem *p = (EMailItem *)calloc(1,sizeof(EMailItem));
		if  (props[i].ulPropTag == ulEmailTag1)
		{
			iFound = 1;
			p->iType = EMAIL_TYPE_HOME;
		}
		else if  (props[i].ulPropTag == ulEmailTag2)
		{
			iFound = 1;
			p->iType = EMAIL_TYPE_WORK;
		}
		else if (props[i].ulPropTag ==  ulEmailTag3)
		{
			iFound = 1;
			p->iType = EMAIL_TYPE_OTHER;
		}

		if (iFound == 0)
		{
			free(p);
			continue;
		}

		if (pContact->pEMails == NULL)
		{
			pContact->pEMails = new list<EMailItem*>;
		}

		_tcscpy(p->szEmail,props[i].Value.lpszW);
		pContact->pEMails->push_back(p);
	}

	MAPIFreeBuffer(props);
	return PE_RET_OK;
}


int ContactsHelper::GetContactPhone(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{

	SizedSPropTagArray(13, mcols) = {13,
	{
		PR_HOME_TELEPHONE_NUMBER,
			PR_MOBILE_TELEPHONE_NUMBER,
			PR_BUSINESS_TELEPHONE_NUMBER,
			PR_BUSINESS_FAX_NUMBER,
			PR_PAGER_TELEPHONE_NUMBER,
			PR_OTHER_TELEPHONE_NUMBER,
			PR_CALLBACK_TELEPHONE_NUMBER,
			PR_CAR_TELEPHONE_NUMBER,
			PR_ISDN_NUMBER,
			PR_RADIO_TELEPHONE_NUMBER,
			PR_TELEX_NUMBER,
			PR_TTYTDD_PHONE_NUMBER,
			PR_ASSISTANT_TELEPHONE_NUMBER
	}
	};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	for (int i = 0; i < (int)pcount; ++i)
	{
		int iPhoneType = PHONE_TYPE_UNKONW;
		switch (props[i].ulPropTag)
		{
		case  PR_HOME_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_HOME;
				break;
			}
		case  PR_MOBILE_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_MOBILE;
				break;
			}
		case  PR_BUSINESS_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_WORK;
				break;
			}
		case  PR_BUSINESS_FAX_NUMBER:
			{
				iPhoneType = PHONE_TYPE_FAX_WORK;
				break;
			}
		case  PR_PAGER_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_PAGER;
				break;
			}
		case  PR_OTHER_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_OTHER;
				break;
			}
		case  PR_CALLBACK_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_CALLBACK;
				break;
			}
		case  PR_CAR_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_CAR;
				break;
			}
		case  PR_ISDN_NUMBER:
			{
				iPhoneType = PHONE_TYPE_ISDN;
				break;
			}
		case  PR_RADIO_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_RADIO;
				break;
			}
		case  PR_TELEX_NUMBER:
			{
				iPhoneType = PHONE_TYPE_TELEX;
				break;
			}
		case  PR_TTYTDD_PHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_TTY_TDD;
				break;
			}
		case  PR_ASSISTANT_TELEPHONE_NUMBER:
			{
				iPhoneType = PHONE_TYPE_ASSISTANT;
				break;
			}
		default:
			{
				iPhoneType = PHONE_TYPE_UNKONW;
				break;
			}
		}

		if (iPhoneType == PHONE_TYPE_UNKONW)
		{
			continue;
		}

		if (pContact->pPhones == NULL)
		{
			pContact->pPhones = new list<PhoneItem*>;
		}

		PhoneItem *p = (PhoneItem *)calloc(1,sizeof(PhoneItem));
		p->iPhoneType = iPhoneType;
		_tcscpy(p->szPhoneNo,props[i].Value.lpszW);
		pContact->pPhones->push_back(p);

	}

	MAPIFreeBuffer(props);
	return PE_RET_OK;;
}


int ContactsHelper::GenContactProperty(LPMESSAGE lpMessage)
{
	MAPINAMEID  rgnmid[2] = {0};
	LPMAPINAMEID rgpnmid[2] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&MOBITNT_ITEM_ID_GUID;
	rgnmid[0].ulKind = MNID_STRING;
	rgnmid[0].Kind.lpwstrName = _T("MOBITNT_ITEM_ID");
	rgpnmid[0] = &rgnmid[0];

	rgnmid[1].lpguid = (LPGUID)&MOBITNT_CONTACTS_TYPE_GUID;
	rgnmid[1].ulKind = MNID_STRING;
	rgnmid[1].Kind.lpwstrName = _T("MOBITNT_CONTACTS_TYPE");
	rgpnmid[1] = &rgnmid[1];

	HRESULT hRes = lpMessage->GetIDsFromNames(2,(LPMAPINAMEID*) &rgpnmid,MAPI_CREATE,&lpNamedPropTags);
	if (FAILED(hRes))
	{
		return PE_RET_FAIL;
	}

	m_lTag4ItemID 		= CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[0],PT_LONG);
	m_lTag4ContactType   = CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[1],PT_LONG);

	MAPIFreeBuffer(lpNamedPropTags);
	return PE_RET_OK;
}


bool ContactsHelper::GetContactType(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	SizedSPropTagArray(1, mcols) = {1,{m_lTag4ContactType}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	if (props[0].ulPropTag != m_lTag4ContactType )
	{
		MAPIFreeBuffer(props);
		return false;
	}

	pContact->iIsSim = props[0].Value.l;

	MAPIFreeBuffer(props);

	return true;
}



bool ContactsHelper::GetContactProperty(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	SizedSPropTagArray(2, mcols) = {2,{m_lTag4ItemID,m_lTag4ContactType}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		return false;
	}

	if (pcount != 2 || props[0].ulPropTag != m_lTag4ItemID || props[1].ulPropTag !=  m_lTag4ContactType  )
	{
		MAPIFreeBuffer(props);
		return false;
	}

	pContact->ulContactID = props[0].Value.l;

	MAPIFreeBuffer(props);

	return true;
}

bool ContactsHelper::SetContactProperty(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	SPropValue spvProps[2] = {0};

	spvProps[0].ulPropTag = m_lTag4ItemID;
	spvProps[0].Value.l = pContact->ulContactID;

	spvProps[1].ulPropTag = m_lTag4ContactType;
	spvProps[1].Value.l = pContact->iIsSim;

	HRESULT hRes = lpMessage->SetProps(2, spvProps, NULL);
	if (FAILED(hRes))
	{
		return false;
	}

	return true;
}



int ContactsHelper::IsContactExistInPhoneFolder(CONTACT_ITEM *pContact)
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
		return PE_DIFFERENT_ITEM;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		return PE_DIFFERENT_ITEM;
	}

	hr = S_OK;
	ULONG iCount = 0;
	pTable->GetRowCount(0,&iCount);

	int iDiff = PE_DIFFERENT_ITEM;

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
			return PE_DIFFERENT_ITEM;
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
			return PE_DIFFERENT_ITEM;
		}

		CONTACT_ITEM *pTmpContact = (CONTACT_ITEM*)calloc(1,sizeof(CONTACT_ITEM));
		if (PE_RET_OK == GetContactName(pMsg,pTmpContact))
		{
			GetContactEMail(pMsg,pTmpContact);
			GetContactPhone(pMsg,pTmpContact);
			GetContactAddress(pMsg,pTmpContact);

			iDiff = ContactCompare(pContact,pTmpContact);
		}

		FreeContactItem(pTmpContact);
		free(pTmpContact);

		if (iDiff == PE_SAME_ITEM)
		{
			pMsg->Release();
			FreeProws(pRows);
			pTable->Release();
			return PE_SAME_ITEM;
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	return iDiff;
}


int ContactsHelper::SaveContact2OutLook(CONTACT_ITEM *pContact)
{
	int iRet = IsContactExistInPhoneFolder(pContact);
	if (iRet == PE_SAME_ITEM)
	{
		return PE_RET_OK;
	}

	PECore::SetSyncStat(PE_SYNC_CONTACT);

	LPMESSAGE lpMessage = 0;
	// create a message and set its properties
	HRESULT hRes = m_pFolder->CreateMessage(0,0,&lpMessage);
	if (FAILED(hRes))
	{
		PEWriteLog(_T("AddDiary:createMessage failed"));
		return PE_RET_FAIL;
	}

	GenContactProperty(lpMessage);
	SetContactProperty(lpMessage,pContact);

	SetContactName(lpMessage,pContact);
	SetContactFileUnder(lpMessage,pContact);
	SetContactPhone(lpMessage,pContact);
	SetContactEMail(lpMessage,pContact);
	SetContactAddress(lpMessage,pContact);
	SetContactOrg(lpMessage,pContact);
	SetContactIM(lpMessage,pContact);
	SetContactNotes(lpMessage,pContact);
	SetContactBirthDay(lpMessage,pContact);
	SetContactWebSite(lpMessage,pContact);

	SaveImage(lpMessage,pContact);

	hRes = lpMessage->SaveChanges(FORCE_SAVE);
	if (FAILED(hRes))
	{
		PEWriteLog(_T("SaveContact2OutLook failed!"));
	}

	lpMessage->Release();

	return PE_RET_OK;
}

int ContactsHelper::Export()
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
	int iTotal = 1;
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

		if (PE_RET_OK != GenContactProperty(pMsg) )
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			pMsg->Release();
			return PE_RET_FAIL;
		}

		CONTACT_ITEM contact;
		memset(&contact,0,sizeof(contact));
		if (!GetContactProperty(pMsg,&contact))
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (PE_RET_OK == GetContactName(pMsg,&contact))
		{
			GetContactEMail(pMsg,&contact);
			GetContactPhone(pMsg,&contact);
			GetContactAddress(pMsg,&contact);
			GetContactOrg(pMsg,&contact);
			GetContactWebSite(pMsg,&contact);

			int iCode = 0;
			DeviceAgent::RestoreContact(&contact,iCode);
			FreeContactItem(&contact);
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}


int ContactsHelper::DeleteContactByEntryID(ULONG cb,LPBYTE lpb)
{
	SBinary bin;
	bin.cb = cb;
	bin.lpb = lpb;

	DeleteItemByEntryID(bin);

	return PE_RET_OK;
}



BOOL ContactsHelper::openFolder()
{
	m_pFolder = NULL;
	m_iDefaultFolder = 1;
	
	TCHAR szPhoneName[255];
	if (PECore::GetCurPhoneName(szPhoneName) != PE_RET_OK)
	{
		return FALSE;
	}

	TCHAR szFolderName[1024];
	GenFolderName(szFolderName,_T("Contact"),szPhoneName);

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenContacts(FALSE);
	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("open calendar root folder failed!"));
		return FALSE;
	}

	HRESULT hr = pRootFolder->CreateFolder(	FOLDER_GENERIC,
		szFolderName,
		MOBITNT_CONTACT_FOLDER_COMMENT,
		NULL,
		MAPI_UNICODE|OPEN_IF_EXISTS,
		&m_pFolder);
	pRootFolder->Release();
	if (FAILED(hr))
	{
		return FALSE;
	}

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag =  PR_CONTAINER_CLASS;
	spvProps[0].Value.lpszW = _T("IPF.Contact");

	hr = m_pFolder->SetProps(1,spvProps,NULL);
	if (FAILED(hr))
	{
		PEWriteLog(_T("Faild to set contact folder property"));
	}

	hr = m_pFolder->SaveChanges(0);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;
}


void ContactsHelper::FreeContactItem(CONTACT_ITEM *pContact)
{
	free(pContact->szNotes);

	if (pContact->pPhones)
	{
		list<PhoneItem*>::iterator it1;
		for (it1 = pContact->pPhones->begin(); it1 != pContact->pPhones->end(); ++it1)
		{ 
			free(*it1);
		}
		pContact->pPhones->clear();

		delete pContact->pPhones;
	}

	if (pContact->pEMails)
	{
		list<EMailItem*>::iterator it1;
		for (it1 = pContact->pEMails->begin(); it1 != pContact->pEMails->end(); ++it1)
		{ 
			free(*it1);
		}
		pContact->pEMails->clear();

		delete pContact->pEMails;
	}

	if (pContact->pAddrs)
	{
		list<AddrItem*>::iterator it1;
		for (it1 = pContact->pAddrs->begin(); it1 != pContact->pAddrs->end(); ++it1)
		{ 
			free(*it1);
		}
		pContact->pAddrs->clear();

		delete pContact->pAddrs;
	}

	if (pContact->pIMs)
	{
		list<ImItem*>::iterator it1;
		for (it1 = pContact->pIMs->begin(); it1 != pContact->pIMs->end(); ++it1)
		{ 
			free(*it1);
		}
		pContact->pIMs->clear();

		delete pContact->pIMs;
	}

	
	if (pContact->pSites)
	{
		list<WebSiteItem*>::iterator it1;
		for (it1 = pContact->pSites->begin(); it1 != pContact->pSites->end(); ++it1)
		{ 
			free(*it1);
		}
		pContact->pSites->clear();

		delete pContact->pSites;
	}

	free(pContact->PhotoData);
	pContact->iPhotoDataLen = 0;
}


int ContactsHelper::FindContactItemByEntryID(TCHAR *tszEntryID,CONTACT_ITEM  *pContact)
{
	SBinary bin;
	TCHAR2SBinary(tszEntryID,bin);
	LPMESSAGE pMsg = FindItemByEntryID(bin);
	if (pMsg == NULL)
	{
		return PE_RET_FAIL;
	}

	memset(pContact,0,sizeof(CONTACT_ITEM));

	GenContactProperty(pMsg);
	if (!GetContactProperty(pMsg,pContact))
	{
		pMsg->Release();
		return PE_RET_FAIL;
	}

	GetContactName(pMsg,pContact);

	pMsg->Release();
	return PE_RET_OK;

}


void ContactsHelper::RemoveDupliacte()
{

}

extern int g_iStartPercet;
extern int g_iEndPercet;

extern int g_iCanSync;

int ContactsHelper::SyncContactFromFolder2Phone()
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
	int iTotal = 1;
	if (iCount < 1)
	{
		pTable->Release();
		pTable = NULL;
		return PE_RET_OK;
	}

	float fStep = (float)(g_iEndPercet - g_iStartPercet)/(float)iCount;

	for (ULONG i = 1; i <= iCount; ++i)
	{
		hr = pTable->QueryRows(1,0, &pRows);
		if (FAILED(hr))
		{
			continue;
		}
		g_iStartPercet = (int)(fStep * i);
		if (g_iStartPercet > 100)
		{
			g_iStartPercet = 100;
		}

		PECore::SendSyncState2UI(PE_SYNC_CONTACT_2_PHONE,100,g_iStartPercet);

		if (pRows->cRows != 1)
		{
			pTable->Release();
			pTable = NULL;
			return PE_RET_FAIL;
		}

		if (!g_iCanSync)
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

		CONTACT_ITEM *pContact = (CONTACT_ITEM*)calloc(1,sizeof(CONTACT_ITEM));
		if ( GenContactProperty(pMsg) != PE_RET_OK)
		{
			goto NEXTTURN;
		}

#if 0
		if (GetContactType(pMsg,pContact) && pContact->iIsSim)
		{
			//don't sync sim contact for now
			goto NEXTTURN;
		}
#endif

		if (PE_RET_OK != GetContactName(pMsg,pContact))
		{
			goto NEXTTURN;
		}

		GetContactEMail(pMsg,pContact);
		GetContactPhone(pMsg,pContact);
		GetContactAddress(pMsg,pContact);
		GetContactOrg(pMsg,pContact);
		GetContactWebSite(pMsg,pContact);

		int iPhoneRetCode = 0;
		if (m_iDefaultFolder)
		{
			//phone folder sync directtly,it's meaningless to compare same folder
			GetContactProperty(pMsg,pContact);
			DeviceAgent::RestoreContact(pContact,iPhoneRetCode);
			if (iPhoneRetCode == 21)
			{
				//the contact exists in outlook phone folder but removed from phone
				//delete it on outlook side as well
				PEWriteLog(_T("Delete contact removed from phone"));
				DeleteItemByEntryID(pRows->aRow[0].lpProps[0].Value.bin);
				FreeContactItem(pContact);
				free(pContact);
	
				pMsg->Release();
				FreeProws(pRows);
				pTable->Release();
				pTable = NULL;
				return PR_RET_TRY_AGAIN;
			}
		}
		else if (IsContactExistInPhoneFolder(pContact) == PE_DIFFERENT_ITEM)
		{
			DeviceAgent::RestoreContact(pContact,iPhoneRetCode);
		}

NEXTTURN:
		FreeContactItem(pContact);
		free(pContact);

		pMsg->Release();
		FreeProws(pRows);
	}


	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}


int ContactsHelper::SyncContact2Phone(TCHAR *szSubFolder)
{
	if (!szSubFolder)
	{
		if (openFolder())
		{
			int iRet = PE_RET_OK;
			do
			{
				iRet = SyncContactFromFolder2Phone();
			}while (iRet == PR_RET_TRY_AGAIN);
			return PE_RET_OK;
		}
		return PE_RET_FAIL;
	}

#if 0
	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenContacts(FALSE);
	if (!pRootFolder)
	{
		return PE_RET_FAIL;
	}

	if (_tcscmp(szSubFolder,CONTACT_ROOT_FOLDER_NAME) == 0)
	{
		m_pFolder = pRootFolder;
		SyncContactFromFolder2Phone();
		return PE_RET_OK;	
	}
#endif

	LPMAPIFOLDER pSubFolder= m_pMAPIMgr->OpenFolderByPath(szSubFolder);
	if(pSubFolder)
	{
		m_pFolder = pSubFolder;
		int iRet = PE_RET_OK;
		do
		{
			iRet = SyncContactFromFolder2Phone();
		}while (iRet == PR_RET_TRY_AGAIN);
	}

	return PE_RET_OK;	
}



int ContactsHelper::SaveImage(LPMESSAGE lpMessage,CONTACT_ITEM  *pContact)
{
	if (pContact->iPhotoDataLen < 1 || pContact->PhotoData == NULL)
	{
		return PE_RET_OK;
	}

	LPATTACH lpAttach;
	ULONG attachNum = 1;
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
		lpStream->Write(pContact->PhotoData,pContact->iPhotoDataLen,&ulCount);
		lpStream->Commit(STGC_DEFAULT);
	}

	//Stream copied, Set the filename properties
	TCHAR szName[255];
	_stprintf(szName,_T("CImage.png"));

	SPropValue spvAttach[5]; 
	spvAttach[0].ulPropTag = PR_ATTACH_METHOD; 
	spvAttach[0].Value.l = ATTACH_BY_VALUE; 
	spvAttach[1].ulPropTag = PR_RENDERING_POSITION; 
	spvAttach[1].Value.l = -1; 
	spvAttach[2].ulPropTag = PR_ATTACH_EXTENSION;
	spvAttach[2].Value.lpszW = L".png";
	spvAttach[3].ulPropTag = PR_ATTACHMENT_CONTACTPHOTO;
	spvAttach[3].Value.b = TRUE;
	spvAttach[4].ulPropTag = PR_DISPLAY_NAME; 
	spvAttach[4].Value.lpszW = szName;
	hr = lpAttach->SetProps(5,(LPSPropValue)&spvAttach, NULL);

	hr = lpAttach->SaveChanges(NULL);
	if(!SUCCEEDED(hr))
	{
		return PE_RET_FAIL;
	}

	MAPINAMEID	rgnmid[1] = {0};
	LPMAPINAMEID rgpnmid[1] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&PSETID_Address;
	rgnmid[0].ulKind = MNID_ID;
	rgnmid[0].Kind.lID = PidLidHasPicture;
	rgpnmid[0] = &rgnmid[0];


	HRESULT hRes = m_pFolder->GetIDsFromNames(1,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("SaveImage:GetIDsFromNames failed"));
		return PE_RET_OK;
	}

	SPropValue spvProps[1] = {0};
	spvProps[0].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidWorkAddressStreet],PT_BOOLEAN);
	spvProps[0].Value.b = TRUE;

	MAPIFreeBuffer(lpNamedPropTags);

	hRes = lpMessage->SetProps(1,spvProps, NULL);
	if (FAILED(hRes))
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("SaveImage:SetProps hRes is %d"),hRes);
		PEWriteLog(szInfo);
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}


#if 0
int ContactsHelper::EmulateContact()
{
	for (int i = 0; i < 1000; ++i)
	{
		CONTACT_ITEM c;
		memset(&c,0,sizeof(c));

		_stprintf(c.szFamilyName,_T("Family%d"),i);
		_stprintf(c.szMiddleName,_T("Middle%d"),i);
		_stprintf(c.szGivenName,_T("Given%d"),i);

		_stprintf(c.szOrgCompany,_T("Company%d"),i);
		_stprintf(c.szOrgTitle,_T("Title%d"),i);

		c.pAddrs = new list<AddrItem*>;
		c.pAddrs->clear();
		for (int k = 1; k <= 3; ++k)
		{
			AddrItem *p = (AddrItem*)calloc(1,sizeof(AddrItem));
			p->iAddrType = k;

			int iPhoneNo = rand();
			_stprintf(p->szAddrCity,_T("city%d"),iPhoneNo);
			_stprintf(p->szAddrFormatAddress,_T("FormatAddress%d"),iPhoneNo);
			_stprintf(p->szAddrPostCode,_T("PostCode%d"),iPhoneNo);
			_stprintf(p->szAddrRegion,_T("Region%d"),iPhoneNo);
			_stprintf(p->szAddrStreet,_T("Street%d"),iPhoneNo);
			_stprintf(p->szCountry,_T("Country%d"),iPhoneNo);	
			c.pAddrs->push_back(p);
		}

		c.pPhones = new list<PhoneItem*>;
		c.pPhones->clear();
		for (int m = 1; m <= 20; ++m)
		{
			PhoneItem *p = (PhoneItem*)calloc(1,sizeof(PhoneItem));
			p->iPhoneType = m;

			int iPhoneNo = rand();
			_stprintf(p->szPhoneNo,_T("%d"),iPhoneNo);
			c.pPhones->push_back(p);
		}

		c.pEMails = new list<EMailItem*>;
		c.pEMails->clear();
		for (int n = 1; n <= 4; ++n)
		{
			EMailItem *p = (EMailItem*)calloc(1,sizeof(EMailItem));
			p->iType = n;

			int iPhoneNo = rand();
			_stprintf(p->szEmail,_T("%d@a.c"),iPhoneNo);
			c.pEMails->push_back(p);
		}

		LPMESSAGE lpMessage = 0;
		// create a message and set its properties
		HRESULT hRes = m_pFolder->CreateMessage(0,0,&lpMessage);
		if (FAILED(hRes))
		{
			PEWriteLog(_T("AddDiary:createMessage failed"));
			return PE_RET_FAIL;
		}

		GenContactProperty(lpMessage);
		SetContactProperty(lpMessage,&c);

		SetContactName(lpMessage,&c);
		SetContactPhone(lpMessage,&c);
		SetContactEMail(lpMessage,&c);
		SetContactAddress(lpMessage,&c);
		SetContactOrg(lpMessage,&c);
		SetContactIM(lpMessage,&c);
		SaveImage(lpMessage,&c);

		hRes = lpMessage->SaveChanges(FORCE_SAVE);
		if (FAILED(hRes))
		{
			PEWriteLog(_T("SaveContact2OutLook failed!"));
		}

		lpMessage->Release();
	}

	return PE_RET_OK;
}
#endif

list<TCHAR*> ContactsHelper::GetSubFolderList()
{
	list<TCHAR*> subFolderList;
	subFolderList.clear();

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenRootFolder(0);

	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("OpenContacts failed!"));
		return subFolderList;
	}

	subFolderList = m_pMAPIMgr->GetSubFolderList(pRootFolder,0,FOLDER_TYPE_CONTACT);

	list<TCHAR*>::iterator it;
	for (it = subFolderList.begin(); it != subFolderList.end();)
	{ 
		TCHAR *szName = (TCHAR*)*it;
		if (m_sFolderName == szName)
		{
			free(szName);
			it = subFolderList.erase(it);

		}
		else
		{
			++it;
		}
	}

	RELEASE(pRootFolder);

	return subFolderList;
}

list<CONTACT_ITEM*> ContactsHelper::GetContactList()
{
	list<CONTACT_ITEM*> contactlist;

	contactlist.clear();

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
		return contactlist;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		return contactlist;
	}

	hr = S_OK;
	ULONG iCount = 0;
	pTable->GetRowCount(0,&iCount);
	int iTotal = 1;
	if (iCount < 1)
	{
		pTable->Release();
		pTable = NULL;
		return contactlist;
	}

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
			return contactlist;
		}

		if (!g_iCanSync)
		{
			pTable->Release();
			pTable = NULL;
			return contactlist;
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
			return contactlist;
		}

		CONTACT_ITEM *pContact = (CONTACT_ITEM*)calloc(1,sizeof(CONTACT_ITEM));
		if (PE_RET_OK == GetContactName(pMsg,pContact))
		{
			GetContactPhone(pMsg,pContact);
			contactlist.push_back(pContact);
		}

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return contactlist;


}



