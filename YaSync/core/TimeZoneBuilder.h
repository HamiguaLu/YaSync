#ifndef __TIME_ZONE_BUILDER_HDR__
#define	__TIME_ZONE_BUILDER_HDR__

const WORD	TZRULE_FLAG_RECUR_CURRENT_TZREG		= 0x0001;
const WORD	TZRULE_FLAG_EFFECTIVE_TZREG			= 0x0002;
const BYTE	TZ_BIN_VERSION_MAJOR				= 0x02;
const BYTE	TZ_BIN_VERSION_MINOR				= 0x01;


#pragma pack(1)
typedef struct _TimeZoneStruct
{
	DWORD lBias; // offset from GMT
	DWORD lStandardBias; // offset from bias during standard time
	DWORD lDaylightBias; // offset from bias during daylight time
	WORD wStandardYear;
	SYSTEMTIME stStandardDate; // time to switch to standard time
	WORD wDaylightYear;
	SYSTEMTIME stDaylightDate; // time to switch to daylight time
}TimeZoneStruct;


// TZRule
// =====================
//   This structure represents both a description when a daylight.
//   savings shift occurs, and in addition, the year in which that
//   timezone rule came into effect.
//
typedef struct _TZRule
{
	BYTE bMajorVersion;
	BYTE bMinorVersion;
	WORD wReserved;
	WORD wTZRuleFlags;
	WORD wYear;
	BYTE X[14];
	DWORD lBias; // offset from GMT
	DWORD lStandardBias; // offset from bias during standard time
	DWORD lDaylightBias; // offset from bias during daylight time
	SYSTEMTIME stStandardDate; // time to switch to standard time
	SYSTEMTIME stDaylightDate; // time to switch to daylight time
}TZRule;

// TimeZoneDefinitionStruct
// =====================
//   This represents an entire timezone including all historical, current
//   and future timezone shift rules for daylight savings time, etc.
//
typedef struct _TimeZoneDefinitionStruct
{
	BYTE bMajorVersion;
	BYTE bMinorVersion;
	WORD cbHeader;
	WORD wReserved;
	WORD cchKeyName;
	LPWSTR szKeyName;
	WORD cRules;
	TZRule* lpTZRule;
}TimeZoneDefinitionStruct;

#pragma pack()

int BuildTimeZoneStruct(TCHAR *szTimeZone,ULONG* lpcbTZ,LPBYTE* lppTZ);
int BuildTimeZoneDefinition(TCHAR *szTimeZone,int iRecu,ULONG* lpcbTZ,LPBYTE* lppTZ);




void TimeZoneDefinitionStruct2Bin(TimeZoneDefinitionStruct *tzs,ULONG* lpcbTZ,LPBYTE* lppTZ);


#endif


