#include "stdafx.h"
#include "CalendarHelper.h"
#include "PECore.h"
#include "AppTools.h"
#include "mapiutil.h"
#include "PEInterface.h"
#include <initguid.h>
#include "RecurBuilder.h"
#include "TimeZoneBuilder.h"

#include "TrayHelper.h"

DEFINE_OLEGUID(PSETID_Appointment, MAKELONG(0x2000+(0x02),0x0006),0,0);
DEFINE_GUID(PSETID_Meeting, MAKELONG(0xDA90, 0x6ED8),0x450B, 0x101B, 0x98, 0xDA, 0x0, 0xAA, 0x0, 0x3F, 0x13, 0x05);
DEFINE_OLEGUID(PSETID_Common, MAKELONG(0x2000+(0x8),0x0006),0,0);


// The array is the list of named properties to be set.
ULONG aulCalendarProps[] = {
	PidLidLocation,
	PidLidAppointmentStartWhole,
	PidLidAppointmentEndWhole,
	PidLidRecurring,
	PidLidClipStart,
	PidLidClipEnd,
	PidLidRecurrenceType,
	PidLidAppointmentRecur,
	// PSETID_Meeting
	PidLidIsRecurring,

	// PSETID_Common
	PidLidCommonStart,
	PidLidCommonEnd,
};
#define ulCalendarProps (sizeof(aulCalendarProps)/sizeof(aulCalendarProps [0]))

enum _CALPROPER{
	p_PidLidLocation,
	p_PidLidAppointmentStartWhole,
	p_PidLidAppointmentEndWhole,
	p_PidLidRecurring,
	p_PidLidClipStart,
	p_PidLidClipEnd,
	p_PidLidRecurrenceType,
	p_PidLidAppointmentRecur,

	p_PidLidIsRecurring,

	p_PidLidCommonStart,
	p_PidLidCommonEnd,

	p_PR_SUBJECT_W,
	p_PR_START_DATE,
	p_PR_END_DATE,
	p_PR_MESSAGE_CLASS_W,
	NUM_PROPS
};


ULONG aulReminderProps[] = {
	PidLidReminderSet,
	PidLidReminderDelta,
	PidLidReminderTime,
	PidLidReminderSignalTime,
};
#define ulReminderProps (sizeof(aulReminderProps)/sizeof(aulReminderProps [0]))

enum _RMD_PROPS{
	p_PidLidReminderSet,
	p_PidLidReminderDelta,
	p_PidLidReminderTime,
	p_PidLidReminderSignalTime,

	NUM_RMD_PROPS
};



ULONG aulTimeZoneProps[] = {
	PidLidTimeZoneStruct,
	PidLidTimeZoneDescription,
	PidLidAppointmentTimeZoneDefinitionRecur,
	PidLidAppointmentTimeZoneDefinitionStartDisplay,
	PidLidAppointmentTimeZoneDefinitionEndDisplay,
};
#define ulTimeZoneProps (sizeof(aulTimeZoneProps)/sizeof(aulTimeZoneProps [0]))

enum _TM_PROPS{
	p_PidLidTimeZoneStruct,
	p_PidLidTimeZoneDescription,
	p_PidLidAppointmentTimeZoneDefinitionRecur,
	p_PidLidAppointmentTimeZoneDefinitionStartDisplay,
	p_PidLidAppointmentTimeZoneDefinitionEndDisplay,

	NUM_TM_PROPS
};




ULONG ulFirstMeetingProp = p_PidLidIsRecurring;
ULONG ulFirstCommonProp = p_PidLidCommonStart;

HRESULT CalendarHelper::SetCalendarReminder(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{
	HRESULT hRes = S_OK;

	MAPINAMEID	rgnmid[ulReminderProps] = {0};
	LPMAPINAMEID rgpnmid[ulReminderProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulReminderProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Common;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulReminderProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(ulReminderProps,(LPMAPINAMEID*) &rgpnmid,	NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return E_FAIL;
	}

	// Since we know in advance which props we'll be setting, we can statically declare most of the structures involved and save expensive MAPIAllocateBuffer calls
	SPropValue spvProps[NUM_RMD_PROPS] = {0};
	spvProps[p_PidLidReminderSet].ulPropTag         = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderSet],	PT_BOOLEAN);
	spvProps[p_PidLidReminderDelta].ulPropTag		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderDelta],	PT_LONG);
	spvProps[p_PidLidReminderTime].ulPropTag		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderTime],	PT_SYSTIME);
	spvProps[p_PidLidReminderSignalTime].ulPropTag  = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderSignalTime],PT_SYSTIME);

	int iCount = 1;
	spvProps[0].Value.b = (USHORT) pCal->dwHasAlarm;
	if ( pCal->dwHasAlarm)
	{
		spvProps[p_PidLidReminderDelta].Value.l =  pCal->dwReminderDelta;
		spvProps[p_PidLidReminderTime].Value.ft =  pCal->ftDTSTART;
		spvProps[p_PidLidReminderSignalTime].Value.ft =  pCal->ftDTSTART;

		time_t timeSpan = 60 *  pCal->dwReminderDelta;;
		timeSpan = (-1)*timeSpan * ONE_SECOND;//in minutes
		spvProps[p_PidLidReminderSignalTime].Value.ft =  pCal->ftDTSTART;
		AddFileTimeSpan(spvProps[p_PidLidReminderSignalTime].Value.ft, timeSpan);

		iCount = 4;
	}

	hRes = lpMessage->SetProps(iCount,spvProps, NULL);
	if (FAILED(hRes))
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("SetCalendarReminder:SetProps hRes is %d"),hRes);
		PEWriteLog(szInfo);
		PEWriteLog(_T("SetCalendarReminder:SetProps failed"));
	}

	return hRes;
}


int CalendarHelper::GetCalendarReminder(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{
	HRESULT hRes = S_OK;

	MAPINAMEID	rgnmid[ulReminderProps] = {0};
	LPMAPINAMEID rgpnmid[ulReminderProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulReminderProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Common;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulReminderProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(ulReminderProps,(LPMAPINAMEID*) &rgpnmid, NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return E_FAIL;
	}


	ULONG ulReminderSetTag    		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderSet],	PT_BOOLEAN);
	ULONG ulReminderDeltaTag		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderDelta],	PT_LONG);
	ULONG ulReminderTimeTag 		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderTime],	PT_SYSTIME);
	ULONG ulReminderSignalTimeTag 	= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderSignalTime],PT_SYSTIME);
	SizedSPropTagArray(4, mcols) = {4,{ulReminderSetTag,ulReminderDeltaTag,ulReminderTimeTag,ulReminderSignalTimeTag}};
	ULONG pcount;
	SPropValue *props=0;

	MAPIFreeBuffer(lpNamedPropTags);

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (pcount != 4)
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag != ulReminderSetTag
		||props[1].ulPropTag != ulReminderDeltaTag
		||props[2].ulPropTag != ulReminderTimeTag
		||props[3].ulPropTag != ulReminderSignalTimeTag)
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	pCal->dwHasAlarm	= props[0].Value.b;
	pCal->dwReminderDelta= props[1].Value.l;

	MAPIFreeBuffer(props);
	return PE_RET_OK;;
}



HRESULT CalendarHelper::SetCalendarTimeZone(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{
	HRESULT hRes = S_OK;

	MAPINAMEID	rgnmid[ulTimeZoneProps] = {0};
	LPMAPINAMEID rgpnmid[ulTimeZoneProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulTimeZoneProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Appointment;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulTimeZoneProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(NUM_TM_PROPS,(LPMAPINAMEID*) &rgpnmid,	NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return E_FAIL;
	}

	MAPIFreeBuffer(lpNamedPropTags);

	// Since we know in advance which props we'll be setting, we can statically declare most of the structures involved and save expensive MAPIAllocateBuffer calls
	SPropValue spvProps[NUM_TM_PROPS] = {0};
	spvProps[p_PidLidTimeZoneStruct].ulPropTag	= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidTimeZoneStruct],	PT_BINARY);
	spvProps[p_PidLidTimeZoneDescription].ulPropTag	= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidTimeZoneDescription],PT_UNICODE);
	spvProps[p_PidLidAppointmentTimeZoneDefinitionRecur].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentTimeZoneDefinitionRecur],PT_BINARY);
	spvProps[p_PidLidAppointmentTimeZoneDefinitionStartDisplay].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentTimeZoneDefinitionStartDisplay],PT_BINARY);
	spvProps[p_PidLidAppointmentTimeZoneDefinitionEndDisplay].ulPropTag = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentTimeZoneDefinitionEndDisplay],PT_BINARY);

	BuildTimeZoneStruct( pCal->szTimeZone,&spvProps[p_PidLidTimeZoneStruct].Value.bin.cb,&spvProps[p_PidLidTimeZoneStruct].Value.bin.lpb);
	spvProps[p_PidLidTimeZoneDescription].Value.lpszW = _T("");
	BuildTimeZoneDefinition( pCal->szTimeZone, pCal->dwIsRecur,&spvProps[p_PidLidAppointmentTimeZoneDefinitionRecur].Value.bin.cb,&spvProps[p_PidLidAppointmentTimeZoneDefinitionRecur].Value.bin.lpb);
	BuildTimeZoneDefinition( pCal->szTimeZone, pCal->dwIsRecur,&spvProps[p_PidLidAppointmentTimeZoneDefinitionStartDisplay].Value.bin.cb,&spvProps[p_PidLidAppointmentTimeZoneDefinitionStartDisplay].Value.bin.lpb);
	BuildTimeZoneDefinition( pCal->szTimeZone, pCal->dwIsRecur,&spvProps[p_PidLidAppointmentTimeZoneDefinitionEndDisplay].Value.bin.cb,&spvProps[p_PidLidAppointmentTimeZoneDefinitionEndDisplay].Value.bin.lpb);

	hRes = lpMessage->SetProps(NUM_TM_PROPS,spvProps, NULL);
	if (FAILED(hRes))
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("SetCalendarTimeZone:SetProps hRes is %d"),hRes);
		PEWriteLog(szInfo);
	}

	free(spvProps[p_PidLidTimeZoneStruct].Value.bin.lpb);
	free(spvProps[p_PidLidAppointmentTimeZoneDefinitionRecur].Value.bin.lpb);
	free(spvProps[p_PidLidAppointmentTimeZoneDefinitionStartDisplay].Value.bin.lpb);
	free(spvProps[p_PidLidAppointmentTimeZoneDefinitionEndDisplay].Value.bin.lpb);

	return hRes;
}



#if 0
int CalendarHelper::GetCalendarTimeZone(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{

	HRESULT hRes = S_OK;

	MAPINAMEID	rgnmid[ulTimeZoneProps] = {0};
	LPMAPINAMEID rgpnmid[ulTimeZoneProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulTimeZoneProps ; ++i)
	{
		rgnmid[i].lpguid = (LPGUID)&PSETID_Appointment;

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulTimeZoneProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(NUM_TM_PROPS,(LPMAPINAMEID*) &rgpnmid,	NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return E_FAIL;
	}



	ULONG ulReminderSetTag    		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderSet],	PT_BOOLEAN);
	ULONG ulReminderDeltaTag		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderDelta],	PT_LONG);
	ULONG ulReminderTimeTag 		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderTime],	PT_SYSTIME);
	ULONG ulReminderSignalTimeTag 	= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidReminderSignalTime],PT_SYSTIME);
	SizedSPropTagArray(4, mcols) = {4,{ulReminderSetTag,ulReminderDeltaTag,ulReminderTimeTag,ulReminderSignalTimeTag}};
	ULONG pcount;
	SPropValue *props=0;

	MAPIFreeBuffer(lpNamedPropTags);

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (pcount != 4)
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag != ulReminderSetTag
		||props[1].ulPropTag != ulReminderDeltaTag
		||props[2].ulPropTag != ulReminderTimeTag
		||props[3].ulPropTag != ulReminderSignalTimeTag)
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	pCal->dwHasAlarm	= props[0].Value.b;
	pCal->dwReminderDelta= props[1].Value.l;

	MAPIFreeBuffer(props);
	return PE_RET_OK;;
}
#endif



HRESULT CalendarHelper::SetCalendarPropertyEx(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{
	if (!m_pFolder)
	{
		PEWriteLog(_T("AddDiary:m_pFolder is null"));
		return MAPI_E_INVALID_PARAMETER;
	}

	HRESULT hRes = S_OK;

	MAPINAMEID	rgnmid[ulCalendarProps] = {0};
	LPMAPINAMEID rgpnmid[ulCalendarProps] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulCalendarProps ; ++i)
	{
		if (i < ulFirstMeetingProp)
		{
			rgnmid[i].lpguid = (LPGUID)&PSETID_Appointment;
		}
		else if (i < ulFirstCommonProp)
		{
			rgnmid[i].lpguid = (LPGUID)&PSETID_Meeting;
		}
		else
		{
			rgnmid[i].lpguid = (LPGUID)&PSETID_Common;
		}

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulCalendarProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(ulCalendarProps,(LPMAPINAMEID*) &rgpnmid,	NULL,&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("AddDiary:GetIDsFromNames failed"));
		return E_FAIL;
	}

	// Since we know in advance which props we'll be setting, we can statically declare most of the structures involved and save expensive MAPIAllocateBuffer calls
	SPropValue spvProps[NUM_PROPS] = {0};
	spvProps[p_PidLidLocation].ulPropTag                                  = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidLocation],PT_UNICODE);
	spvProps[p_PidLidAppointmentStartWhole].ulPropTag                     = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentStartWhole],PT_SYSTIME);
	spvProps[p_PidLidAppointmentEndWhole].ulPropTag                       = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentEndWhole],PT_SYSTIME);
	spvProps[p_PidLidRecurring].ulPropTag                                 = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidRecurring],PT_BOOLEAN);
	spvProps[p_PidLidClipStart].ulPropTag                                 = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidClipStart],PT_SYSTIME);
	spvProps[p_PidLidClipEnd].ulPropTag                                   = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidClipEnd],PT_SYSTIME);
	spvProps[p_PidLidRecurrenceType].ulPropTag                            = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidRecurrenceType],PT_LONG);
	spvProps[p_PidLidAppointmentRecur].ulPropTag                          = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentRecur],PT_BINARY);

	spvProps[p_PidLidIsRecurring].ulPropTag                               = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidIsRecurring],PT_BOOLEAN);

	spvProps[p_PidLidCommonStart].ulPropTag                               = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidCommonStart],PT_SYSTIME);
	spvProps[p_PidLidCommonEnd].ulPropTag                                 = CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidCommonEnd],PT_SYSTIME);

	spvProps[p_PR_SUBJECT_W].ulPropTag		    = PR_SUBJECT_W;
	spvProps[p_PR_START_DATE].ulPropTag         = PR_START_DATE;
	spvProps[p_PR_END_DATE].ulPropTag           = PR_END_DATE;
	spvProps[p_PR_MESSAGE_CLASS_W].ulPropTag    = PR_MESSAGE_CLASS_W;

	spvProps[p_PidLidLocation].Value.lpszW =  pCal->szLocation;
	spvProps[p_PidLidAppointmentStartWhole].Value.ft =  pCal->ftDTSTART;
	spvProps[p_PidLidAppointmentEndWhole].Value.ft =  pCal->ftDTEND;
	spvProps[p_PidLidRecurring].Value.b = false;

	spvProps[p_PidLidClipStart].Value.ft =  pCal->ftDTSTART;
	spvProps[p_PidLidClipEnd].Value.ft =  pCal->ftDTEND;

	spvProps[p_PidLidCommonStart].Value.ft =  pCal->ftDTSTART;
	spvProps[p_PidLidCommonEnd].Value.ft =  pCal->ftDTEND;

	spvProps[p_PR_SUBJECT_W].Value.lpszW =  pCal->szTitle;
	spvProps[p_PR_START_DATE].Value.ft =  pCal->ftDTSTART;
	spvProps[p_PR_END_DATE].Value.ft =  pCal->ftDTEND;
	spvProps[p_PR_MESSAGE_CLASS_W].Value.lpszW = L"IPM.Appointment";

	byte *pRRBuf = NULL;
	if ( pCal->dwIsRecur)
	{
		int iBufLen = 0;
		int iRecType = 0;
		int iEndType = 0;

		PEWriteLog("RRULE:");
		PEWriteLog( pCal->szRRULE);

		if (PE_RET_OK == ParseRRULE(pCal,&pRRBuf,iBufLen,iRecType,iEndType))
		{
			spvProps[p_PidLidAppointmentRecur].Value.bin.cb = iBufLen;
			spvProps[p_PidLidAppointmentRecur].Value.bin.lpb = pRRBuf;

			spvProps[p_PidLidRecurrenceType].Value.l = iRecType;
			spvProps[p_PidLidIsRecurring].Value.b = true;

			FILETIME ft = spvProps[p_PidLidClipStart].Value.ft;
			GetLocalMidnightTime(ft);
			spvProps[p_PidLidClipStart].Value.ft = ft;

			if (iEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				spvProps[p_PidLidClipEnd].Value.ft.dwHighDateTime = 0x0CB2E579;
				spvProps[p_PidLidClipEnd].Value.ft.dwLowDateTime = 0x49B47A00;
			}
			else
			{
				FILETIME ft = spvProps[p_PidLidClipEnd].Value.ft;
				GetLocalMidnightTime(ft);
				spvProps[p_PidLidClipEnd].Value.ft = ft;
			}
		}
		else
		{
			PEWriteLog("ParseRRULE failed");
		}
	}

	hRes = lpMessage->SetProps(NUM_PROPS,spvProps, NULL);
	if (SUCCEEDED(hRes))
	{
		SetCalendarReminder(lpMessage,pCal);
#if 0
		hRes = lpMessage->SaveChanges(FORCE_SAVE);
		if (FAILED(hRes))
		{
			PEWriteLog(_T("AddCalendar:SaveChanges failed"));
			TCHAR szInfo[255];
			_stprintf(szInfo,_T("AddCalendar:SaveChanges hRes is %d"),hRes);
			PEWriteLog(szInfo);
		}
#endif
	}
	else
	{
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("AddCalendar:SetProps hRes is %d"),hRes);
		PEWriteLog(szInfo);
		PEWriteLog(_T("AddCalendar:SetProps failed"));
	}

	MAPIFreeBuffer(lpNamedPropTags);

	free(pRRBuf);

	return hRes;
}


int CalendarHelper::CalendarCompare(CAL_EVT_ITEM *pCal1,CAL_EVT_ITEM *pCal2)
{
	if ( pCal1->ulCalendarID !=  pCal2->ulCalendarID)
	{
		return PE_RET_FAIL;
	}

	if ( pCal1->dwRecurrenceType !=  pCal2->dwRecurrenceType)
	{
		return PE_RET_FAIL;
	}

	return PE_RET_OK;

}


int CalendarHelper::GetCalendarPropertyEx(LPMESSAGE lpMessage,CAL_EVT_ITEM* pCal)
{
	HRESULT hRes = S_OK;

	MAPINAMEID	rgnmid[ulCalendarProps];
	LPMAPINAMEID rgpnmid[ulCalendarProps];
	LPSPropTagArray lpNamedPropTags = NULL;

	ULONG i = 0;
	for (i = 0 ; i < ulCalendarProps ; ++i)
	{
		if (i < ulFirstMeetingProp)
		{
			rgnmid[i].lpguid = (LPGUID)&PSETID_Appointment;
		}
		else if (i < ulFirstCommonProp)
		{
			rgnmid[i].lpguid = (LPGUID)&PSETID_Meeting;
		}
		else
		{
			rgnmid[i].lpguid = (LPGUID)&PSETID_Common;
		}

		rgnmid[i].ulKind = MNID_ID;
		rgnmid[i].Kind.lID = aulCalendarProps[i];
		rgpnmid[i] = &rgnmid[i];
	}

	hRes = m_pFolder->GetIDsFromNames(
		ulCalendarProps,
		(LPMAPINAMEID*) &rgpnmid,
		NULL,
		&lpNamedPropTags);
	if (FAILED(hRes) || lpNamedPropTags == NULL)
	{
		PEWriteLog(_T("GetCalendarPropertyEx:GetIDsFromNames failed"));
		return PE_RET_FAIL;
	}

	ULONG ulRecurringTag			= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidRecurring],PT_BOOLEAN);//PidLidRecurring
	ULONG ulRecurringTypeTag		= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidRecurrenceType],PT_LONG);//PidLidRecurrenceType
	ULONG ulStartTimeTag			= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentStartWhole],PT_SYSTIME);//PidLidAppointmentStartWhole
	ULONG ulEndTimeTag				= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentEndWhole],PT_SYSTIME);//PidLidAppointmentEndWhole
	ULONG ulLocationTag				= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidLocation],PT_UNICODE);//PidLidLocation
	ULONG ulAptRecurTag				= CHANGE_PROP_TYPE(lpNamedPropTags->aulPropTag[p_PidLidAppointmentRecur],PT_BINARY);//PidLidAppointmentRecur

	MAPIFreeBuffer(lpNamedPropTags);

	SizedSPropTagArray(7, mcols) = {7,{PR_SUBJECT,ulRecurringTag,ulRecurringTypeTag,ulStartTimeTag,ulEndTimeTag,ulLocationTag,ulAptRecurTag}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		MAPIFreeBuffer(props);
		PEWriteLog(_T("GetCalendarPropertyEx failed1"));
		return PE_RET_FAIL;
	}

	if (pcount != 7)
	{
		MAPIFreeBuffer(props);
		PEWriteLog(_T("GetCalendarPropertyEx failed2"));
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag != PR_SUBJECT
		||props[1].ulPropTag != ulRecurringTag
		||props[2].ulPropTag != ulRecurringTypeTag
		||props[3].ulPropTag != ulStartTimeTag
		||props[4].ulPropTag != ulEndTimeTag
		||props[5].ulPropTag != ulLocationTag)
	{
		MAPIFreeBuffer(props);
		PEWriteLog(_T("GetCalendarPropertyEx failed3"));
		return PE_RET_FAIL;
	}

	_tcscpy( pCal->szTitle,props[0].Value.lpszW);
	pCal->dwRecurring			= props[1].Value.l;
	pCal->dwRecurrenceType		= props[2].Value.l;
	pCal->ftDTSTART				= props[3].Value.ft;
	pCal->ftDTEND				= props[4].Value.ft;
	_tcscpy( pCal->szLocation,props[5].Value.lpszW);

	if (props[6].ulPropTag == ulAptRecurTag)
	{
		if (props[6].Value.bin.cb > 0)
		{
			if (PE_RET_FAIL == ParseRecurBin( pCal->szRRULE,props[6].Value.bin.lpb,props[6].Value.bin.cb))
			{
				MAPIFreeBuffer(props);
				PEWriteLog(_T("ParseRecurBin failed"));
				PEWriteLog(_T("GetCalendarPropertyEx failed4"));
				return PE_RET_FAIL;
			}
		}
	}

	MAPIFreeBuffer(props);
	return PE_RET_OK;;
}


int CalendarHelper::GenCalendarProperty(LPMESSAGE lpMessage)
{
	MAPINAMEID  rgnmid[2] = {0};
	LPMAPINAMEID rgpnmid[2] = {0};
	LPSPropTagArray lpNamedPropTags = NULL;

	rgnmid[0].lpguid = (LPGUID)&MOBITNT_ITEM_ID_GUID;
	rgnmid[0].ulKind = MNID_STRING;
	rgnmid[0].Kind.lpwstrName = _T("MOBITNT_ITEM_ID");
	rgpnmid[0] = &rgnmid[0];

	rgnmid[1].lpguid = (LPGUID)&MOBITNT_CALENDAR_ID_GUID;
	rgnmid[1].ulKind = MNID_STRING;
	rgnmid[1].Kind.lpwstrName = _T("MOBITNT_CALENDAR_ID");
	rgpnmid[1] = &rgnmid[1];

	HRESULT hRes = lpMessage->GetIDsFromNames(2,(LPMAPINAMEID*) &rgpnmid,MAPI_CREATE,&lpNamedPropTags);
	if (FAILED(hRes))
	{
		return PE_RET_FAIL;
	}

	m_lTag4ItemID = CHANGE_PROP_TYPE(lpNamedPropTags[0].aulPropTag[0],PT_LONG);
	m_lTag4CalendarID = CHANGE_PROP_TYPE(lpNamedPropTags[1].aulPropTag[0],PT_LONG);

	MAPIFreeBuffer(lpNamedPropTags);
	return PE_RET_OK;
}

int CalendarHelper::GetCalendarProperty(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{
	SizedSPropTagArray(2, mcols) = {2,{m_lTag4ItemID,m_lTag4CalendarID}};
	ULONG pcount;
	SPropValue *props=0;

	HRESULT hr = lpMessage->GetProps((SPropTagArray*)&mcols,0,&pcount,&props);
	if (FAILED(hr))
	{
		PEWriteLog(_T("GetCalendarProperty failed"));
		return PE_RET_FAIL;
	}

	if (props[0].ulPropTag != m_lTag4ItemID || props[1].ulPropTag != m_lTag4CalendarID )
	{
		MAPIFreeBuffer(props);
		return PE_RET_FAIL;
	}

	pCal->ulEvtID = props[0].Value.l;
	pCal->ulCalendarID = props[1].Value.l;

	MAPIFreeBuffer(props);

	return PE_RET_OK;
}

bool CalendarHelper::SetCalendarProperty(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal)
{
	SPropValue spvProps[2] = {0};

	spvProps[0].ulPropTag = m_lTag4ItemID;
	spvProps[1].ulPropTag = m_lTag4CalendarID;


	spvProps[0].Value.l =  pCal->ulEvtID;
	spvProps[1].Value.l =  pCal->ulCalendarID;

	HRESULT hRes = lpMessage->SetProps(2, spvProps, NULL);
	if (FAILED(hRes))
	{
			PEWriteLog(_T("SetCalendarProperty failed"));
		return false;
	}

	return true;
}



int CalendarHelper::IsCalendarExistInPhoneFolder(CAL_EVT_ITEM *pCal)
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

		if (PE_RET_OK != GenCalendarProperty(pMsg) )
		{
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			pMsg->Release();
			return PE_DIFFERENT_ITEM;
		}

		CAL_EVT_ITEM *pCalendar = (CAL_EVT_ITEM *)calloc(1,sizeof(CAL_EVT_ITEM));
		if (!GetCalendarPropertyEx(pMsg,pCalendar))
		{
			pMsg->Release();
			FreeProws(pRows);
			free(pCalendar);
			continue;
		}

		if (pCalendar->ulCalendarID !=  pCal->ulCalendarID)
		{
			pMsg->Release();
			FreeProws(pRows);
			free(pCalendar);
			continue;
		}

		if (pCalendar->dwAllDay !=  pCal->dwAllDay)
		{
			pMsg->Release();
			FreeProws(pRows);
			free(pCalendar);
			continue;
		}

		if (pCalendar->dwHasAlarm !=  pCal->dwHasAlarm)
		{
			pMsg->Release();
			FreeProws(pRows);
			free(pCalendar);
			continue;
		}

		if (pCalendar->dwIsRecur !=  pCal->dwIsRecur)
		{
			pMsg->Release();
			FreeProws(pRows);
			free(pCalendar);
			continue;
		}

		if (pCalendar->dwReminderDelta !=  pCal->dwReminderDelta)
		{
			pMsg->Release();
			FreeProws(pRows);
			free(pCalendar);
			continue;
		}

		if ( PECompareFileTime(pCalendar->ftDTSTART ,  pCal->ftDTSTART) != PE_RET_CMP_EQUAL)
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (PECompareFileTime(pCalendar->ftDTEND ,  pCal->ftDTEND) != PE_RET_CMP_EQUAL)
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (_tcscmp(pCalendar->szTitle, pCal->szTitle) != 0)
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (_tcscmp(pCalendar->szDesc, pCal->szDesc) != 0)
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		if (_tcscmp(pCalendar->szLocation, pCal->szLocation) != 0)
		{
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}


		pCalendar->bin.cb = pRows->aRow[0].lpProps[0].Value.bin.cb;
		if (pCalendar->bin.cb > 255)
		{
			PEWriteLog(_T("PB is too long!"));
			pMsg->Release();
			FreeProws(pRows);
			continue;
		}

		memcpy(pCalendar->bin.pb , pRows->aRow[0].lpProps[0].Value.bin.lpb,pCalendar->bin.cb);
		FreeProws(pRows);
		pTable->Release();
		pTable = NULL;
		pMsg->Release();
		return PE_SAME_ITEM;
	}

	pTable->Release();
	pTable = NULL;
	return PE_DIFFERENT_ITEM;
}


int CalendarHelper::SaveCalendar2OutLook(CAL_EVT_ITEM *pCal)
{
	int iRet = IsCalendarExistInPhoneFolder(pCal);
	if (iRet == PE_SAME_ITEM)
	{
		return PE_RET_OK;
	}

	PECore::SetSyncStat(PE_SYNC_CALENDAR);

	LPMESSAGE lpMessage = 0;
	// create a message and set its properties
	HRESULT hRes = m_pFolder->CreateMessage(0,0,&lpMessage);
	if (FAILED(hRes))
	{
		PEWriteLog(_T("AddDiary:createMessage failed"));
		return PE_RET_FAIL;
	}

	GenCalendarProperty(lpMessage);
	SetCalendarProperty(lpMessage, pCal);
	SetCalendarPropertyEx(lpMessage, pCal);
	SetCalendarTimeZone(lpMessage, pCal);

	hRes = lpMessage->SaveChanges(FORCE_SAVE);
	if (FAILED(hRes))
	{
		PEWriteLog(_T("AddCalendar:SaveChanges failed"));
		TCHAR szInfo[255];
		_stprintf(szInfo,_T("AddCalendar:SaveChanges hRes is %d"),hRes);
		PEWriteLog(szInfo);
	}

	lpMessage->Release();

	return PE_RET_OK;
}



int CalendarHelper::GetLastSyncTime(TCHAR *szCalendarID,FILETIME &ftLastSyncTime)
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




int CalendarHelper::Export()
{
	return PE_RET_OK;
}


int CalendarHelper::DeleteCalendarByEntryID(ULONG cb,LPBYTE lpb)
{
	SBinary bin;
	bin.cb = cb;
	bin.lpb = lpb;

	DeleteItemByEntryID(bin);

	return PE_RET_OK;
}



BOOL CalendarHelper::openFolder()
{
	m_pFolder = NULL;
	m_iDefaultFolder = 1;

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenSpecialFolder(PR_IPM_APPOINTMENT_ENTRYID,FALSE);
	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("open calendar root folder failed!"));
		return FALSE;
	}

	PE_PHONE_SETTING *pSetting2 = PECore::GetCurPhone();
	if (!pSetting2)
	{	
		PEWriteLog(_T("CurPhone is NULL!"));
		return FALSE;
	}

	TCHAR szFolderName[255];
	GenFolderName(szFolderName,_T("Calendar"),pSetting2->szPhoneName);
	HRESULT hr = pRootFolder->CreateFolder(	FOLDER_GENERIC,
		szFolderName,
		MOBITNT_CALL_FOLDER_COMMENT,
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
	spvProps[0].Value.lpszW = _T("IPF.Appointment");

	hr = m_pFolder->SetProps(1,spvProps,NULL);
	if (FAILED(hr))
	{
		PEWriteLog(_T("Faild to set call folder property"));
	}

	hr = m_pFolder->SaveChanges(0);
	if (FAILED(hr))
	{
		return FALSE;
	}

	return TRUE;
}


int CalendarHelper::FindCalendarItemByEntryID(TCHAR *tszEntryID,CAL_EVT_ITEM *pCal)
{
	SBinary bin;
	TCHAR2SBinary(tszEntryID,bin);
	LPMESSAGE pMsg = FindItemByEntryID(bin);
	if (pMsg == NULL)
	{
		return PE_RET_FAIL;
	}

	GenCalendarProperty(pMsg);
	if (!GetCalendarProperty(pMsg,pCal))
	{
		pMsg->Release();
		return PE_RET_FAIL;
	}

	if (!GetCalendarPropertyEx(pMsg,pCal))
	{
		pMsg->Release();
		return PE_RET_FAIL;
	}

	pMsg->Release();
	return PE_RET_OK;

}


int CalendarHelper::GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft)
{
	CAL_EVT_ITEM calendar;
	memset(&calendar,0,sizeof(calendar));

#if 0
	memset(&calendar,0,sizeof(calendar));

	GenCalendarProperty(pMsg);
	if (!GetCalendarProperty(pMsg,calendar))
	{
		PEWriteLog(_T("GetCalendarProperty failed"));
		return PE_RET_FAIL;
	}
#endif

	if (PE_RET_OK != GetCalendarPropertyEx(pMsg,&calendar))
	{
		PEWriteLog(_T("GetCalendarPropertyEx failed"));
		return PE_RET_FAIL;
	}

	ft =  calendar.ftDTSTART;
	return PE_RET_OK;
}


list<TCHAR*> CalendarHelper::GetSubFolderList()
{
	PEWriteLog(_T("Enter"));

	list<TCHAR*> subFolderList;
	subFolderList.clear();

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenRootFolder(0);

	if (pRootFolder == NULL)
	{
		PEWriteLog(_T("OpenContacts failed!"));
		return subFolderList;
	}

	subFolderList = m_pMAPIMgr->GetSubFolderList(pRootFolder,0,FOLDER_TYPE_CALENDAR);

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

extern int g_iStartPercet;
extern int g_iEndPercet;
extern int g_iCanSync;

int CalendarHelper::SyncCalendarFromFolder2Phone(FILETIME& ftSyncAfter)
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
		PEWriteLog(_T("SyncCalendarFromFolder2Phone failed1"));
		return PE_RET_FAIL;
	}

	hr = pTable->SetColumns((LPSPropTagArray)&Columns, 0);
	if ((FAILED(hr)))
	{
		pTable->Release();
		pTable = NULL;
		PEWriteLog(_T("SyncCalendarFromFolder2Phone failed2"));
		return PE_RET_FAIL;
	}

	hr = S_OK;
	ULONG iCount = 0;
	hr = pTable->GetRowCount(0,&iCount);
	if (FAILED(hr))
	{
		pTable->Release();
		pTable = NULL;
		PEWriteLog(_T("SyncCalendarFromFolder2Phone failed3"));
		return PE_RET_OK;
	}

	int iTotal = 1;
	if (iCount < 1)
	{
		pTable->Release();
		pTable = NULL;
		PEWriteLog(_T("SyncCalendarFromFolder2Phone failed4"));
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
		
		if (pRows->cRows != 1)
		{
			continue;
		}

		g_iStartPercet = (int)(fStep * i);
		if (g_iStartPercet > 100)
		{
			g_iStartPercet = 100;
		}

		PECore::SendSyncState2UI(PE_SYNC_CAL_2_PHONE,100,g_iStartPercet);

		if (!g_iCanSync)
		{
			pTable->Release();
			pTable = NULL;
			PEWriteLog(_T("SyncCalendarFromFolder2Phone cancel now"));
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

		if (PE_RET_OK != GenCalendarProperty(pMsg))
		{
			PEWriteLog(_T("GenCalendarProperty failed"));
			FreeProws(pRows);
			pTable->Release();
			pTable = NULL;
			return PE_RET_FAIL;
		}


		CAL_EVT_ITEM *pCalendar = (CAL_EVT_ITEM*)calloc(1,sizeof(CAL_EVT_ITEM));

		GetCalendarProperty(pMsg,pCalendar);
		GetCalendarPropertyEx(pMsg,pCalendar);

		if (PECompareFileTime(pCalendar->ftDTSTART,ftSyncAfter) == PE_RET_CMP_LOWER)
		{
			free(pCalendar);
			pMsg->Release();
			FreeProws(pRows);
			PEWriteLog(_T("Time passed"));
			continue;
		}

		GetCalendarReminder(pMsg,pCalendar);

		if (m_iDefaultFolder)
		{
			//phone folder sync directtly,it's meaningless to compare same folder
			DeviceAgent::SaveCalendar2Phone(pCalendar);
		}
		else if (IsCalendarExistInPhoneFolder(pCalendar) == PE_DIFFERENT_ITEM)
		{
			DeviceAgent::SaveCalendar2Phone(pCalendar);
		}

		free(pCalendar);

		pMsg->Release();
		FreeProws(pRows);
	}

	pTable->Release();
	pTable = NULL;
	return PE_RET_OK;
}


int CalendarHelper::SyncCalendar2Phone(TCHAR *szSubFolder,FILETIME& ftSyncAfter)
{
	PEWriteLog(_T("SyncCalendar2Phone"));

	if (szSubFolder == NULL)
	{
		if (openFolder())
		{
			SyncCalendarFromFolder2Phone(ftSyncAfter);
			return PE_RET_OK;
		}

		return PE_RET_FAIL;
	}
#if 0
	if (_tcslen(szSubFolder) < 1)
	{
		PEWriteLog(_T("szSubFolder is empty"));
		return PE_RET_FAIL;
	}

	LPMAPIFOLDER pRootFolder = m_pMAPIMgr->OpenSpecialFolder(PR_IPM_APPOINTMENT_ENTRYID,FALSE);
	if (!pRootFolder)
	{
		PEWriteLog(_T("Can not open calendar root folder"));
		return PE_RET_FAIL;
	}

	if (_tcscmp(szSubFolder,CALENDAR_ROOT_FOLDER_NAME) == 0)
	{
		m_pFolder = pRootFolder;
		PEWriteLog(_T("sync calendar root folder"));
		SyncCalendarFromFolder2Phone(ftSyncAfter);
		return PE_RET_OK;	
	}

	PEWriteLog(szSubFolder);
#endif

	LPMAPIFOLDER pSubFolder= m_pMAPIMgr->OpenFolderByPath(szSubFolder);
	if(pSubFolder)
	{
		m_pFolder = pSubFolder;
		SyncCalendarFromFolder2Phone(ftSyncAfter);
	}

	return PE_RET_OK;
}





