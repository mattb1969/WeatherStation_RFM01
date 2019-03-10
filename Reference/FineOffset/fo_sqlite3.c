/*****************************************************************************
* File:		fo_sqlite3.c
*
* Overview:	This file implements the functions needed for the Sqlite3 database
*
******************************************************************************/

#include "fo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

extern ARG		arg;
extern WDATA	wdata[20];
extern MINMAX	wmm[2];
extern char		now[20], today[20];
extern FILE		*logfile;

sqlite3			*conn;
sqlite3_stmt 	*stmt;
int8_t			rc;

char 			sql[1000];


/*****************************************************************************
* Function:	create_conn
*
* Overview:	This function creates a connection to the Sqlite3 database
*
******************************************************************************/
void create_conn()
{
	char errmsg[] = "%s Database connection failed\n";

    if(sqlite3_open("weather.db", &conn))
    {
		if(arg.logging) fprintf(logfile, errmsg, now);
		printf(errmsg, now); fflush(stdout); exit(1);
    }
    // Check to see if there is already a record with min/max values for today
    dayfile_read();
}


/*****************************************************************************
* Function:	close_conn
*
* Overview:	This function closes the connection established by create_conn()
*
******************************************************************************/
void close_conn()
{
    sqlite3_close(conn);
}


/*****************************************************************************
* Function:	dayfile_read
*
* Overview:	This function is called from create_conn() to initialize the
*			min/max data structure. Note: the table must exist prior to a
*			call to this function
*
* --
* -- Table structure for table dayfile
* --
*    CREATE TABLE IF NOT EXISTS dayfile (date TEXT PRIMARY KEY, type TEXT,
*		temperature REAL, temperature_t TEXT, windchill REAL, windchill_t TEXT,
*		apparent REAL, apparrent_t TEXT, dewpoint REAL, dewpoint_t TEXT,
*		humidity REAL, humidity_t TEXT, windspeed REAL, windspeed_t TEXT,
*		windgust REAL, windgust_t TEXT, pressure REAL, pressure_t TEXT,
*		rain INTEGER);
*
******************************************************************************/

void dayfile_read()
{
	char errmsg[] = "SQL error: %d : %s\n";
	char testdate[11];

	rc = sqlite3_prepare_v2(conn, "SELECT * FROM dayfile ORDER BY date DESC, type DESC", -1, &stmt, 0);
    if(rc)
    {
    	if(arg.logging) fprintf(logfile, errmsg, rc, sqlite3_errmsg(conn));
		printf(errmsg, rc, sqlite3_errmsg(conn)); fflush(stdout); exit(1);
	}
	else
	{
		reset_min_max();

		rc = sqlite3_step(stmt);
		if(rc==SQLITE_DONE)
		{
			// Should only happen when table is empty
			// Do nothing, since min/max is already initialized
		}
		else if(rc==SQLITE_ROW)
		{
			// What is the date of the most recent record?
			strcpy(testdate, (char *)sqlite3_column_text(stmt, 0));
			if(strncmp(testdate, today, 10)==0)
			{
				// There are already min/max values for today in the database,
				// so copy them to the min/max structure. The first record is MAX
				wmm[MAX].temperature = sqlite3_column_double(stmt, 2);
				strcpy(wmm[MAX].temperature_t, (char *)sqlite3_column_text(stmt, 3));
				wmm[MAX].windchill = sqlite3_column_double(stmt, 4);
				strcpy(wmm[MAX].windchill_t, (char *)sqlite3_column_text(stmt, 5));
				wmm[MAX].apparent = sqlite3_column_double(stmt, 6);
				strcpy(wmm[MAX].apparent_t, (char *)sqlite3_column_text(stmt, 7));
				wmm[MAX].dewpoint = sqlite3_column_double(stmt, 8);
				strcpy(wmm[MAX].dewpoint_t, (char *)sqlite3_column_text(stmt, 9));
				wmm[MAX].humidity = sqlite3_column_double(stmt, 10);
				strcpy(wmm[MAX].humidity_t, (char *)sqlite3_column_text(stmt, 11));
				wmm[MAX].windspeed = sqlite3_column_double(stmt, 12);
				strcpy(wmm[MAX].windspeed_t, (char *)sqlite3_column_text(stmt, 13));
				wmm[MAX].windgust = sqlite3_column_double(stmt, 14);
				strcpy(wmm[MAX].windgust_t, (char *)sqlite3_column_text(stmt, 15));
				wmm[MAX].pressure = sqlite3_column_double(stmt, 16);
				strcpy(wmm[MAX].pressure_t, (char *)sqlite3_column_text(stmt, 17));
				wmm[MAX].rain = sqlite3_column_int(stmt, 18);
				// read MIN record and copy values
				sqlite3_step(stmt);
				wmm[MIN].temperature = sqlite3_column_double(stmt, 2);
				strcpy(wmm[MIN].temperature_t, (char *)sqlite3_column_text(stmt, 3));
				wmm[MIN].windchill = sqlite3_column_double(stmt, 4);
				strcpy(wmm[MIN].windchill_t, (char *)sqlite3_column_text(stmt, 5));
				wmm[MIN].apparent = sqlite3_column_double(stmt, 6);
				strcpy(wmm[MIN].apparent_t, (char *)sqlite3_column_text(stmt, 7));
				wmm[MIN].dewpoint = sqlite3_column_double(stmt, 8);
				strcpy(wmm[MIN].dewpoint_t, (char *)sqlite3_column_text(stmt, 9));
				wmm[MIN].humidity = sqlite3_column_double(stmt, 10);
				strcpy(wmm[MIN].humidity_t, (char *)sqlite3_column_text(stmt, 11));
				wmm[MIN].windspeed = sqlite3_column_double(stmt, 12);
				strcpy(wmm[MIN].windspeed_t, (char *)sqlite3_column_text(stmt, 13));
				wmm[MIN].windgust = sqlite3_column_double(stmt, 14);
				strcpy(wmm[MIN].windgust_t, (char *)sqlite3_column_text(stmt, 15));
				wmm[MIN].pressure = sqlite3_column_double(stmt, 16);
				strcpy(wmm[MIN].pressure_t, (char *)sqlite3_column_text(stmt, 17));
				wmm[MIN].rain = sqlite3_column_int(stmt, 18);
			}
			else
			{
				// Set the rain counter to the most recent max value.
				// This is probably the wrong value, but better than 0 ... I think
				wmm[MIN].rain = wmm[MAX].rain = sqlite3_column_int(stmt, 18);
			}
		}
		else
		{
			// Should not happen, but you never know ...
			printf("Error: %d : %s\n",  rc, sqlite3_errmsg(conn));
			fflush(stdout); exit(1);
		}
	}

	// finalize the statement to release resources
	sqlite3_finalize(stmt);
}


/*****************************************************************************
* Function:	dayfile_write
*
* Overview:	This is the 'mirror' function to dayfile_read. It checks if a
*			record for the current day already exists and either inserts a
*			new record or updates the existing one. This function is called
*			by process_data() at midnight and by pgm_end()
*
******************************************************************************/

void dayfile_write()
{
	char errmsg1[] = "SQL error: %d : %s\n";
	char errmsg2[] = "%s Insert into database failed: %s\n";
	char errmsg3[] = "%s Update of database failed: %s\n";

	sprintf(sql, "SELECT date FROM dayfile WHERE date='%s'", wmm[MIN].date);

	rc = sqlite3_prepare_v2(conn, sql, -1, &stmt, 0);
    if(rc)
    {
    	if(arg.logging) fprintf(logfile, errmsg1, rc, sqlite3_errmsg(conn));
		printf(errmsg1, rc, sqlite3_errmsg(conn)); fflush(stdout); exit(1);
	}
	else
	{
		rc = sqlite3_step(stmt);
		if(rc==SQLITE_DONE)
		{
			// no record found, insert new
			dayfile_write_query(INSERT, MIN);
			if(sqlite3_exec(conn, sql, 0, 0, 0))
			{
				if(arg.logging) fprintf(logfile, errmsg2, now, sql);
				printf(errmsg2, now, sql); fflush(stdout); exit(1);
			}

			dayfile_write_query(INSERT, MAX);
			if(sqlite3_exec(conn, sql, 0, 0, 0))
			{
				if(arg.logging) fprintf(logfile, errmsg2, now, sql);
				printf(errmsg2, now, sql); fflush(stdout); exit(1);
			}
		}
		else if(rc==SQLITE_ROW)
		{
			// update records
			dayfile_write_query(UPDATE, MIN);
			if(sqlite3_exec(conn, sql, 0, 0, 0))
			{
				if(arg.logging) fprintf(logfile, errmsg3, now, sql);
				printf(errmsg3, now, sql); fflush(stdout); exit(1);
			}

			dayfile_write_query(UPDATE, MAX);
			if(sqlite3_exec(conn, sql, 0, 0, 0))
			{
				if(arg.logging) fprintf(logfile, errmsg3, now, sql);
				printf(errmsg3, now, sql); fflush(stdout); exit(1);
			}
		}
	}

	// finalize the statement to release resources
	sqlite3_finalize(stmt);
}


/*****************************************************************************
* Function:	details_write
*
* Overview:	This function writes the received weather data to the 'details'
*			table. Note: the table must exist prior to a call to this function
*
* --
* -- Table structure for table details
* --
*    CREATE TABLE IF NOT EXISTS details (datetime TEXT PRIMARY KEY,
*		year INTEGER, month INTEGER, day INTEGER, time TEXT,
*		temperature REAL, dewpoint REAL, apparent REAL, windchill REAL,
*		humidity REAL, windspeed REAL, windgust REAL, windbearing REAL,
*		pressure REAL, rain REAL);
*
******************************************************************************/

void details_write(uint8_t wd)
{
	char errmsg[] = "%s Insert into database failed: %s\n";

	details_write_query(wd);
	if(sqlite3_exec(conn, sql, 0, 0, 0))
    {
		if(arg.logging) fprintf(logfile, errmsg, now, sql);
		printf(errmsg, now, sql); fflush(stdout); exit(1);
    }
}

