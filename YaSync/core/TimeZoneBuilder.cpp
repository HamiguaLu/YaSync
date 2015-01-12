#include "stdafx.h"
#include "PEProtocol.h"
#include "TimeZoneBuilder.h"


// Allocates with new, free with delete
// The following code builds a time zone structure
int BuildTimeZoneStruct(TCHAR *szTimeZone,ULONG* lpcbTZ,LPBYTE* lppTZ)
{
	TIME_ZONE_INFORMATION tzInfo;
	memset(&tzInfo,0,sizeof(tzInfo));
	DWORD dwTZType = GetTimeZoneInformation(&tzInfo);

#if 1
	CString sInfo;
	sInfo.Format(_T("TimeZone:type %d,name:%s,AndroidTimeZone:%s"),dwTZType,tzInfo.StandardName,szTimeZone);
	PEWriteLog(sInfo.GetBuffer());
	sInfo.ReleaseBuffer();
#endif

	TimeZoneStruct *s = (TimeZoneStruct*)calloc(1,sizeof(TimeZoneStruct));
	s->lBias = tzInfo.Bias;
	s->lDaylightBias = tzInfo.DaylightBias;
	s->lStandardBias = tzInfo.StandardBias;
	s->stDaylightDate = tzInfo.DaylightDate;
	s->stStandardDate = tzInfo.StandardDate;
	s->wDaylightYear = tzInfo.DaylightDate.wYear;
	s->wStandardYear = tzInfo.StandardDate.wYear;

	*lpcbTZ = sizeof(TimeZoneStruct);
	*lppTZ = (LPBYTE)s;

	return PE_RET_OK;
}

int BuildTZRule(TZRule *ptzRule,WORD wTZRFlags,WORD wYear,TCHAR *szTimeZone)
{
	TIME_ZONE_INFORMATION tzInfo;
	DWORD dwTZType = GetTimeZoneInformation(&tzInfo);

	memset(ptzRule,0,sizeof(TZRule));
	ptzRule->bMajorVersion = TZ_BIN_VERSION_MAJOR;
	ptzRule->bMinorVersion = TZ_BIN_VERSION_MINOR;
	ptzRule->wReserved	   = 0x003E;
	
	ptzRule->wTZRuleFlags   = wTZRFlags;
	ptzRule->wYear			= wYear;
	ptzRule->lBias			= tzInfo.Bias;
	ptzRule->lStandardBias	= tzInfo.StandardBias; // offset from bias during standard time
	ptzRule->lDaylightBias	= tzInfo.DaylightBias; // offset from bias during daylight time
	ptzRule->stStandardDate	= tzInfo.StandardDate; // time to switch to standard time
	ptzRule->stDaylightDate	= tzInfo.DaylightDate;

	return PE_RET_OK;
}




// Allocates with new, free with delete
// The following code builds a time zone definition with two rules
int BuildTimeZoneDefinition(TCHAR *szTimeZone,int iRecu,ULONG* lpcbTZ,LPBYTE* lppTZ)
{
	TimeZoneDefinitionStruct *s = (TimeZoneDefinitionStruct*)calloc(1,sizeof(TimeZoneDefinitionStruct));
	s->bMajorVersion = TZ_BIN_VERSION_MAJOR;
	s->bMinorVersion = TZ_BIN_VERSION_MINOR;
	s->wReserved = 2;

	s->cchKeyName = (WORD)_tcslen(szTimeZone);
	s->szKeyName = szTimeZone;

	s->cbHeader = sizeof(WORD)*2 + sizeof(WCHAR)*s->cchKeyName + sizeof(WORD);

	s->cRules = 1;

	TZRule r;

	WORD wTzFlags = TZRULE_FLAG_EFFECTIVE_TZREG;
	if (iRecu)
	{
		wTzFlags = TZRULE_FLAG_EFFECTIVE_TZREG | TZRULE_FLAG_RECUR_CURRENT_TZREG;
	}

	BuildTZRule(&r,wTzFlags,2010,szTimeZone);

	s->lpTZRule = &r;

	TimeZoneDefinitionStruct2Bin(s,lpcbTZ,lppTZ);
	free(s);
	
	return PE_RET_OK;
}


void TimeZoneDefinitionStruct2Bin(TimeZoneDefinitionStruct *s,ULONG* lpcbTZ,LPBYTE* lppTZ)
{
	int iLen = sizeof(TimeZoneDefinitionStruct) - 2 * sizeof(char*) + s->cchKeyName*sizeof(WCHAR) + s->cRules * sizeof(TZRule);
	*lpcbTZ = iLen;

	LPBYTE pb = (LPBYTE)calloc(1,*lpcbTZ);
	*lppTZ = pb;

	memcpy(pb,&s->bMajorVersion,1);
	pb += sizeof(BYTE);
	
	memcpy(pb,&s->bMinorVersion,1);
	pb += sizeof(BYTE);

	memcpy(pb,&s->cbHeader,sizeof(WORD));
	pb += sizeof(WORD);

	memcpy(pb,&s->wReserved,sizeof(WORD));
	pb += sizeof(WORD);

	memcpy(pb,&s->cchKeyName,sizeof(WORD));
	pb += sizeof(WORD);

	memcpy(pb,s->szKeyName,s->cchKeyName * sizeof(WCHAR));
	pb += s->cchKeyName * sizeof(WCHAR);

	memcpy(pb,&s->cRules,sizeof(WORD));
	pb += sizeof(WORD);

	for (int i = 0; i < s->cRules; ++i)
	{
		memcpy(pb,s->lpTZRule,sizeof(TZRule));
		pb += sizeof(TZRule);
	}
}





