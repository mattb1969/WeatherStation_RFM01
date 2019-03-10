/*****************************************************************************
* File:		fo_dayfile.c
*
* Overview:	This file implements functions to write the weather data to a file
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fo.h"

extern MINMAX	wmm[2];
extern WDATA	wdata[20];
extern char		now[20], today[20];

FILE 			*details;


/*****************************************************************************
* Function:	create_conn
*
* Overview:	This function is named to be consistent with the database functions.
*			It calls dayfile_read() to initialize the min/max data structure
8			and creates the details file handle.
*
******************************************************************************/
void create_conn()
{
	dayfile_read();
	details = fopen("fopi.details", "a+");
}


/*****************************************************************************
* Function:	close_conn
*
* Overview:	This is just a dummy function to be consistent with the databases
*
******************************************************************************/
void close_conn()
{
	// nothing to do here
}



/*****************************************************************************
* Function:	dayfile_read
*
* Overview:	This function is called from create_conn() to initialize the
*			min/max data structure.
*
******************************************************************************/
void dayfile_read()
{
	char		line[150];
	uint8_t 	ok = FALSE;
	FILE 		*dayfile;

	dayfile = fopen("fopi.dayfile","rt");

	printf("Opended fopi.dayfile\n");

	if(dayfile!=NULL)
	{
		while(fgets(line, 150, dayfile) != NULL)
		{
			// Read the file and search for possible today's data
			sscanf(line, "%[^','],", wmm[MIN].date);
			if(strncmp(today, wmm[MIN].date, 10)==0)
			{
				// There are already min/max values for today in the file,
				// so copy them to the min/max data structure.
				sscanf(&line[13], "%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%u",
					&wmm[MIN].temperature, wmm[MIN].temperature_t, &wmm[MIN].windchill, wmm[MIN].windchill_t,
					&wmm[MIN].apparent, wmm[MIN].apparent_t, &wmm[MIN].dewpoint, wmm[MIN].dewpoint_t,
					&wmm[MIN].humidity, wmm[MIN].humidity_t, &wmm[MIN].windspeed, wmm[MIN].windspeed_t,
					&wmm[MIN].windgust, wmm[MIN].windgust_t, &wmm[MIN].pressure, wmm[MIN].pressure_t, (unsigned int *)&wmm[MIN].rain);
				fgets(line, 150, dayfile);
				sscanf(&line[13], "%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%u",
					&wmm[MAX].temperature, wmm[MAX].temperature_t, &wmm[MAX].windchill, wmm[MAX].windchill_t,
					&wmm[MAX].apparent, wmm[MAX].apparent_t, &wmm[MAX].dewpoint, wmm[MAX].dewpoint_t,
					&wmm[MAX].humidity, wmm[MAX].humidity_t, &wmm[MAX].windspeed, wmm[MAX].windspeed_t,
					&wmm[MAX].windgust, wmm[MAX].windgust_t, &wmm[MAX].pressure, wmm[MAX].pressure_t, (unsigned int *)&wmm[MAX].rain);
				ok = TRUE;
			}
		}
	}

	if(!ok)
	{
		uint16_t rain = 0;
		// Get the latest rain counter, if fopi.dayfile exists
		// Possible not the correct rain value, but better than 0
		if(dayfile!=NULL)
		{
			sscanf(&line[13], "%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%f,%[^','],%u",
				&wmm[MAX].temperature, wmm[MAX].temperature_t, &wmm[MAX].windchill, wmm[MAX].windchill_t,
				&wmm[MAX].apparent, wmm[MAX].apparent_t, &wmm[MAX].dewpoint, wmm[MAX].dewpoint_t,
				&wmm[MAX].humidity, wmm[MAX].humidity_t, &wmm[MAX].windspeed, wmm[MAX].windspeed_t,
				&wmm[MAX].windgust, wmm[MAX].windgust_t, &wmm[MAX].pressure, wmm[MAX].pressure_t, (unsigned int *)&rain);
		}
		// No fopi.dayfile file or no record for today, set start values for min/max
		reset_min_max();
		wmm[MIN].rain = wmm[MAX].rain = rain;
	}

	fclose(dayfile);
}


/*****************************************************************************
* Function:	dayfile_write
*
* Overview:	This is the 'mirror' function to dayfile_read(). It checks if a
*			record for the current day already exists and either inserts a
*			new record or updates the existing one. This function is called
*			by process_data() at midnight and by pgm_end()
*
******************************************************************************/
void dayfile_write()
{
	FILE	*dayin, *dayout;
	char	line[150];

	dayin = fopen("fopi.dayfile","r");
	if(dayin==NULL)
	{
		// fopi.dayfile does not exist, so create it and write today's min/max data
		dayout = fopen("fopi.dayfile","a+");
		fprintf(dayout, "%s,%d,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%d\n",
			wmm[MIN].date, 0, wmm[MIN].temperature, wmm[MIN].temperature_t, wmm[MIN].windchill, wmm[MIN].windchill_t,
			wmm[MIN].apparent, wmm[MIN].apparent_t, wmm[MIN].dewpoint, wmm[MIN].dewpoint_t,
			wmm[MIN].humidity, wmm[MIN].humidity_t, wmm[MIN].windspeed, wmm[MIN].windspeed_t,
			wmm[MIN].windgust, wmm[MIN].windgust_t, wmm[MIN].pressure, wmm[MIN].pressure_t, wmm[MIN].rain);
		fprintf(dayout, "%s,%d,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%d\n",
			wmm[MIN].date, 1, wmm[MAX].temperature, wmm[MAX].temperature_t, wmm[MAX].windchill, wmm[MAX].windchill_t,
			wmm[MAX].apparent, wmm[MAX].apparent_t, wmm[MAX].dewpoint, wmm[MAX].dewpoint_t,
			wmm[MAX].humidity, wmm[MAX].humidity_t, wmm[MAX].windspeed, wmm[MAX].windspeed_t,
			wmm[MAX].windgust, wmm[MAX].windgust_t, wmm[MAX].pressure, wmm[MAX].pressure_t, wmm[MAX].rain);
		fclose(dayout);
	}
	else
	{
		// fopi.dayfile already exists, so read until we find today's record or EOF and copy to tmp file
		dayout = fopen("dayfile.tmp","w");
		while(fgets(line, 150, dayin) != NULL)
		{
			if(strncmp(today, line, 10)>0) fprintf(dayout, "%s", line);
			else break;
		}
		// write today's min/max values
		fprintf(dayout, "%s,%d,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%d\n",
			wmm[MIN].date, 0, wmm[MIN].temperature, wmm[MIN].temperature_t, wmm[MIN].windchill, wmm[MIN].windchill_t,
			wmm[MIN].apparent, wmm[MIN].apparent_t, wmm[MIN].dewpoint, wmm[MIN].dewpoint_t,
			wmm[MIN].humidity, wmm[MIN].humidity_t, wmm[MIN].windspeed, wmm[MIN].windspeed_t,
			wmm[MIN].windgust, wmm[MIN].windgust_t, wmm[MIN].pressure, wmm[MIN].pressure_t, wmm[MIN].rain);
		fprintf(dayout, "%s,%d,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%.1f,%s,%d\n",
			wmm[MIN].date, 1, wmm[MAX].temperature, wmm[MAX].temperature_t, wmm[MAX].windchill, wmm[MAX].windchill_t,
			wmm[MAX].apparent, wmm[MAX].apparent_t, wmm[MAX].dewpoint, wmm[MAX].dewpoint_t,
			wmm[MAX].humidity, wmm[MAX].humidity_t, wmm[MAX].windspeed, wmm[MAX].windspeed_t,
			wmm[MAX].windgust, wmm[MAX].windgust_t, wmm[MAX].pressure, wmm[MAX].pressure_t, wmm[MAX].rain);
		fclose(dayout);
		fclose(dayin);
		// delet old fopi.dayfile ...
		remove("fopi.dayfile");
		// ... and rename tmp file
		rename("dayfile.tmp", "fopi.dayfile");
	}
}


/*****************************************************************************
* Function:	details_write
*
* Overview:	This function writes the received or averaged weather data to a file
*
******************************************************************************/
void details_write(uint8_t wd)
{
	fprintf(details, "%s,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
		wdata[wd].datetime, wdata[wd].temperature, wdata[wd].windchill,
		wdata[wd].apparent, wdata[wd].dewpoint, wdata[wd].humidity,
		wdata[wd].windspeed, wdata[wd].windgust, wdata[wd].windbearing,
		wdata[wd].pressure, wdata[wd].rain);
}
