

#ifndef ICALRECUR_H
#define ICALRECUR_H

#include <time.h>

/*
* Recurrance enumerations
*/




typedef int icalrecurrencetype_frequency;

#define	ICAL_SECONDLY_RECURRENCE		0
#define	ICAL_MINUTELY_RECURRENCE		1
#define	ICAL_HOURLY_RECURRENCE			2
#define	ICAL_DAILY_RECURRENCE			3
#define	ICAL_WEEKLY_RECURRENCE			4
#define	ICAL_MONTHLY_RECURRENCE			5
#define	ICAL_YEARLY_RECURRENCE			6
#define	ICAL_NO_RECURRENCE				7

#define	ICAL_MONTHLY_NTH_RECURRENCE		8
#define	ICAL_YEARLY_NTH_RECURRENCE		9


typedef int icalrecurrencetype_weekday;

#define	ICAL_NO_WEEKDAY					0
#define	ICAL_SUNDAY_WEEKDAY				1
#define	ICAL_MONDAY_WEEKDAY				2
#define	ICAL_TUESDAY_WEEKDAY			3
#define	ICAL_WEDNESDAY_WEEKDAY			4
#define	ICAL_THURSDAY_WEEKDAY			5
#define	ICAL_FRIDAY_WEEKDAY				6
#define	ICAL_SATURDAY_WEEKDAY			7


enum _ICAL_RECU{
	ICAL_RECURRENCE_ARRAY_MAX = 0x7f7f,
	ICAL_RECURRENCE_ARRAY_MAX_BYTE = 0x7f
};



/**
* Recurrence type routines
*/

/* See RFC 2445 Section 4.3.10, RECUR Value, for an explaination of
the values and fields in struct icalrecurrencetype */

#define ICAL_BY_SECOND_SIZE 61
#define ICAL_BY_MINUTE_SIZE 61
#define ICAL_BY_HOUR_SIZE 25
#define ICAL_BY_DAY_SIZE 364 /* 7 days * 52 weeks */
#define ICAL_BY_MONTHDAY_SIZE 32
#define ICAL_BY_YEARDAY_SIZE 367
#define ICAL_BY_WEEKNO_SIZE 54
#define ICAL_BY_MONTH_SIZE 13
#define ICAL_BY_SETPOS_SIZE 367

struct _icaltimezone {
    char		*tzid;
    /**< The unique ID of this timezone,
       e.g. "/citadel.org/Olson_20010601_1/Africa/Banjul".
       This should only be used to identify a VTIMEZONE. It is not
       meant to be displayed to the user in any form. */

    char		*location;
    /**< The location for the timezone, e.g. "Africa/Accra" for the
       Olson database. We look for this in the "LOCATION" or
       "X-LIC-LOCATION" properties of the VTIMEZONE component. It
       isn't a standard property yet. This will be NULL if no location
       is found in the VTIMEZONE. */

    char		*tznames;
    /**< This will be set to a combination of the TZNAME properties
       from the last STANDARD and DAYLIGHT components in the
       VTIMEZONE, e.g. "EST/EDT".  If they both use the same TZNAME,
       or only one type of component is found, then only one TZNAME
       will appear, e.g. "AZOT". If no TZNAME is found this will be
       NULL. */

    double		 latitude;
    double		 longitude;
    /**< The coordinates of the city, in degrees. */


    int			 end_year;
    /**< This is the last year for which we have expanded the data to.
       If we need to calculate a date past this we need to expand the
       timezone component data from scratch. */
};

struct icaltimetype
{
	int year;	/**< Actual year, e.g. 2001. */
	int month;	/**< 1 (Jan) to 12 (Dec). */
	int day;
	int hour;
	int minute;
	int second;

	int is_utc;     /**< 1-> time is in UTC timezone */

	int is_date;    /**< 1 -> interpret this as date. */
   
	int is_daylight; /**< 1 -> time is in daylight savings time. */
   
	const _icaltimezone *zone;	/**< timezone */
};

/** Main struct for holding digested recurrence rules */
struct icalrecurrencetype 
{
	icalrecurrencetype_frequency freq;


	/* until and count are mutually exclusive. */
	struct icaltimetype until; 
	int count;

	short interval;

	icalrecurrencetype_weekday week_start;

	/* The BY* parameters can each take a list of values. Here I
	* assume that the list of values will not be larger than the
	* range of the value -- that is, the client will not name a
	* value more than once. 

	* Each of the lists is terminated with the value
	* ICAL_RECURRENCE_ARRAY_MAX unless the the list is full.
	*/

	short by_second[ICAL_BY_SECOND_SIZE];
	short by_minute[ICAL_BY_MINUTE_SIZE];
	short by_hour[ICAL_BY_HOUR_SIZE];
	short by_day[ICAL_BY_DAY_SIZE]; /* Encoded value, see below */
	short by_month_day[ICAL_BY_MONTHDAY_SIZE];
	short by_year_day[ ICAL_BY_YEARDAY_SIZE];
	short by_week_no[ICAL_BY_WEEKNO_SIZE];
	short by_month[ICAL_BY_MONTH_SIZE];
	short by_set_pos[ICAL_BY_SETPOS_SIZE];
};


void icalrecurrencetype_clear(struct icalrecurrencetype *r);

/**
* Array Encoding
*
* The 'day' element of the by_day array is encoded to allow
* representation of both the day of the week ( Monday, Tueday), but also
* the Nth day of the week ( First tuesday of the month, last thursday of
* the year) These routines decode the day values 
*/

/** 1 == Monday, etc. */
int icalrecurrencetype_day_day_of_week(short day);

/** 0 == any of day of week. 1 == first, 2 = second, -2 == second to last, etc */
int icalrecurrencetype_day_position(short day);

icalrecurrencetype_weekday icalrecur_string_to_weekday(const char* str);

/** Recurrance rule parser */

/** Convert between strings and recurrencetype structures. */
struct icalrecurrencetype icalrecurrencetype_from_string(const char* str);
char* icalrecurrencetype_as_string(struct icalrecurrencetype *recur);
char* icalrecurrencetype_as_string_r(struct icalrecurrencetype *recur);



#endif
