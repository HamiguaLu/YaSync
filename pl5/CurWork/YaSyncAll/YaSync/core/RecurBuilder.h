#ifndef __RECUR_BUILDER_HDR__
#define	__RECUR_BUILDER_HDR__


enum __NonPropFlag
{
	flagSearchFlag = 0x10000, // ensure that all flags in the enum are > 0xffff
	flagSearchState,
	flagTableStatus,
	flagTableType,
	flagObjectType,
	flagSecurityVersion,
	flagSecurityInfo,
	flagACEFlag,
	flagACEType,
	flagACEMaskContainer,
	flagACEMaskNonContainer,
	flagACEMaskFreeBusy,
	flagStreamFlag,
	flagRestrictionType,
	flagBitmask,
	flagRelop,
	flagAccountType,
	flagBounceCode,
	flagOPReply,
	flagOpForward,
	flagFuzzyLevel,
	flagRulesVersion,
	flagNotifEventType,
	flagTableEventType,
	flagTZRule,
	flagRuleFlag,
	flagExtendedFolderFlagType,
	flagExtendedFolderFlag,
	flagRecurFrequency,
	flagPatternType,
	flagCalendarType,
	flagDOW,
	flagN,
	flagEndType,
	flagFirstDOW,
	flagOverrideFlags,
	flagReportTagVersion,
	flagGlobalObjectIdMonth,
	flagOneOffEntryId,
	flagEntryId0,
	flagEntryId1,
	flagMessageDatabaseObjectType,
	flagContabVersion,
	flagContabType,
	flagContabIndex,
	flagExchangeABVersion,
	flagMDBVersion,
	flagMDBFlag,
	flagPropDefVersion,
	flagPDOFlag,
	flagVarEnum,
	flagInternalType,
	flagPersistID,
	flagElementID,
	flagWABEntryIDType,
	flagWebViewVersion,
	flagWebViewType,
	flagWebViewFlags,
	flagFolderType,
	flagFieldCap,
	flagCcsf,
	flagIet,
	flagV2Magic,
	flagV2Version,
};



// [MS-OXOCAL].pdf
#define dispidApptRecur 0x8216
#define ARO_SUBJECT          0x0001
#define ARO_MEETINGTYPE      0x0002
#define ARO_REMINDERDELTA    0x0004
#define ARO_REMINDER         0x0008
#define ARO_LOCATION         0x0010
#define ARO_BUSYSTATUS       0x0020
#define ARO_ATTACHMENT       0x0040
#define ARO_SUBTYPE          0x0080
#define ARO_APPTCOLOR        0x0100
#define ARO_EXCEPTIONAL_BODY 0x0200

enum IdGroup
{
	IDC_RCEV_PAT_ORB_DAILY = 0x200A,
	IDC_RCEV_PAT_ORB_WEEKLY,
	IDC_RCEV_PAT_ORB_MONTHLY,
	IDC_RCEV_PAT_ORB_YEARLY,
	IDC_RCEV_PAT_ERB_END=0x2021,
	IDC_RCEV_PAT_ERB_AFTERNOCCUR,
	IDC_RCEV_PAT_ERB_NOEND,
};


#define rectypeNone					0x00000000
#define rectypeDaily				0x00000001
#define rectypeWeekly				0x00000002
#define rectypeMonthly				0x00000003
#define rectypeYearly				0x00000004

enum _RPTDAY
{
	rptMinute = 0,
	rptWeek,
	rptMonth,
	rptMonthNth,
	rptMonthEnd,
	rptHjMonth = 10,
	rptHjMonthNth,
	rptHjMonthEnd
};

const ULONG rpn1st = 1;
const ULONG rpn2nd = 2;
const ULONG rpn3rd = 3;
const ULONG rpn4th = 4;
const ULONG rpnLast = 5;

const ULONG rdfSun = 0x01;
const ULONG rdfMon = 0x02;
const ULONG rdfTue = 0x04;
const ULONG rdfWed = 0x08;
const ULONG rdfThu = 0x10;
const ULONG rdfFri = 0x20;
const ULONG rdfSat = 0x40;

#pragma pack(1)

struct AptHeader
{
	WORD ReaderVersion;
	WORD WriterVersion;
	WORD RecurFrequency;
	WORD PatternType;
};

struct AptRecurPatternDailyBin
{
	WORD ReaderVersion;
	WORD WriterVersion;
	WORD RecurFrequency;
	WORD PatternType;
	WORD CalendarType;
	DWORD FirstDateTime;
	DWORD Period;
	DWORD SlidingFlag;
	DWORD EndType;
	DWORD OccurrenceCount;
	DWORD FirstDOW;
	DWORD DeletedInstanceCount;
	DWORD ModifiedInstanceCount;
	DWORD StartDate;
	DWORD EndDate;;
	DWORD ReaderVersion2;
	DWORD WriterVersion2;
	DWORD StartTimeOffset;
	DWORD EndTimeOffset;
	WORD  ExceptionCount;
	DWORD ReservedBlock1Size;
	DWORD ReservedBlock2Size;
};


struct AptRecurPatternMonthNthBin
{
	WORD ReaderVersion;
	WORD WriterVersion;
	WORD RecurFrequency;
	WORD PatternType;
	WORD CalendarType;
	DWORD FirstDateTime;
	DWORD Period;
	DWORD SlidingFlag;
	DWORD PatternTypeSpecific;
	DWORD PatternTypeSpecificNth;
	DWORD EndType;
	DWORD OccurrenceCount;
	DWORD FirstDOW;
	DWORD DeletedInstanceCount;
	DWORD ModifiedInstanceCount;
	DWORD StartDate;
	DWORD EndDate;;
	DWORD ReaderVersion2;
	DWORD WriterVersion2;
	DWORD StartTimeOffset;
	DWORD EndTimeOffset;
	WORD  ExceptionCount;
	DWORD ReservedBlock1Size;
	DWORD ReservedBlock2Size;
};



struct AptRecurPatternBin
{
	WORD ReaderVersion;
	WORD WriterVersion;
	WORD RecurFrequency;
	WORD PatternType;
	WORD CalendarType;
	DWORD FirstDateTime;
	DWORD Period;
	DWORD SlidingFlag;
	DWORD PatternTypeSpecific;
	DWORD EndType;
	DWORD OccurrenceCount;
	DWORD FirstDOW;
	DWORD DeletedInstanceCount;
	DWORD ModifiedInstanceCount;
	DWORD StartDate;
	DWORD EndDate;;
	DWORD ReaderVersion2;
	DWORD WriterVersion2;
	DWORD StartTimeOffset;
	DWORD EndTimeOffset;
	WORD  ExceptionCount;
	DWORD ReservedBlock1Size;
	DWORD ReservedBlock2Size;
};

#pragma pack()


int ParseRRULE(CAL_EVT_ITEM *pCal,byte **pBuf,int &iBufLen,int &iRecType,int &iEndType);

int ParseRecurBin(TCHAR *tszRRULE,byte *pBuf,int iBufLen);

int CalculateDailyRecurrence(AptRecurPatternDailyBin *pRecur);
int CalculateWeeklyRecurrence(AptRecurPatternBin *pRecur);
int CalculateMonthlyOrYearlyRecurrence(AptRecurPatternBin *pRecur);
int CalculateMonthlyNthRecurrence(AptRecurPatternMonthNthBin *pRecur);

#endif