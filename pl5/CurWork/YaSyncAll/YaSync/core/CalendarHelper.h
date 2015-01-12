#pragma once
#include "PEProtocol.h"
#include "MAPIEx.h"


#include "OutLookHelper.h"

#include "DeviceAgent.h"

#define PidLidLocation 											0x8208
#define PidLidAppointmentStartWhole 							0x820D
#define PidLidAppointmentEndWhole 								0x820E
#define PidLidClipStart											0x8235
#define PidLidClipEnd											0x8236
#define PidLidTimeZoneStruct 									0x8233
#define PidLidTimeZoneDescription 								0x8234
#define PidLidAppointmentTimeZoneDefinitionRecur				0x8260
#define PidLidAppointmentTimeZoneDefinitionStartDisplay 		0x825E
#define PidLidAppointmentTimeZoneDefinitionEndDisplay 			0x825F

#define PidLidAppointmentRecur									0x8216
#define PidLidRecurring 										0x8223
#define PidLidIsRecurring										0x0005
#define PidLidRecurrenceType									0x8231

#define PidLidReminderDelta										0x8501
#define PidLidReminderTime										0x8502
#define PidLidReminderSet										0x8503
#define PidLidReminderSignalTime								0x8560


#define PidLidCommonStart										0x8516
#define PidLidCommonEnd											0x8517



enum _WEEK_DAY_
{
	WEEK_Sunday =					0x01,
	WEEK_Monday = 					0x02,
	WEEK_Tuesday =					0x04,
	WEEK_Wednesday = 					0x08,
	WEEK_Thursday  =					0x10,
	WEEK_Friday   =					0x20,
	WEEK_Saturday =					0x40
};


#define MONTH_January  					0x0000
#define MONTH_February					0x60AE
#define MONTH_March						0xE04B
#define MONTH_April 					0x40FA
#define MONTH_May						0x00A3
#define MONTH_June						0x6051
#define MONTH_July						0x20FA
#define MONTH_August 					0x80A8
#define MONTH_September 				0xE056
#define MONTH_October  					0xA0FF
#define MONTH_November 					0x00AE
#define MONTH_December 					0xC056

#define FREQ_DAILY						0x0A
#define FREQ_WEEKLY						0x0B
#define FREQ_MONTHLY					0x0C
#define FREQ_YEARLY						0x0D




class CalendarHelper:public COutLookHelper
{
public:
	BOOL openFolder();
	static int CalendarCompare(CAL_EVT_ITEM *pCal1,CAL_EVT_ITEM *pCal2);
	int GetMaxCalendarID(ULONG &lMaxCalendarID);
	int IsCalendarExistInPhoneFolder(CAL_EVT_ITEM  *pCal);
	int SaveCalendar2OutLook(CAL_EVT_ITEM *pCal);
	int Export();
	int DeleteCalendarByEntryID(ULONG cb,LPBYTE lpb);
	int FindCalendarItemByEntryID(TCHAR *tszEntryID,CAL_EVT_ITEM *pCal);

	int SyncAllCalendar2Phone();
	list<TCHAR*> GetSubFolderList();

	int GetLastSyncTime(TCHAR *szCalendarID,FILETIME &ftLastSyncTime);

	int SyncCalendar2Phone(TCHAR *szSubFolder,FILETIME& ftSyncAfter);
protected:
	ULONG m_lTag4CalendarID;

	int GetItemSyncTime(LPMESSAGE pMsg,OUT FILETIME &ft);

	int GenCalendarProperty(LPMESSAGE lpMessage);

	int GetCalendarProperty(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);
	int GetCalendarPropertyEx(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);
	int GetCalendarPropertyRRULE(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);

	bool SetCalendarProperty(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);
	HRESULT SetCalendarPropertyEx(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);

	int GetCalendarReminder(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);
	HRESULT SetCalendarReminder(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);
	HRESULT SetCalendarTimeZone(LPMESSAGE lpMessage,CAL_EVT_ITEM *pCal);

	int SyncCalendarFromFolder2Phone(FILETIME& ftSyncAfter);
};


