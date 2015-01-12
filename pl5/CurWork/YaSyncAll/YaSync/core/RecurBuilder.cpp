#include "stdafx.h"
#include "PEProtocol.h"
#include "RecurBuilder.h"

#include "mapiutil.h"
#include "RRuleParse.h"

#include <math.h>
#include <stdio.h>

#include "AppTools.h"


void ParseWeekMask4ByDay(short *by_day,DWORD dwWeekMask)
{
	int iPos = 0;
	if (dwWeekMask & rdfSun)
	{
		by_day[iPos++] = 1;
	}

	if (dwWeekMask & rdfMon)
	{
		by_day[iPos++] = 2;
	}

	if (dwWeekMask & rdfTue)
	{
		by_day[iPos++] = 3;
	}

	if (dwWeekMask & rdfWed)
	{
		by_day[iPos++] = 4;
	}

	if (dwWeekMask & rdfThu)
	{
		by_day[iPos++] = 5;
	}

	if (dwWeekMask & rdfFri)
	{
		by_day[iPos++] = 6;
	}

	if (dwWeekMask & rdfSat)
	{
		by_day[iPos++] = 7;
	}

	return;
}

int ParseRecurBin(TCHAR *tszRRULE,byte *pBuf,int iBufLen)
{
	if (iBufLen < 76)
	{
		return PE_RET_FAIL;
	}

	struct icalrecurrencetype recuInfo;
	memset(&recuInfo,0,sizeof(recuInfo));

	memset(&recuInfo.by_day,0x7f,sizeof(recuInfo.by_day));
	memset(&recuInfo.by_hour,0x7f,sizeof(recuInfo.by_hour));
	memset(&recuInfo.by_minute,0x7f,sizeof(recuInfo.by_minute));
	memset(&recuInfo.by_month,0x7f,sizeof(recuInfo.by_month));
	memset(&recuInfo.by_month_day,0x7f,sizeof(recuInfo.by_month_day));
	memset(&recuInfo.by_second,0x7f,sizeof(recuInfo.by_second));
	memset(&recuInfo.by_set_pos,0x7f,sizeof(recuInfo.by_set_pos));
	memset(&recuInfo.by_week_no,0x7f,sizeof(recuInfo.by_week_no));
	memset(&recuInfo.by_year_day,0x7f,sizeof(recuInfo.by_year_day));

	AptHeader hdr;
	memcpy(&hdr,pBuf,sizeof(hdr));

	switch (hdr.RecurFrequency)
	{
	case IDC_RCEV_PAT_ORB_DAILY:
		{
			AptRecurPatternDailyBin aptRecu = {0};
			if (sizeof(AptRecurPatternDailyBin) > iBufLen)
			{
				//print error to log
				return PE_RET_FAIL;
			}

			memcpy(&aptRecu,pBuf,sizeof(aptRecu));
			recuInfo.week_start = aptRecu.FirstDOW + 1;
			recuInfo.interval = (short)aptRecu.Period/(24*60);
			
			if (aptRecu.EndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				recuInfo.count = 0;
			}
			else
			{
				recuInfo.count = aptRecu.OccurrenceCount;
			}

			recuInfo.freq = ICAL_DAILY_RECURRENCE;
			break;
		}
	case IDC_RCEV_PAT_ORB_WEEKLY:
		{
			AptRecurPatternBin aptRecu = {0};
			if (sizeof(AptRecurPatternBin) > iBufLen)
			{
				//print error to log
				return PE_RET_FAIL;
			}

			memcpy(&aptRecu,pBuf,sizeof(aptRecu));
			recuInfo.week_start = aptRecu.FirstDOW + 1;
			recuInfo.interval = (short)aptRecu.Period;
			
			if (aptRecu.EndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				recuInfo.count = 0;
			}
			else
			{
				recuInfo.count = aptRecu.OccurrenceCount;
			}

			ParseWeekMask4ByDay(recuInfo.by_day,aptRecu.PatternTypeSpecific);
			recuInfo.freq = ICAL_WEEKLY_RECURRENCE;
			break;	
		}
	case IDC_RCEV_PAT_ORB_MONTHLY:
		{
			if (hdr.PatternType == rptMonth)
			{
				AptRecurPatternBin aptRecu = {0};
				if (sizeof(AptRecurPatternBin) > iBufLen)
				{
					//print error to log
					return PE_RET_FAIL;
				}

				memcpy(&aptRecu,pBuf,sizeof(aptRecu));
				recuInfo.week_start = aptRecu.FirstDOW + 1;
				recuInfo.interval = (short)aptRecu.Period;
				
				if (aptRecu.EndType == IDC_RCEV_PAT_ERB_NOEND)
				{
					recuInfo.count = 0;
				}
				else
				{
					recuInfo.count = aptRecu.OccurrenceCount;
				}

				//ParseWeekMask4ByDay(recuInfo.by_day,aptRecu.PatternTypeSpecific);
				recuInfo.freq = ICAL_MONTHLY_RECURRENCE;
			}
			else if (hdr.PatternType == rptMonthNth)
			{
				AptRecurPatternMonthNthBin aptRecu = {0};
				if (sizeof(AptRecurPatternMonthNthBin) > iBufLen)
				{
					//print error to log
					return PE_RET_FAIL;
				}

				memcpy(&aptRecu,pBuf,sizeof(aptRecu));
				recuInfo.week_start = aptRecu.FirstDOW + 1;
				recuInfo.interval = (short)aptRecu.Period;
				
				if (aptRecu.EndType == IDC_RCEV_PAT_ERB_NOEND)
				{
					recuInfo.count = 0;
				}
				else
				{
					recuInfo.count = aptRecu.OccurrenceCount;
				}

				recuInfo.by_day[0] = (short)(aptRecu.PatternTypeSpecificNth + 8 * aptRecu.PatternTypeSpecific);

				ParseWeekMask4ByDay(recuInfo.by_day,aptRecu.PatternTypeSpecific);
				recuInfo.freq = ICAL_MONTHLY_RECURRENCE;
			}
			break;
		}
	case IDC_RCEV_PAT_ORB_YEARLY:
		{
			if (hdr.PatternType == rptMonth)
			{
				AptRecurPatternBin aptRecu = {0};
				if (sizeof(AptRecurPatternBin) > iBufLen)
				{
					//print error to log
					return PE_RET_FAIL;
				}

				memcpy(&aptRecu,pBuf,sizeof(aptRecu));
				recuInfo.week_start = aptRecu.FirstDOW + 1;
				recuInfo.interval = (short)aptRecu.Period/12;
				
				if (aptRecu.EndType == IDC_RCEV_PAT_ERB_NOEND)
				{
					recuInfo.count = 0;
				}
				else
				{
					recuInfo.count = aptRecu.OccurrenceCount;
				}

				//ParseWeekMask4ByDay(recuInfo.by_day,aptRecu.PatternTypeSpecific);
				recuInfo.freq = ICAL_YEARLY_RECURRENCE;
			}
			else if (hdr.PatternType == rptMonthNth)
			{
				AptRecurPatternMonthNthBin aptRecu = {0};
				if (sizeof(AptRecurPatternMonthNthBin) > iBufLen)
				{
					//print error to log
					return PE_RET_FAIL;
				}

				memcpy(&aptRecu,pBuf,sizeof(aptRecu));
				recuInfo.week_start = aptRecu.FirstDOW + 1;
				recuInfo.interval = (short)aptRecu.Period;
				
				if (aptRecu.EndType == IDC_RCEV_PAT_ERB_NOEND)
				{
					recuInfo.count = 0;
				}
				else
				{
					recuInfo.count = aptRecu.OccurrenceCount;
				}

				recuInfo.by_day[0] = (short)(aptRecu.PatternTypeSpecificNth + 8 * aptRecu.PatternTypeSpecific);
				recuInfo.freq = ICAL_YEARLY_RECURRENCE;
			}
			break;
		}
	default:
		return PE_RET_FAIL;
	}

	char *szRRULE = icalrecurrencetype_as_string(&recuInfo);
	MultiByteToWideChar(CP_ACP,0,szRRULE,-1,tszRRULE,1024);
	free(szRRULE);

	return PE_RET_OK;
}


short by_day[ICAL_BY_DAY_SIZE];
DWORD ParseByDay4Week(short *by_day)
{
	DWORD dwWeekMask = 0;
	for (int i = 0; i < 7; ++i)
	{
		if (by_day[i] > 10)
		{
			break;
		}

		switch (by_day[i])
		{
		case 1:
			dwWeekMask |= rdfSun;
			break;
		case 2:
			dwWeekMask |= rdfMon;
			break;
		case 3:
			dwWeekMask |= rdfTue;
			break;
		case 4:
			dwWeekMask |= rdfWed;
			break;
		case 5:
			dwWeekMask |= rdfThu;
			break;
		case 6:
			dwWeekMask |= rdfFri;
			break;
		case 7:
			dwWeekMask |= rdfSat;
			break;
		}
	}

	return dwWeekMask;
}

DWORD ParseByDay4Month(short *by_day,int &pos)
{
	if (by_day[0] == 0x7f7f)
	{
		return 0;
	}

	int dow = icalrecurrencetype_day_day_of_week(by_day[0]);
	pos = icalrecurrencetype_day_position(by_day[0]);

	DWORD dwWeekMask = 0;

	switch (dow)
	{
	case 1:
		dwWeekMask |= rdfSun;
		break;
	case 2:
		dwWeekMask |= rdfMon;
		break;
	case 3:
		dwWeekMask |= rdfTue;
		break;
	case 4:
		dwWeekMask |= rdfWed;
		break;
	case 5:
		dwWeekMask |= rdfThu;
		break;
	case 6:
		dwWeekMask |= rdfFri;
		break;
	case 7:
		dwWeekMask |= rdfSat;
		break;
	}

	return dwWeekMask;
}

void computeTimeOffset(FILETIME &ft,TCHAR *szTZ,DWORD& dwTime,DWORD& dwTimeOffset)
{
	FILETIME ft1 = ft;
	GetMidnightTime(ft1);
	time_t t1 = 0;
	memcpy(&t1,&ft1,sizeof(FILETIME));
	dwTime = (DWORD)(t1 / (ONE_SECOND * 60));

	FILETIME ftLocal;
	if (_tcscmp(szTZ,_T("UTC")) == 0)
	{
		ftLocal = ft;
	}
	else
	{
		FileTimeToLocalFileTime(&ft,&ftLocal);
	}

	SYSTEMTIME stLocal;
	FileTimeToSystemTime(&ftLocal,&stLocal);

	dwTimeOffset = stLocal.wHour * 60 + stLocal.wMinute;
}


int ParseRRULE(CAL_EVT_ITEM *pCal,byte **pBuf,int &iBufLen,int &iRecType,int &iEndType)
{
	char szRRULE[1024];
	memset(szRRULE,0,1024);
	WideCharToMultiByte(CP_UTF8,0, pCal->szRRULE,(int)_tcslen( pCal->szRRULE),szRRULE,1024,0,0);

	struct icalrecurrencetype recuInfo = icalrecurrencetype_from_string(szRRULE);
	DWORD dwEndType = 0;
	if (recuInfo.count > 0)
	{
		dwEndType = IDC_RCEV_PAT_ERB_AFTERNOCCUR;
	}
	else if (recuInfo.until.year > 2010)
	{
		dwEndType = IDC_RCEV_PAT_ERB_END;
	}
	else
	{
		dwEndType = IDC_RCEV_PAT_ERB_NOEND;
	}

	iEndType = dwEndType;

	iRecType = rectypeNone;

	if (recuInfo.freq == ICAL_MONTHLY_RECURRENCE)
	{
		int pos = 0;
		ParseByDay4Month(recuInfo.by_day,pos);
		if (pos > 0)
		{
			recuInfo.freq = ICAL_MONTHLY_NTH_RECURRENCE;
		}
	}

	if (recuInfo.freq == ICAL_YEARLY_RECURRENCE)
	{
		int pos = 0;
		ParseByDay4Month(recuInfo.by_day,pos);
		if (pos > 0)
		{
			recuInfo.freq = ICAL_YEARLY_NTH_RECURRENCE;
		}
	}

	switch (recuInfo.freq)
	{
	case ICAL_DAILY_RECURRENCE:
		{
			AptRecurPatternDailyBin *pRRULE = (AptRecurPatternDailyBin*)calloc(1,sizeof(AptRecurPatternDailyBin));
			*pBuf = (byte*)pRRULE;
			iBufLen = sizeof(AptRecurPatternDailyBin);
			iRecType = rectypeDaily;

			pRRULE->ReaderVersion2 = 0x00003006;
			pRRULE->WriterVersion2 = 0x00003009;
			pRRULE->ReaderVersion = 0x3004;
			pRRULE->WriterVersion = 0x3004;
			pRRULE->RecurFrequency = IDC_RCEV_PAT_ORB_DAILY;
			pRRULE->PatternType = 0;
			pRRULE->EndType = dwEndType;
			pRRULE->FirstDOW =  recuInfo.week_start - 1;
			pRRULE->Period = recuInfo.interval * 1440;

			computeTimeOffset( pCal->ftDTSTART, pCal->szTimeZone,pRRULE->StartDate,pRRULE->StartTimeOffset);
			computeTimeOffset( pCal->ftDTEND, pCal->szTimeZone,pRRULE->EndDate,pRRULE->EndTimeOffset);

			if (dwEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				pRRULE->OccurrenceCount = 0x0000000A;
				pRRULE->EndDate = 0x5AE980DF;
			}
			else
			{
				pRRULE->OccurrenceCount = recuInfo.count;
			}

			CalculateDailyRecurrence(pRRULE);
			break;
		}
	case ICAL_WEEKLY_RECURRENCE:
		{
			AptRecurPatternBin *pRRULE = (AptRecurPatternBin*)calloc(1,sizeof(AptRecurPatternBin));
			*pBuf = (byte*)pRRULE;
			iBufLen = sizeof(AptRecurPatternBin);
			iRecType = rectypeWeekly;

			pRRULE->ReaderVersion2 = 0x00003006;
			pRRULE->WriterVersion2 = 0x00003009;
			pRRULE->ReaderVersion = 0x3004;
			pRRULE->WriterVersion = 0x3004;
			pRRULE->RecurFrequency = IDC_RCEV_PAT_ORB_WEEKLY;
			pRRULE->PatternTypeSpecific = ParseByDay4Week(recuInfo.by_day);
			pRRULE->PatternType = rptWeek;
			pRRULE->EndType = dwEndType;
			pRRULE->FirstDOW =  recuInfo.week_start - 1;
			pRRULE->Period = recuInfo.interval;

			computeTimeOffset( pCal->ftDTSTART, pCal->szTimeZone,pRRULE->StartDate,pRRULE->StartTimeOffset);
			computeTimeOffset( pCal->ftDTEND, pCal->szTimeZone,pRRULE->EndDate,pRRULE->EndTimeOffset);

			if (dwEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				pRRULE->OccurrenceCount = 0x0000000A;
				pRRULE->EndDate = 0x5AE980DF;
			}
			else
			{
				pRRULE->OccurrenceCount = recuInfo.count;
			}

			CalculateWeeklyRecurrence(pRRULE);
			break;	
		}
	case ICAL_MONTHLY_RECURRENCE:
		{
			AptRecurPatternBin *pRRULE = (AptRecurPatternBin*)calloc(1,sizeof(AptRecurPatternBin));
			*pBuf = (byte*)pRRULE;
			iBufLen = sizeof(AptRecurPatternBin);
			iRecType = rectypeMonthly;

			pRRULE->ReaderVersion2 = 0x00003006;
			pRRULE->WriterVersion2 = 0x00003009;
			pRRULE->ReaderVersion = 0x3004;
			pRRULE->WriterVersion = 0x3004;
			pRRULE->RecurFrequency = IDC_RCEV_PAT_ORB_MONTHLY;
			pRRULE->PatternType = rptMonth;
			pRRULE->EndType = dwEndType;
			pRRULE->FirstDOW =  recuInfo.week_start - 1;
			pRRULE->Period = recuInfo.interval;


			SYSTEMTIME st;
			FileTimeToSystemTime (& pCal->ftDTSTART,&st);
			pRRULE->PatternTypeSpecific = st.wDay;

			computeTimeOffset( pCal->ftDTSTART, pCal->szTimeZone,pRRULE->StartDate,pRRULE->StartTimeOffset);
			computeTimeOffset( pCal->ftDTEND, pCal->szTimeZone,pRRULE->EndDate,pRRULE->EndTimeOffset);

			if (dwEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				pRRULE->OccurrenceCount = 0x0000000A;
				pRRULE->EndDate = 0x5AE980DF;
			}
			else
			{
				pRRULE->OccurrenceCount = recuInfo.count;
			}

			CalculateMonthlyOrYearlyRecurrence(pRRULE);
			break;
		}
	case ICAL_MONTHLY_NTH_RECURRENCE:
		{
			AptRecurPatternMonthNthBin *pRRULE = (AptRecurPatternMonthNthBin*)calloc(1,sizeof(AptRecurPatternMonthNthBin));
			*pBuf = (byte*)pRRULE;
			iBufLen = sizeof(AptRecurPatternMonthNthBin);
			iRecType = rectypeMonthly;

			pRRULE->ReaderVersion2 = 0x00003006;
			pRRULE->WriterVersion2 = 0x00003009;
			pRRULE->ReaderVersion = 0x3004;
			pRRULE->WriterVersion = 0x3004;
			pRRULE->RecurFrequency = IDC_RCEV_PAT_ORB_MONTHLY;
			pRRULE->PatternType = rptMonthNth;
			pRRULE->EndType = dwEndType;
			pRRULE->FirstDOW =  recuInfo.week_start - 1;
			pRRULE->Period = recuInfo.interval;

			int pos = 0;
			pRRULE->PatternTypeSpecific = ParseByDay4Month(recuInfo.by_day,pos);
			pRRULE->PatternTypeSpecificNth = (DWORD)pos;

			computeTimeOffset( pCal->ftDTSTART, pCal->szTimeZone,pRRULE->StartDate,pRRULE->StartTimeOffset);
			computeTimeOffset( pCal->ftDTEND, pCal->szTimeZone,pRRULE->EndDate,pRRULE->EndTimeOffset);

			if (dwEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				pRRULE->OccurrenceCount = 0x0000000A;
				pRRULE->EndDate = 0x5AE980DF;
			}
			else
			{
				pRRULE->OccurrenceCount = recuInfo.count;
			}

			CalculateMonthlyNthRecurrence(pRRULE);
			break;
		}
	case ICAL_YEARLY_RECURRENCE:
		{
			AptRecurPatternBin *pRRULE = (AptRecurPatternBin*)calloc(1,sizeof(AptRecurPatternBin));
			*pBuf = (byte*)pRRULE;
			iBufLen = sizeof(AptRecurPatternBin);
			iRecType = rectypeYearly;

			pRRULE->ReaderVersion2 = 0x00003006;
			pRRULE->WriterVersion2 = 0x00003009;
			pRRULE->ReaderVersion = 0x3004;
			pRRULE->WriterVersion = 0x3004;
			pRRULE->RecurFrequency = IDC_RCEV_PAT_ORB_YEARLY;
			pRRULE->PatternType = rptMonth;
			pRRULE->EndType = dwEndType;
			pRRULE->FirstDOW =  recuInfo.week_start - 1;
			pRRULE->Period = recuInfo.interval * 12;
			pRRULE->OccurrenceCount = recuInfo.count;

			SYSTEMTIME st;
			FileTimeToSystemTime (& pCal->ftDTSTART,&st);
			pRRULE->PatternTypeSpecific = st.wDay;

			computeTimeOffset( pCal->ftDTSTART, pCal->szTimeZone,pRRULE->StartDate,pRRULE->StartTimeOffset);
			computeTimeOffset( pCal->ftDTEND, pCal->szTimeZone,pRRULE->EndDate,pRRULE->EndTimeOffset);

			if (dwEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				pRRULE->OccurrenceCount = 0x0000000A;
				pRRULE->EndDate = 0x5AE980DF;
			}
			else
			{
				pRRULE->OccurrenceCount = recuInfo.count;
			}

			CalculateMonthlyOrYearlyRecurrence(pRRULE);
			break;
		}
	case ICAL_YEARLY_NTH_RECURRENCE:
		{
			AptRecurPatternMonthNthBin *pRRULE = (AptRecurPatternMonthNthBin*)calloc(1,sizeof(AptRecurPatternMonthNthBin));
			*pBuf = (byte*)pRRULE;
			iBufLen = sizeof(AptRecurPatternMonthNthBin);
			iRecType = rectypeMonthly;

			pRRULE->ReaderVersion2 = 0x00003006;
			pRRULE->WriterVersion2 = 0x00003009;
			pRRULE->ReaderVersion = 0x3004;
			pRRULE->WriterVersion = 0x3004;
			pRRULE->RecurFrequency = IDC_RCEV_PAT_ORB_YEARLY;
			pRRULE->PatternType = rptMonthNth;
			pRRULE->EndType = dwEndType;
			pRRULE->FirstDOW =  recuInfo.week_start - 1;
			pRRULE->Period = recuInfo.interval;

			int pos = 0;
			pRRULE->PatternTypeSpecific = ParseByDay4Month(recuInfo.by_day,pos);
			pRRULE->PatternTypeSpecificNth = (DWORD)pos;

			computeTimeOffset( pCal->ftDTSTART, pCal->szTimeZone,pRRULE->StartDate,pRRULE->StartTimeOffset);
			computeTimeOffset( pCal->ftDTEND, pCal->szTimeZone,pRRULE->EndDate,pRRULE->EndTimeOffset);
			if (dwEndType == IDC_RCEV_PAT_ERB_NOEND)
			{
				pRRULE->OccurrenceCount = 0x0000000A;
				pRRULE->EndDate = 0x5AE980DF;
			}
			else
			{
				pRRULE->OccurrenceCount = recuInfo.count;
			}

			CalculateMonthlyNthRecurrence(pRRULE);
			break;
		}
	default :
		return PE_RET_FAIL;
	}

	return PE_RET_OK;
}


CTime _ToCTime(DWORD dwTime)
{
	time_t startdate = dwTime;
	startdate = startdate * 60;
	startdate = startdate * ONE_SECOND;
	SYSTEMTIME st;
	FileTimeToSystemTime((FILETIME*)&startdate,&st);
	CTime tmp(st);

	return tmp;
}

int CalculateDailyRecurrence(AptRecurPatternDailyBin *pRecur)
{
	//double minutesBetweenMinDateAndStartDate = myRecurrenceValues.startDate.Subtract(minDateGregorian).TotalMinutes;
	DWORD minutesBetweenMinDateAndStartDate = (DWORD)pRecur->StartDate;

	// We can use FirstDateTime to find a valid date given a "clip start date" that falls close to dates in our series
	// For daily recurrences, FirstDateTime will always be a multiple of 1440 (the number of minutes in a day).
	// We calculate FirstDateTime by taking the start date (as expressed in minutes between minimum date and start date)  and modulo the period in minutes. 
	pRecur->FirstDateTime = minutesBetweenMinDateAndStartDate % pRecur->Period;

	return PE_RET_OK;
}

int CalculateWeeklyRecurrence(AptRecurPatternBin *pRecur)
{
	// Find the date of the first day of the week prior to the start date.
	//DateTime modifiedStartDate = myRecurrenceValues.startDate.AddDays(-(int)myRecurrenceValues.startDate.DayOfWeek);
	CTime t1 = _ToCTime(pRecur->StartDate);
	SYSTEMTIME st;
	t1.GetAsSystemTime(st);

	CTimeSpan tDiff(st.wDayOfWeek,0,0,0);

	CTime t2 = t1 - tDiff;

	// Calculate the number of minutes between midnight that day and midnight January 1, 1601.
	//double modifiedStartDateInMinutes = modifiedStartDate.Subtract(minDateGregorian).TotalMinutes;
	t2.GetAsSystemTime(st);
	ULONG64 ft;
	SystemTimeToFileTime(&st,(FILETIME*)&ft);
	DWORD modifiedStartDateInMinutes = (DWORD)(ft/ONE_SECOND*60);

	// Take that value modulo the value of the Period field x 10080 (# of minutes in a week) to get the value of FirstDateTime.
	pRecur->FirstDateTime = modifiedStartDateInMinutes % (pRecur->Period * 10080);

	return PE_RET_OK;
}


int CalculateMonthlyOrYearlyRecurrence(AptRecurPatternBin *pRecur)
{
	// Find the first day of the month of the start date
	//DateTime modifiedStartDate = myRecurrenceValues.startDate.AddDays(-myHebrewCal.GetDayOfMonth(myRecurrenceValues.startDate)+1);
	CTime t1 = _ToCTime(pRecur->StartDate);
	CTimeSpan tDiff(t1.GetDay() - 1,0,0,0);
	
	CTime t2 = t1 -tDiff;

	SYSTEMTIME st;
	t2.GetAsSystemTime(st);

	DWORD dwMonths = (st.wYear - 1601) * 12 + st.wMonth;

	int offsetInMonths = dwMonths % (pRecur->Period);

	memset(&st,0,sizeof(st));
	st.wYear = 1601 + offsetInMonths / 12;
	st.wMonth = offsetInMonths % 12;
	st.wDay = 1;

	FILETIME ft;
	SystemTimeToFileTime(&st,&ft);

	time_t t;
	memcpy(&t,&ft,sizeof(FILETIME));

	pRecur->FirstDateTime = (DWORD)(t/ONE_SECOND);
	pRecur->FirstDateTime = pRecur->FirstDateTime/60;
	return PE_RET_OK;
}



int CalculateMonthlyNthRecurrence(AptRecurPatternMonthNthBin *pRecur)
{
	CTime t1 = _ToCTime(pRecur->StartDate);
	CTimeSpan tDiff(t1.GetDay() - 1,0,0,0);
	
	CTime t2 = t1 -tDiff;

	SYSTEMTIME st;
	t2.GetAsSystemTime(st);

	DWORD dwMonths = (st.wYear - 1601) * 12 + st.wMonth;

	int offsetInMonths = dwMonths % (pRecur->Period);

	memset(&st,0,sizeof(st));
	st.wYear = 1601 + offsetInMonths / 12;
	st.wMonth = offsetInMonths % 12;
	st.wDay = 1;

	FILETIME ft;
	SystemTimeToFileTime(&st,&ft);

	time_t t;
	memcpy(&t,&ft,sizeof(FILETIME));

	pRecur->FirstDateTime = (DWORD)(t/ONE_SECOND);
	pRecur->FirstDateTime = pRecur->FirstDateTime/60;
	return PE_RET_OK;
}



