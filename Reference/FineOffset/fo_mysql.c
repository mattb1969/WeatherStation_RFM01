/*****************************************************************************
* File:		fo_mysql.c
*
* Overview:	This file implements the functions needed for the MySQL database
*
******************************************************************************/

// You need to add "include /usr/include/mysql/" to /etc/ld.so.conf for this to work
#include <mysql/mysql.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "fo.h"


MYSQL			*conn;
MYSQL_RES 		*result;
MYSQL_ROW 		row;
char			sql[1000];

extern ARG		arg;
extern WDATA	wdata[20];
extern MINMAX	wmm[2];
extern char		now[20], today[20];
extern FILE		*logfile;


/*****************************************************************************
* Function:	create_conn
*
* Overview:	This function creates a connection to the MySQL database
*
******************************************************************************/
void create_conn()
{
    char *server = "localhost";
    char *user = "<your user name>";
    char *password = "<your password>";
    char *database = "<your database>";
    conn = mysql_init(NULL);

    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        printf("%s\n", mysql_error(conn)); fflush(stdout); exit(1);
    }
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
    mysql_close(conn);
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
*	 CREATE TABLE IF NOT EXISTS dayfile (
*		date date NOT NULL, type tinyint(1) unsigned NOT NULL,
*   	temperature float NOT NULL, temperature_t time NOT NULL,
*   	windchill float NOT NULL, windchill_t time NOT NULL,
*   	apparent float NOT NULL, apparent_t time NOT NULL,
*   	dewpoint float NOT NULL, dewpoint_t time NOT NULL,
*   	humidity float NOT NULL, humidity_t time NOT NULL,
*   	windspeed float NOT NULL, windspeed_t time NOT NULL,
*   	windgust float NOT NULL, windgust_t time NOT NULL,
*   	pressure float NOT NULL, pressure_t time NOT NULL,
*   	rain smallint(6) unsigned NOT NULL,
*   	KEY date (date), KEY type (type)
*	 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;
*
******************************************************************************/
void dayfile_read()
{
    char testdate[10];

	reset_min_max();

    if (mysql_query(conn, "SELECT * FROM dayfile ORDER BY date DESC, type DESC LIMIT 2"))
    {
        printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
        fflush(stdout); exit(1);
    }
    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);

    if(row==NULL)
	{
		// Should only happen when table is empty
		// Do nothing, since min/max is already initialized
	}
	else
	{
		// What is the date of the most recent record?
		strcpy(testdate, row[0]);
		if(strncmp(testdate, today, 10)==0)
		{
			// There are already min/max values for today in the database,
			// so copy them to the min/max data structure. The first record is MAX
			wmm[MAX].temperature = atof(row[2]);	strcpy(wmm[MAX].temperature_t, row[3]);
			wmm[MAX].windchill = atof(row[4]);		strcpy(wmm[MAX].windchill_t, row[5]);
			wmm[MAX].apparent = atof(row[6]);		strcpy(wmm[MAX].apparent_t, row[7]);
			wmm[MAX].dewpoint = atof(row[8]);		strcpy(wmm[MAX].dewpoint_t, row[9]);
			wmm[MAX].humidity = atof(row[10]);		strcpy(wmm[MAX].humidity_t, row[11]);
			wmm[MAX].windspeed = atof(row[12]);		strcpy(wmm[MAX].windspeed_t, row[13]);
			wmm[MAX].windgust = atof(row[14]);		strcpy(wmm[MAX].windgust_t, row[15]);
			wmm[MAX].pressure = atof(row[16]);		strcpy(wmm[MAX].pressure_t, row[17]);
			wmm[MAX].rain = atoi(row[18]);
			// read MIN record and copy values
			row = mysql_fetch_row(result);
			wmm[MIN].temperature = atof(row[2]);	strcpy(wmm[MIN].temperature_t, row[3]);
			wmm[MIN].windchill = atof(row[4]);		strcpy(wmm[MIN].windchill_t, row[5]);
			wmm[MIN].apparent = atof(row[6]);		strcpy(wmm[MIN].apparent_t, row[7]);
			wmm[MIN].dewpoint = atof(row[8]);		strcpy(wmm[MIN].dewpoint_t, row[9]);
			wmm[MIN].humidity = atof(row[10]);		strcpy(wmm[MIN].humidity_t, row[11]);
			wmm[MIN].windspeed = atof(row[12]);		strcpy(wmm[MIN].windspeed_t, row[13]);
			wmm[MIN].windgust = atof(row[14]);		strcpy(wmm[MIN].windgust_t, row[15]);
			wmm[MIN].pressure = atof(row[16]);		strcpy(wmm[MIN].pressure_t, row[17]);
			wmm[MIN].rain = atoi(row[18]);
		}
		else
		{
			// Set the rain counter to the most recent max value.
			// This is possibly still a wrong value, but better than 0 ... I think
			wmm[MIN].rain = wmm[MAX].rain = atoi(row[18]);
		}
	}

    mysql_free_result(result);
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
	sprintf(sql, "SELECT date FROM dayfile WHERE date='%s'", wmm[MIN].date);
    if (mysql_query(conn, sql))
    {
        printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
        fflush(stdout);
        exit(1);
    }
    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    if(row==NULL)
	{
		// no record found, insert new
		dayfile_write_query(INSERT, MIN);
		if (mysql_query(conn, sql))
		{
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			fflush(stdout); exit(1);
		}
		dayfile_write_query(INSERT, MAX);
		if (mysql_query(conn, sql))
		{
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			fflush(stdout);	exit(1);
		}
	}
	else
	{
		// update records
		dayfile_write_query(UPDATE, MIN);
		if (mysql_query(conn, sql))
		{
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			fflush(stdout);	exit(1);
		}
		dayfile_write_query(UPDATE, MAX);
		if (mysql_query(conn, sql))
		{
			printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
			fflush(stdout);	exit(1);
		}
	}
    mysql_free_result(result);
}


/*****************************************************************************
* Function:	details_write
*
* Overview:	This function writes the received weather data to the 'details'
*			table. Note: the table must exist prior to a call to this function
*
* --
* -- Table structure for table dayfile
* --
*	 CREATE TABLE IF NOT EXISTS details (
*		datetime datetime NOT NULL,
*		year tinyint(2) unsigned NOT NULL,
*		month tinyint(2) unsigned NOT NULL,
*		day tinyint(2) unsigned NOT NULL,
*		time time NOT NULL,
*		temperature decimal(3,1) DEFAULT NULL,
*		windchill decimal(3,1) DEFAULT NULL,
*		apparent decimal(3,1) DEFAULT NULL,
*		dewpoint decimal(3,1) DEFAULT NULL,
*		humidity decimal(3,1) unsigned DEFAULT NULL,
*		windspeed decimal(4,1) unsigned DEFAULT NULL,
*		windgust decimal(4,1) unsigned DEFAULT NULL,
*		windbearing decimal(4,1) unsigned DEFAULT NULL,
*		pressure decimal(5,1) unsigned DEFAULT NULL,
*		rain smallint(5) unsigned DEFAULT NULL,
*		PRIMARY KEY (datetime),
*		KEY year (year), KEY month (month), KEY day (day)
*	 ) ENGINE=InnoDB DEFAULT CHARSET=latin1;
*
******************************************************************************/
void details_write(uint8_t wd)
{
    details_write_query(wd);
    if (mysql_query(conn, sql))
    {
        printf("Error %u: %s\n", mysql_errno(conn), mysql_error(conn));
        fflush(stdout); exit(1);
    }
}


