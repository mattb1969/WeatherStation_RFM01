/*****************************************************************************
* File:		fo_util.c
*
* Overview:	This file implements some functions used in other fo_xxx.c programs
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include "fo.h"

extern WDATA	wdata[20];
extern MINMAX	wmm[2];


/*****************************************************************************
* Function: crc8
*
* Overview:	This function calculates the 8bit CRC for len bytes in buffer
*			Function taken from Luc Small (http://lucsmall.com) and modified
*			by Kevin Sangeelee (http://www.susa.net/wordpress/)
*
******************************************************************************/
uint8_t crc8(uint8_t *buffer, uint8_t len)
{
    uint8_t i, crc = 0;

    // Indicated changes are from reference CRC-8 function in OneWire library
    while (len--)
    {
        uint8_t inbyte = *buffer++;
        for (i=8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x80; 	// changed from & 0x01
            crc <<= 1; 								// changed from right shift
            if (mix) crc ^= 0x31;					// changed from 0x8C;
            inbyte <<= 1; 							// changed from right shift
        }
    }
    return crc;
}


/*****************************************************************************
* Function:	scheduler
*
* Overview:	This function switches the RasPi niceness of processes to REALTIME
*			mode (high priority) or STANDARD mode (normal priority)
*			Function taken from Kevin Sangeelees' WH1080 project
*
******************************************************************************/
void scheduler(uint8_t mode)
{
    struct sched_param p;

    printf("Realtime Scheduler %i\n",mode);
    if(mode==REALTIME)
    {
        p.__sched_priority = sched_get_priority_max(SCHED_RR);
        if(sched_setscheduler(0, SCHED_RR, &p)==-1)
            printf("Failed to switch to realtime scheduler.\n");
    }
    else
    {
        p.__sched_priority = 0;
        if(sched_setscheduler(0, SCHED_OTHER, &p)==-1 )
            printf("Failed to switch to normal scheduler.\n");
    }
}


/*****************************************************************************
* Function:	reset_min_max
*
* Overview:	This function sets the min/max to the default values at midnight
*			or if at program start there are no values in the datafile /
*			database for the current day
*
******************************************************************************/
void reset_min_max()
{
	extern char		today[20];
	char time[] = "00:00:00\0";

	strncpy(wmm[MIN].date, today, 10); wmm[MIN].date[10] = 0;
	wmm[MIN].temperature = 9999;	strcpy(wmm[MIN].temperature_t, time);
	wmm[MAX].temperature = -9999;	strcpy(wmm[MAX].temperature_t, time);
	wmm[MIN].windchill   = 9999;	strcpy(wmm[MIN].windchill_t, time);
	wmm[MAX].windchill   = -9999;	strcpy(wmm[MAX].windchill_t, time);
	wmm[MIN].apparent    = 9999;	strcpy(wmm[MIN].apparent_t, time);
	wmm[MAX].apparent    = -9999;	strcpy(wmm[MAX].apparent_t, time);
	wmm[MIN].dewpoint    = 9999;	strcpy(wmm[MIN].dewpoint_t, time);
	wmm[MAX].dewpoint    = -9999;	strcpy(wmm[MAX].dewpoint_t, time);
	wmm[MIN].humidity    = 9999;	strcpy(wmm[MIN].humidity_t, time);
	wmm[MAX].humidity    = -9999;	strcpy(wmm[MAX].humidity_t, time);
	wmm[MIN].windspeed   = 9999;	strcpy(wmm[MIN].windspeed_t, time);
	wmm[MAX].windspeed   = -9999;	strcpy(wmm[MAX].windspeed_t, time);
	wmm[MIN].windgust    = 9999;	strcpy(wmm[MIN].windgust_t, time);
	wmm[MAX].windgust    = -9999;	strcpy(wmm[MAX].windgust_t, time);
	wmm[MIN].pressure    = 9999;	strcpy(wmm[MIN].pressure_t, time);
	wmm[MAX].pressure    = -9999;	strcpy(wmm[MAX].pressure_t, time);
	wmm[MIN].rain = wmm[MAX].rain;
}


#if defined(MYSQL5) || defined(SQLITE3)

extern char sql[1000];

/*****************************************************************************
* Function:	details_write_query
*
* Overview:	This function generates the SQL query to insert a new record into
*			the 'details' table of the MySQL or Sqlite3 database
*
******************************************************************************/
void details_write_query(uint8_t wd)
{
    char format[] = "INSERT INTO details (datetime, year, month, day, time,\
		temperature, windchill, apparent, dewpoint, humidity, windspeed,\
		windgust, windbearing, pressure, rain) VALUES ('%s', %s, %s, %s,\
		'%s', %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f)";
	char datetime[20], year[3], month[3], day[3], time[6];

	strcpy(datetime, wdata[wd].datetime);		datetime[19] = 0;
	strncpy(year, &wdata[wd].datetime[2], 2);	year[2] = 0;
	strncpy(month, &wdata[wd].datetime[5], 2);	month[2] = 0;
	strncpy(day, &wdata[wd].datetime[8], 2);	day[2] = 0;
	strncpy(time, &wdata[wd].datetime[11], 5);	time[5] = 0;

    sprintf(sql, format, datetime, year, month, day, time, wdata[wd].temperature,
		wdata[wd].windchill, wdata[wd].apparent, wdata[wd].dewpoint,
		wdata[wd].humidity, wdata[wd].windspeed, wdata[wd].windgust,
		wdata[wd].windbearing, wdata[wd].pressure, wdata[wd].rain);
}


/*****************************************************************************
* Function:	dayfile_write_query
*
* Overview:	This function generates the SQL query to insert a new record /
*			update an existing record in the 'dayfile' table of the MySQL or
*			Sqlite3 database
*
******************************************************************************/
void dayfile_write_query(uint8_t mode, uint8_t type)
{
	if(mode==INSERT)
	{
		char format[] = "INSERT INTO dayfile (date, type, temperature, temperature_t,\
			windchill, windchill_t, apparent, apparent_t, dewpoint, dewpoint_t,\
			humidity, humidity_t, windspeed, windspeed_t, windgust, windgust_t,\
			pressure, pressure_t, rain) VALUES ('%s',%d,%.1f,'%s',%.1f,'%s',%.1f,\
			'%s',%.1f,'%s',%.1f,'%s',%.1f,'%s',%.1f,'%s',%.1f,'%s',%d)";
		sprintf(sql, format, wmm[MIN].date, type, wmm[type].temperature, wmm[type].temperature_t,
			wmm[type].windchill, wmm[type].windchill_t, wmm[type].apparent, wmm[type].apparent_t,
			wmm[type].dewpoint, wmm[type].dewpoint_t, wmm[type].humidity, wmm[type].humidity_t,
			wmm[type].windspeed, wmm[type].windspeed_t, wmm[type].windgust, wmm[type].windgust_t,
			wmm[type].pressure, wmm[type].pressure_t, wmm[type].rain);
	}
	else
	{
		char format[] = "UPDATE dayfile SET temperature=%.1f, temperature_t='%s',\
			windchill=%.1f, windchill_t='%s', apparent=%.1f, apparent_t='%s',\
			dewpoint=%.1f, dewpoint_t='%s', humidity=%.1f, humidity_t='%s',\
			windspeed=%.1f, windspeed_t='%s', windgust=%.1f, windgust_t='%s',\
			pressure=%.1f, pressure_t='%s', rain=%d WHERE date='%s' AND type=%d";
		sprintf(sql, format, wmm[type].temperature, wmm[type].temperature_t,
			wmm[type].windchill, wmm[type].windchill_t, wmm[type].apparent, wmm[type].apparent_t,
			wmm[type].dewpoint, wmm[type].dewpoint_t, wmm[type].humidity, wmm[type].humidity_t,
			wmm[type].windspeed, wmm[type].windspeed_t, wmm[type].windgust, wmm[type].windgust_t,
			wmm[type].pressure, wmm[type].pressure_t, wmm[type].rain, wmm[MIN].date, type);
	}
}
#endif
