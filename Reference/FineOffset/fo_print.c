/*****************************************************************************
* File:		fo_print.c
*
* Overview:	This file implements all print_xxx() functions. These print functions
*			are mainly intended for debug/configuration/setup purposes, not for
*			normal operation
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fo.h"

extern RX		rx[recordsMAX][bytesMAX];
extern uint8_t	received[bytesMAX];
extern ARG		arg;

extern WDATA	wdata[20];
extern char		now[20];
extern uint8_t	record, byte;
extern FILE		*logfile;

const char *unit[3][4] = {{"C", "km/h", "hPa", "mm"}, {"F", "mph", "inHg", "in"},{"C", "kn", "inHg", "in"}};


/*****************************************************************************
* Function:	binary
*
* Overview:	This function converts a byte to it's binary representation
*
******************************************************************************/
void binary(char *buffer, uint8_t byte)
{
	uint8_t mask;
	buffer[0] = 0;
	for(mask=0x80; mask>0; mask>>=1)
		strcat(buffer, ((byte&mask)==mask) ? "1" : "0");
}


/*****************************************************************************
* Function:	print_status
*
* Overview:	This function prints the high/low status bytes for the
*           record(s) 0 to rec-1
*
******************************************************************************/
void print_status(uint8_t rec)
{
    for(record=0; record<=rec-1; record++)
    {
        printf("%s s  ", &now[11]);
        for(byte=0; byte<bytesMAX; byte++)
        {
            printf("    %02x%02x ", rx[record][byte].status & ~0xff00, rx[record][byte].status>>8);
        }
        printf("\n");
    }
}


/*****************************************************************************
* Function:	print_fifo
*
* Overview:	This function prints the received data bytes for the
*           record(s) 0 to rec-1
*
******************************************************************************/
void print_fifo(uint8_t rec)
{
	char buffer[9];

    for(record=0; record<=rec-1; record++)
    {
        printf("%s f  ", &now[11]);
        for(byte=0; byte<bytesMAX; byte++)
        {
        	binary(buffer, rx[record][byte].fifo);
            printf("%s ", buffer);
        }
        printf("\n");
    }
}


/*****************************************************************************
* Function:	print_corrected
*
* Overview:	This function prints the bit/shift corrected data and the
*			number of found biterrors
*
******************************************************************************/
void print_corrected(uint8_t biterrors)
{
	char buffer[9];

    printf("%s c  ", &now[11]);
    for(byte=0; byte<bytesMAX; byte++)
    {
		binary(buffer, received[byte]);
		printf("%s ", buffer);
    }
    printf("\t(%d bit errors)\n", biterrors);
}


/*****************************************************************************
* Function:	print_decoded
*
* Overview:	This function prints the decoded weather data, The rec parameter
*			indicates how many FIFO reads were necessary to get a valid CRC.
*			A value of 5 means: no valid CRC found was found in the original
*			transmissions, but the data could be corrected by the
*			correct_biterrors() function. Can also be used in normal operation
*			if you want 'realtime' data printed on the console.
*
******************************************************************************/
void print_decoded(uint8_t wd, uint8_t rec)
{
    printf("%s d%d ", &now[11], rec);
    printf("Temperature:%5.1f°%s ",		wdata[wd].temperature, unit[arg.units][0]);
    printf("-- Humidity: %3.1f%% ",		wdata[wd].humidity);
    printf("-- Windspeed: %4.1f%s ",	wdata[wd].windspeed, unit[arg.units][1]);
    printf("-- Windgust: %4.1f%s ",		wdata[wd].windgust, unit[arg.units][1]);
//    char *direction[] = {"N  ", "NNE", "NE ", "ENE", "E  ", "ESE", "SE ", "SSE", "S  ", "SSW", "SW ", "WSW", "W  ", "WNW", "NW ", "NNW"};
//    char *bearing = direction[wdata[wd].windbearing];
//    printf("-- Bearing: %s ", bearing);
    printf("-- Bearing (idx):  %2d ",	(int)wdata[wd].windbearing);
    printf("-- Pressure: %6.1f%s ",		wdata[wd].pressure, unit[arg.units][2]);
    printf("-- Rain (tips): %d\n",		(int)wdata[wd].rain);
}


/*****************************************************************************
* Function:	print_averages
*
* Overview:	This function prints the averaged weather data. Can also be useful
*			in normal operation to print the averaged data to the console.
*
******************************************************************************/
void print_averages()
{
    printf("%s a  ", &wdata[0].datetime[11]);
    printf("Temperature:%5.1f°%s ",		wdata[0].temperature, unit[arg.units][0]);
    printf("-- Humidity: %3.1f%% ",		wdata[0].humidity);
    printf("-- Windspeed: %4.1f%s ",	wdata[0].windspeed, unit[arg.units][1]);
    printf("-- Windgust: %4.1f%s ",		wdata[0].windgust, unit[arg.units][1]);
    printf("-- Bearing (deg): %3d ",	(int)(wdata[0].windbearing+0.5));
    printf("-- Pressure: %6.1f%s ",		wdata[0].pressure, unit[arg.units][2]);
    printf("-- Rain (%s): %.1f\n",		unit[arg.units][3], wdata[0].rain);
}
