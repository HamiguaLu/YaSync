
#include "stdafx.h"

#include "RRuleParse.h"


#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <errno.h> /* for errno */
#include <string.h> /* for strdup and strchr*/
#include <assert.h>
#include <stddef.h> /* For offsetof() macro */


#define BUFFER_RING_SIZE 2500

/** This is the last year we will go up to, since 32-bit time_t values
only go up to the start of 2038. */
#define MAX_TIME_T_YEAR	2037

#define TEMP_MAX 1024

#define snprintf		_snprintf_s
#define strcasecmp      _stricmp

#define icalerror_check_arg_re(x,y,z)

#define icalerror_set_errno(x)	

const char* icalrecur_freq_to_string(icalrecurrencetype_frequency kind);
icalrecurrencetype_frequency icalrecur_string_to_freq(const char* str);

const char* icalrecur_weekday_to_string(icalrecurrencetype_weekday kind);
icalrecurrencetype_weekday icalrecur_string_to_weekday(const char* str);


static struct _ICUR_RECU_TYPE {icalrecurrencetype_weekday wd; const char * str; } 
wd_map[] = {
	{ICAL_SUNDAY_WEEKDAY,"SU"},
	{ICAL_MONDAY_WEEKDAY,"MO"},
	{ICAL_TUESDAY_WEEKDAY,"TU"},
	{ICAL_WEDNESDAY_WEEKDAY,"WE"},
	{ICAL_THURSDAY_WEEKDAY,"TH"},
	{ICAL_FRIDAY_WEEKDAY,"FR"},
	{ICAL_SATURDAY_WEEKDAY,"SA"},
	{ICAL_NO_WEEKDAY,0}
};

const char* icalrecur_weekday_to_string(int kind)
{
	int i;

	for (i=0; wd_map[i].wd  != ICAL_NO_WEEKDAY; i++) {
		if ( wd_map[i].wd ==  kind) {
			return wd_map[i].str;
		}
	}

	return 0;
}

void icalrecurrencetype_clear(struct icalrecurrencetype *recur)
{
	memset(recur,ICAL_RECURRENCE_ARRAY_MAX_BYTE,
		sizeof(struct icalrecurrencetype));

	recur->week_start = ICAL_MONDAY_WEEKDAY;
	recur->freq = ICAL_NO_RECURRENCE;
	recur->interval = 1;
	memset(&(recur->until),0,sizeof(struct icaltimetype));
	recur->count = 0;
}

int icalrecurrencetype_day_position(short day)
{
	int wd, pos;

	wd = icalrecurrencetype_day_day_of_week(day);

	pos = (abs(day)-wd)/8 * ((day<0)?-1:1);


	return pos;
}

int icaltime_is_utc(const struct icaltimetype t) {

	return t.is_utc;
}

void print_time_to_string(char* str, const struct icaltimetype *data)
{
    char temp[20];
    str[0] = '\0';

    if (data != 0) {
        if (icaltime_is_utc(*data)){
            snprintf(temp,sizeof(temp),"%02d%02d%02dZ",data->hour,data->minute,data->second);
            strncat(str,temp,7);
        } else {
            snprintf(temp,sizeof(temp),"%02d%02d%02d",data->hour,data->minute,data->second);
            strncat(str,temp,6);
        }   
    }
}

void print_date_to_string(char* str,  const struct icaltimetype *data)
{
    char temp[20];
    str[0] = '\0';

    if (data != 0) {
        snprintf(temp,sizeof(temp),"%04d%02d%02d",data->year,data->month,data->day);
        strncat(str,temp,8);
    }
}

void print_datetime_to_string(char* str,  const struct icaltimetype *data)
{
    char temp[20];
    str[0] = '\0';

    if (data != 0) {
        print_date_to_string(str,data);
        if ( !data->is_date ) {
            strncat(str,"T",19);
            temp[0] = '\0';
            print_time_to_string(temp,data);
            strncat(str,temp,19);
        }
    }
}



icalrecurrencetype_weekday icalrecur_string_to_weekday(const char* str)
{
	int i;

	for (i=0; wd_map[i].wd  != ICAL_NO_WEEKDAY; i++) {
		if ( strcasecmp(str,wd_map[i].str) == 0){
			return wd_map[i].wd;
		}
	}

	return ICAL_NO_WEEKDAY;
}

int icalrecurrencetype_day_day_of_week(short day)
{
	return abs(day)%8;
}


static struct _ICAL_RECUR_FREQ{
	icalrecurrencetype_frequency kind;
	const char* str;
} freq_map[] = {
	{ICAL_SECONDLY_RECURRENCE,"SECONDLY"},
	{ICAL_MINUTELY_RECURRENCE,"MINUTELY"},
	{ICAL_HOURLY_RECURRENCE,"HOURLY"},
	{ICAL_DAILY_RECURRENCE,"DAILY"},
	{ICAL_WEEKLY_RECURRENCE,"WEEKLY"},
	{ICAL_MONTHLY_RECURRENCE,"MONTHLY"},
	{ICAL_YEARLY_RECURRENCE,"YEARLY"},
	{ICAL_NO_RECURRENCE,0}
};

const char* icalrecur_freq_to_string(icalrecurrencetype_frequency kind)
{
	int i;

	for (i=0; freq_map[i].kind != ICAL_NO_RECURRENCE ; i++) {
		if ( freq_map[i].kind == kind ) {
			return freq_map[i].str;
		}
	}
	return 0;
}

icalrecurrencetype_frequency icalrecur_string_to_freq(const char* str)
{
	int i;

	for (i=0; freq_map[i].kind != ICAL_NO_RECURRENCE ; i++) {
		if ( strcasecmp(str,freq_map[i].str) == 0){
			return freq_map[i].kind;
		}
	}
	return ICAL_NO_RECURRENCE;
}



/*********************** Rule parsing routines ************************/

struct icalrecur_parser {
	const char* rule;
	char* copy;
	char* this_clause;
	char* next_clause;

	struct icalrecurrencetype rt;
};


void 
icalmemory_append_string(char** buf, char** pos, size_t* buf_size, 
			      const char* string)
{
    char *new_buf;
    char *new_pos;

    size_t data_length, final_length, string_length;

    string_length = strlen(string);
    data_length = (size_t)*pos - (size_t)*buf;    
    final_length = data_length + string_length; 

    if ( final_length >= (size_t) *buf_size) {

	
	*buf_size  = (*buf_size) * 2  + final_length;

	new_buf = (char*)realloc(*buf,*buf_size);

	new_pos = (char*)((size_t)new_buf + data_length);
	
	*pos = new_pos;
	*buf = new_buf;
    }
    
    strcpy(*pos, string);

    *pos += string_length;
}

void 
icalmemory_append_char(char** buf, char** pos, size_t* buf_size, 
			      char ch)
{
    char *new_buf;
    char *new_pos;

    size_t data_length, final_length;

    data_length = (size_t)*pos - (size_t)*buf;

    final_length = data_length + 2; 

    if ( final_length > (size_t) *buf_size ) {

	
	*buf_size  = (*buf_size) * 2  + final_length +1;

	new_buf = (char*)realloc(*buf,*buf_size);

	new_pos = (char*)((size_t)new_buf + data_length);
	
	*pos = new_pos;
	*buf = new_buf;
    }

    **pos = ch;
    *pos += 1;
    **pos = 0;
}

struct icaltimetype icaltime_null_time(void)
{
    struct icaltimetype t;
    memset(&t,0,sizeof(struct icaltimetype));

    return t;
}




struct icaltimetype icaltime_from_string(const char* str)
{
    struct icaltimetype tt = icaltime_null_time();
    int size;

	struct _icaltimezone utc_timezone = { (char *) "UTC", NULL, NULL, 0.0, 0.0, 0,  };

    icalerror_check_arg_re(str!=0,"str",icaltime_null_time());

    size = (int)strlen(str);
    
    if ((size == 15) || (size == 19)) { /* floating time with/without separators*/
	tt.is_utc = 0;
	tt.is_date = 0;
    } else if ((size == 16) || (size == 20)) { /* UTC time, ends in 'Z'*/
	if ((str[15] != 'Z') && (str[19] != 'Z'))
	    goto FAIL;

	tt.is_utc = 1;
	tt.zone = &utc_timezone;
	tt.is_date = 0;
    } else if ((size == 8) || (size == 10)) { /* A DATE */
	tt.is_utc = 0;
	tt.is_date = 1;
    } else { /* error */
	goto FAIL;
    }

    if (tt.is_date == 1){
        if (size == 10) {
            char dsep1, dsep2;    
            if (sscanf(str,"%04d%c%02d%c%02d",&tt.year,&dsep1,&tt.month,&dsep2,&tt.day) < 5)
                goto FAIL;
            if ((dsep1 != '-') || (dsep2 != '-'))
                goto FAIL;
        } else if (sscanf(str,"%04d%02d%02d",&tt.year,&tt.month,&tt.day) < 3) {
	    goto FAIL;
        }    
    } else {
       if (size > 16 ) {
         char dsep1, dsep2, tsep, tsep1, tsep2;      
         if (sscanf(str,"%04d%c%02d%c%02d%c%02d%c%02d%c%02d",&tt.year,&dsep1,&tt.month,&dsep2,
                &tt.day,&tsep,&tt.hour,&tsep1,&tt.minute,&tsep2,&tt.second) < 11)
	    goto FAIL;

	if((tsep != 'T') || (dsep1 != '-') || (dsep2 != '-') || (tsep1 != ':') || (tsep2 != ':'))
	    goto FAIL;

       } else {        
	char tsep;
	if (sscanf(str,"%04d%02d%02d%c%02d%02d%02d",&tt.year,&tt.month,&tt.day,
	       &tsep,&tt.hour,&tt.minute,&tt.second) < 7)
	    goto FAIL;

	if(tsep != 'T')
	    goto FAIL;
       }
    }

    return tt;    

FAIL:
    icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
    return icaltime_null_time();
}



const char* icalrecur_first_clause(struct icalrecur_parser *parser)
{
	char *idx;
	parser->this_clause = parser->copy;

	idx = strchr(parser->this_clause,';');

	if (idx == 0){
		parser->next_clause = 0;
		return 0;
	}

	*idx = 0;
	idx++;
	parser->next_clause = idx;

	return parser->this_clause;

}

const char* icalrecur_next_clause(struct icalrecur_parser *parser)
{
	char* idx;

	parser->this_clause = parser->next_clause;

	if(parser->this_clause == 0){
		return 0;
	}

	idx = strchr(parser->this_clause,';');

	if (idx == 0){
		parser->next_clause = 0;
	} else {

		*idx = 0;
		idx++;
		parser->next_clause = idx;
	}

	return parser->this_clause;

}

void icalrecur_clause_name_and_value(struct icalrecur_parser *parser,
									 char** name, char** value)
{
	char *idx;

	*name = parser->this_clause;

	idx = strchr(parser->this_clause,'=');

	if (idx == 0){
		*name = 0;
		*value = 0;
		return;
	}

	*idx = 0;
	idx++;
	*value = idx;
}

void icalrecur_add_byrules(struct icalrecur_parser *parser, short *array,
						   int size, char* vals)
{
	char *t, *n;
	int i=0;
	int sign = 1;
	int v;

	n = vals;

	while(n != 0){

		if(i == size){
			return;
		}

		t = n;

		n = strchr(t,',');

		if(n != 0){
			*n = 0;
			n++;
		}

		/* Get optional sign. HACK. sign is not allowed for all BYxxx
		rule parts */
		if( *t == '-'){
			sign = -1;
			t++;
		} else if (*t == '+'){
			sign = 1;
			t++;
		} else {
			sign = 1;
		}

		v = atoi(t) * sign ;


		array[i++] = (short)v;
		array[i] =  ICAL_RECURRENCE_ARRAY_MAX;

	}

}

/*
* Days in the BYDAY rule are expected by the code to be sorted, and while
* this may be the common case, the RFC doesn't actually mandate it. This
* function sorts the days taking into account the first day of week.
*/
static void
sort_bydayrules(short *array, int week_start)
{
	int one, two, i, j;

	for (i=0;
		i<ICAL_BY_DAY_SIZE && array[i] != ICAL_RECURRENCE_ARRAY_MAX;
		i++) {
			for (j=0; j<i; j++) {
				one = icalrecurrencetype_day_day_of_week(array[j]) - week_start;
				if (one < 0) one += 7;
				two = icalrecurrencetype_day_day_of_week(array[i]) - week_start;
				if (two < 0) two += 7;

				if (one > two) {
					short tmp = array[j];
					array[j] = array[i];
					array[i] = tmp;
				}
			}
	}
}

void icalrecur_add_bydayrules(struct icalrecur_parser *parser, const char* vals)
{

	char *t, *n;
	int i=0;
	int sign = 1;
	int weekno = 0;
	icalrecurrencetype_weekday wd;
	short *array = parser->rt.by_day;
	char* end;
	char* vals_copy;

	vals_copy = _strdup(vals);

	end = (char*)vals_copy+strlen(vals_copy);
	n = vals_copy;

	array[0] = ICAL_RECURRENCE_ARRAY_MAX;

	while(n != 0){


		t = n;

		n = strchr(t,',');

		if(n != 0){
			*n = 0;
			n++;
		}

		/* Get optional sign. */
		if( *t == '-'){
			sign = -1;
			t++;
		} else if (*t == '+'){
			sign = 1;
			t++;
		} else {
			sign = 1;
		}

		/* Get Optional weekno */
		weekno = strtol(t,&t,10);

		/* Outlook/Exchange generate "BYDAY=MO, FR" and "BYDAY=2 TH".
		* Cope with that.
		*/
		if (*t == ' ')
			t++;

		wd = icalrecur_string_to_weekday(t);

		if (wd != ICAL_NO_WEEKDAY) {
			array[i++] = (short) (sign * (wd + 8 * weekno));
			array[i] = ICAL_RECURRENCE_ARRAY_MAX;
		}
	}

	free(vals_copy);

	sort_bydayrules(parser->rt.by_day, parser->rt.week_start);
}


struct icalrecurrencetype icalrecurrencetype_from_string(const char* str)
{
	struct icalrecur_parser parser;

	memset(&parser,0,sizeof(parser));
	icalrecurrencetype_clear(&parser.rt);

	icalerror_check_arg_re(str!=0,"str",parser.rt);

	/* Set up the parser struct */
	parser.rule = str;
	parser.copy = _strdup(parser.rule);
	parser.this_clause = parser.copy;

	if(parser.copy == 0){
		//icalerror_set_errno(ICAL_NEWFAILED_ERROR);
		return parser.rt;
	}

	/* Loop through all of the clauses */
	for(icalrecur_first_clause(&parser); 
		parser.this_clause != 0;
		icalrecur_next_clause(&parser))
	{
		char *name, *value;
		icalrecur_clause_name_and_value(&parser,&name,&value);

		if(name == 0){
			icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
			icalrecurrencetype_clear(&parser.rt);
			free(parser.copy);
			return parser.rt;
		}

		if (strcasecmp(name,"FREQ") == 0){
			parser.rt.freq = icalrecur_string_to_freq(value);
		} else if (strcasecmp(name,"COUNT") == 0){
			int v = atoi(value);
			if (v >= 0) {
				parser.rt.count = v;
			}
		} else if (strcasecmp(name,"UNTIL") == 0){
			parser.rt.until = icaltime_from_string(value);
		} else if (strcasecmp(name,"INTERVAL") == 0){
			int v = atoi(value);
			if (v > 0 && v <= SHRT_MAX) {
				parser.rt.interval = (short) v;
			}
		} else if (strcasecmp(name,"WKST") == 0){
			parser.rt.week_start = icalrecur_string_to_weekday(value);
			sort_bydayrules(parser.rt.by_day, parser.rt.week_start);
		} else if (strcasecmp(name,"BYSECOND") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_second,
				ICAL_BY_SECOND_SIZE,value);
		} else if (strcasecmp(name,"BYMINUTE") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_minute,
				ICAL_BY_MINUTE_SIZE,value);
		} else if (strcasecmp(name,"BYHOUR") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_hour,
				ICAL_BY_HOUR_SIZE,value);
		} else if (strcasecmp(name,"BYDAY") == 0){
			icalrecur_add_bydayrules(&parser,value);
		} else if (strcasecmp(name,"BYMONTHDAY") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_month_day,
				ICAL_BY_MONTHDAY_SIZE,value);
		} else if (strcasecmp(name,"BYYEARDAY") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_year_day,
				ICAL_BY_YEARDAY_SIZE,value);
		} else if (strcasecmp(name,"BYWEEKNO") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_week_no,
				ICAL_BY_WEEKNO_SIZE,value);
		} else if (strcasecmp(name,"BYMONTH") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_month,
				ICAL_BY_MONTH_SIZE,value);
		} else if (strcasecmp(name,"BYSETPOS") == 0){
			icalrecur_add_byrules(&parser,parser.rt.by_set_pos,
				ICAL_BY_SETPOS_SIZE,value);
		} else {
			icalerror_set_errno(ICAL_MALFORMEDDATA_ERROR);
			icalrecurrencetype_clear(&parser.rt);
			free(parser.copy);
			return parser.rt;
		}

	}

	free(parser.copy);

	return parser.rt;

}

static struct _BYRULU_{const char* str;size_t offset; int limit;  } recurmap[] = 
{
	{";BYSECOND=",offsetof(struct icalrecurrencetype,by_second),ICAL_BY_SECOND_SIZE - 1},
	{";BYMINUTE=",offsetof(struct icalrecurrencetype,by_minute),ICAL_BY_MINUTE_SIZE - 1},
	{";BYHOUR=",offsetof(struct icalrecurrencetype,by_hour),ICAL_BY_HOUR_SIZE - 1},
	{";BYDAY=",offsetof(struct icalrecurrencetype,by_day),ICAL_BY_DAY_SIZE - 1},
	{";BYMONTHDAY=",offsetof(struct icalrecurrencetype,by_month_day),ICAL_BY_MONTHDAY_SIZE - 1},
	{";BYYEARDAY=",offsetof(struct icalrecurrencetype,by_year_day),ICAL_BY_YEARDAY_SIZE - 1},
	{";BYWEEKNO=",offsetof(struct icalrecurrencetype,by_week_no),ICAL_BY_WEEKNO_SIZE - 1},
	{";BYMONTH=",offsetof(struct icalrecurrencetype,by_month),ICAL_BY_MONTH_SIZE - 1},
	{";BYSETPOS=",offsetof(struct icalrecurrencetype,by_set_pos),ICAL_BY_SETPOS_SIZE - 1},
	{0,0,0},
};

/* A private routine in icalvalue.c */


char* icalrecurrencetype_as_string(struct icalrecurrencetype *recur)
{
	char *buf;
	buf = icalrecurrencetype_as_string_r(recur);
	return buf;
}


char* icalrecurrencetype_as_string_r(struct icalrecurrencetype *recur)
{
	char* str;
	char *str_p;
	size_t buf_sz = 200;
	char temp[20];
	int i,j;

	if(recur->freq == ICAL_NO_RECURRENCE){
		return 0;
	}

	str = (char*)calloc(1,buf_sz);
	str_p = str;

	icalmemory_append_string(&str,&str_p,&buf_sz,"FREQ=");
	icalmemory_append_string(&str,&str_p,&buf_sz,
		icalrecur_freq_to_string(recur->freq));

	if(recur->until.year != 0){

		temp[0] = 0;
		if (recur->until.is_date)
			print_date_to_string(temp,&(recur->until));
		else
			print_datetime_to_string(temp,&(recur->until));

		icalmemory_append_string(&str,&str_p,&buf_sz,";UNTIL=");
		icalmemory_append_string(&str,&str_p,&buf_sz, temp);
	}

	if(recur->count != 0){
		snprintf(temp,sizeof(temp),"%d",recur->count);
		icalmemory_append_string(&str,&str_p,&buf_sz,";COUNT=");
		icalmemory_append_string(&str,&str_p,&buf_sz, temp);
	}

	if(recur->interval != 1){
		snprintf(temp,sizeof(temp),"%d",recur->interval);
		icalmemory_append_string(&str,&str_p,&buf_sz,";INTERVAL=");
		icalmemory_append_string(&str,&str_p,&buf_sz, temp);
	}

	for(j =0; recurmap[j].str != 0; j++){
		short* array = (short*)(recurmap[j].offset+ (size_t)recur);
		int limit = recurmap[j].limit;

		/* Skip unused arrays */
		if( array[0] != ICAL_RECURRENCE_ARRAY_MAX ) {

			icalmemory_append_string(&str,&str_p,&buf_sz,recurmap[j].str);

			for(i=0; 
				i< limit  && array[i] != ICAL_RECURRENCE_ARRAY_MAX;
				i++){
					if (j == 3) { /* BYDAY */
						const char *daystr = icalrecur_weekday_to_string(
							icalrecurrencetype_day_day_of_week(array[i]));
						int pos;

						pos = icalrecurrencetype_day_position(array[i]);  

						if (pos == 0)
							icalmemory_append_string(&str,&str_p,&buf_sz,daystr);
						else {
							snprintf(temp,sizeof(temp),"%d%s",pos,daystr);
							icalmemory_append_string(&str,&str_p,&buf_sz,temp);
						}                  

					} else {
						snprintf(temp,sizeof(temp),"%d",array[i]);
						icalmemory_append_string(&str,&str_p,&buf_sz, temp);
					}

					if( (i+1)<limit &&array[i+1] 
					!= ICAL_RECURRENCE_ARRAY_MAX){
						icalmemory_append_char(&str,&str_p,&buf_sz,',');
					}
			}	 
		}
	}

	/* Monday is the default, so no need to write that out */
	if ( recur->week_start != ICAL_MONDAY_WEEKDAY && recur->week_start != ICAL_NO_WEEKDAY ) {
		const char *daystr = icalrecur_weekday_to_string(
			icalrecurrencetype_day_day_of_week( recur->week_start ));
		icalmemory_append_string(&str,&str_p,&buf_sz,";WKST=");
		icalmemory_append_string(&str,&str_p,&buf_sz,daystr);
	}

	return  str;
}


