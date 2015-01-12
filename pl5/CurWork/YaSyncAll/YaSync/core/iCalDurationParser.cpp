/* -*- Mode: C -*-
======================================================================
FILE: icaltime.c
CREATOR: eric 02 June 2000

$Id: icalduration.c,v 1.21 2008-01-15 23:17:40 dothebart Exp $
$Locker:  $

(C) COPYRIGHT 2000, Eric Busboom, http://www.softwarestudio.org

This program is free software; you can redistribute it and/or modify
it under the terms of either: 

The LGPL as published by the Free Software Foundation, version
2.1, available at: http://www.fsf.org/copyleft/lesser.html

Or:

The Mozilla Public License Version 1.0. You may obtain a copy of
the License at http://www.mozilla.org/MPL/

The Original Code is eric. The Initial Developer of the Original
Code is Eric Busboom


======================================================================*/

#include "stdafx.h"

#include "icalduration.h"
#include "PEProtocol.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#ifdef WIN32
#define snprintf _snprintf
#endif



/* From Seth Alves,  <alves@hungry.com>   */
struct icaldurationtype icaldurationtype_from_int(int t)
{
	struct icaldurationtype dur;
	int used = 0;

	dur = icaldurationtype_null_duration();

	if(t < 0){
		dur.is_neg = 1;
		t = -t;
	}

	if (t % (60 * 60 * 24 * 7) == 0) {
		dur.weeks = t / (60 * 60 * 24 * 7);
	} else {
		used += dur.weeks * (60 * 60 * 24 * 7);
		dur.days = (t - used) / (60 * 60 * 24);
		used += dur.days * (60 * 60 * 24);
		dur.hours = (t - used) / (60 * 60);
		used += dur.hours * (60 * 60);
		dur.minutes = (t - used) / (60);
		used += dur.minutes * (60);
		dur.seconds = (t - used);
	}

	return dur;
}


time_t GetDurationTime(TCHAR *tszDuration)
{
	char szDuration[255];
	memset(szDuration,0,255);
	WideCharToMultiByte(CP_UTF8,0,tszDuration,(int)_tcslen(tszDuration),szDuration,255,0,0);

	struct icaldurationtype *d = icaldurationtype_from_string(szDuration);
	if (d == NULL)
	{
		return 0;
	}

	int iNeg = 1;
	if (d->is_neg)
	{
		iNeg = -1;
	}

	time_t duration = d->days * 24 * 60 * 60;
	duration += d->weeks * 7 * 24 * 60 * 60;
	duration += d->hours * 60 * 60;
	duration += d->minutes * 60;
	duration += d->seconds;

	duration = iNeg * duration * ONE_SECOND;

	free(d);

	return duration;

}


struct icaldurationtype* icaldurationtype_from_string(const char* str)
{
	int i;
	int begin_flag = 0;
	int time_flag = 0;
	int date_flag = 0;
	int week_flag = 0;
	int digits=-1;
	int scan_size = -1;
	int size = (int)strlen(str);
	char p;
	struct icaldurationtype *d = (icaldurationtype*)calloc(1,sizeof(icaldurationtype));

	for( i=0; i != size; i++)
	{
		p = str[i];

		switch(p) 
		{
		case '-': 
			{
				if(i != 0 || begin_flag == 1) goto error;

				d->is_neg = 1;
				break;
			}
		case 'P':
			{
				if (i != 0 && i !=1 ) goto error;
				begin_flag = 1;
				time_flag = 1;
				break;
			}
		case 'T':
			{
				time_flag = 1;
				break;
			}

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{ 

				/* HACK. Skip any more digits if the l;ast one
				read has not been assigned */
				if(digits != -1)
				{
					break;
				}

				if (begin_flag == 0) goto error;
				/* Get all of the digits, not one at a time */
				scan_size = sscanf(&str[i],"%d",&digits);
				if(scan_size == 0) goto error;
				break;
			}

		case 'H':
			{	
				d->hours = digits; digits = -1;
				break;
			}
		case 'M': 
			{
				d->minutes = digits; digits = -1;	    
				break;
			}
		case 'S': 
			{
				d->seconds = digits; digits = -1;	    
				break;
			}
		case 'W':
			{
				week_flag = 1;	
				d->weeks = digits; digits = -1;	    
				break;
			}
		case 'D': 
			{
				date_flag = 1;
				d->days = digits; digits = -1;	    
				break;
			}
		default:
			{
				goto error;
			}

		}
	}

	return d;

error:
	free(d);
	return NULL;

}





/* From Russel Steinthal */
int icaldurationtype_as_int(struct icaldurationtype dur)
{
	return (int)( (dur.seconds +
		(60 * dur.minutes) +
		(60 * 60 * dur.hours) +
		(60 * 60 * 24 * dur.days) +
		(60 * 60 * 24 * 7 * dur.weeks))
		* (dur.is_neg==1? -1 : 1) ) ;
} 

struct icaldurationtype icaldurationtype_null_duration(void)
{
	struct icaldurationtype d;

	memset(&d,0,sizeof(struct icaldurationtype));

	return d;
}

int icaldurationtype_is_null_duration(struct icaldurationtype d)
{
	if(icaldurationtype_as_int(d) == 0){
		return 1;
	} else {
		return 0;
	}
}

/* in icalvalue_new_from_string_with_error, we should not call
icaldurationtype_is_null_duration() to see if there is an error
condition. Null duration is perfectly valid for an alarm.
We cannot depend on the caller to check icalerrno either,
following the philosophy of unix errno. we set the is_neg
to -1 to indicate that this is a bad duration.
*/
struct icaldurationtype icaldurationtype_bad_duration()
{
	struct icaldurationtype d;
	memset(&d,0,sizeof(struct icaldurationtype));
	d.is_neg = -1;
	return d;
}

int icaldurationtype_is_bad_duration(struct icaldurationtype d)
{
	return (d.is_neg == -1);
}



