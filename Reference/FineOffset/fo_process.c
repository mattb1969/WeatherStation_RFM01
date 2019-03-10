/*****************************************************************************
* File:		fo_process.c
*
* Overview:	This file implements functions to process the received data
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "fo.h"

void			check_min_max();
void			compute_average_bearing();
void			compute_averages();
uint8_t			correct_biterrors();
uint8_t			decode_data();

extern RX		rx[recordsMAX][bytesMAX];
extern uint8_t	received[bytesMAX];
extern ARG		arg;
extern WDATA	wdata[20];
extern uint8_t  wd;
extern MINMAX	wmm[2];
extern FILE		*logfile;
extern char		now[20], today[20];
uint8_t 		record, byte, biterrors;

/*****************************************************************************
* Function:	process_data
*
* Overview:	This is the main function to process the received weather data
*
******************************************************************************/
void process_data(uint8_t rec)
{
	uint8_t crc_ok = TRUE;
	time_t  timer;
    struct  tm* tm_info;

    char errmsg1[] = "%s CRC OK but data invalid, record ignored\n";
    char errmsg2[] = "%s CRC error, record ignored\n";

	time(&timer);
    tm_info = localtime(&timer);
    strftime(now, 20, "%Y-%m-%d %H:%M:%S", tm_info);

	// the print options are mainly for debugging
	if(arg.print_s) print_status(rec);
	if(arg.print_f) print_fifo(rec);

	if(rec==recordsMAX)
	{
		// the calling function (receive_records) could not find a valid CRC, so
		// we try to correct the bit/shift errors in the received data stream
		crc_ok = correct_biterrors();
		if(arg.print_c) print_corrected(biterrors);
    }

	if(crc_ok)
	{
		// decode the received data and check for plausability
		if(decode_data())
		{
			// every 10 minutes calculate averages and store them in the database
			if((memcmp(&now[14], &wdata[0].datetime[14], 1)!=0) && (wd>1))
			{
				compute_averages();
				if(arg.print_a) print_averages();
				// do we have a day change?
				if(memcmp(&now[8], &today[8], 2)!=0)
				{
					dayfile_write();
					strcpy(today, now);
					reset_min_max();
				}
			}

			// Check for possible min/max values
			check_min_max();

			if(arg.print_d || arg.verbose) print_decoded(wd, rec);
			wd++;
		}
		else
		{
			if(arg.logging) fprintf(logfile, errmsg1, now);
			if(arg.verbose) printf(errmsg1, now);
		}
	}
	else
	{
	    if(arg.logging) fprintf(logfile, errmsg2, now);
	    if(arg.verbose) printf(errmsg2, now);
	}
}


/*****************************************************************************
* Function:	correct_biterrors
*
* Overview:	This function ties tp correct the bit errors by comparing the five
*			received data streams and applying a 'majority vote'
*
******************************************************************************/
uint8_t correct_biterrors()
{
    uint8_t bit, bc, bc0[8], bc1[8], div = recordsMAX / 2;

	biterrors = 0;

    for(byte=0; byte<bytesMAX; byte++)
    {
        for(bc=0; bc<8; bc++)
        {
            bc0[bc] = 0;
            bc1[bc] = 0;
        }

        for(record=0; record<recordsMAX; record++)
        {
			for(bc=0; bc<8; bc++)
			{
				bit = (rx[record][byte].fifo >> bc) & ~0xfe;
				if(bit) bc1[bc] += 1;
				else bc0[bc] += 1;
			}
        }

        received[byte] = 0;
        for(bc=0; bc<8; bc++)
        {
            if(bc0[bc]>div)
            {
                received[byte] |= (0<<bc);  // not realy necessary
                biterrors += bc1[bc];
            }
            else
            {
                received[byte] |= (1<<bc);
                biterrors += bc0[bc];
            }
        }
    }

    return (received[9]==crc8(received, 9)) ? TRUE : FALSE;
}


/*****************************************************************************
* Function:	decode_data
*
* Overview:	This function decodes the weather data
*
******************************************************************************/
uint8_t decode_data()
{
	strcpy(wdata[wd].datetime, now);

    // temperature in °C
    uint8_t  sign = (received[1] >> 3) & 1;
    uint16_t temperature = ((received[1] & ~(0xff << 1)) << 8) | received[2];
    if(sign) wdata[wd].temperature = ((~temperature)+sign) / 10.0;
    else wdata[wd].temperature = temperature / 10.0;

    // humidity in %
    wdata[wd].humidity = received[3] & ~0x80;

    // wind speed im km/h, adjusted by wind speed multiplier
    wdata[wd].windspeed = received[4] * 1.224 * arg.wspeedm;

    // wind gust in km/h, adjusted by wind speed multiplier
    wdata[wd].windgust = received[5] * 1.224 * arg.wspeedm;

    //rainbucket tip counter, every tip is 0.3mm
    wdata[wd].rain = ((received[6] & ~0xf0) << 8) | received[7];

    // status bits, high nibble of received[8]
    wdata[wd].status = received[8] >> 4;
	if(wdata[wd].status)
	{
		if(arg.logging) fprintf(logfile, "%s Statusbit(s) set: %d\n", now, wdata[wd].status);
		if(arg.verbose) printf("%s Statusbit(s) set: %d\n", now, wdata[wd].status);
	}

    // wind bearing, low nibble of received[8]
    wdata[wd].windbearing = received[8] & ~0xf0;

#ifdef BMP085
    // sea level pressure
    wdata[wd].pressure = read_bmp085(arg.altitude);
#else
    wdata[wd].pressure = 0.0;
#endif

	// Since the CRC is only 8 bit, the chance of invalid data is 1 in 256, so we still
	// have to test for possible transmisson errors. If an error is detected, return FALSE
    if(abs(wdata[wd].temperature)>50.0) return FALSE;
    if(wdata[wd].humidity>99.9) return FALSE;
    if(wdata[wd].windspeed>100.0) return FALSE;
    if(wdata[wd].windspeed>wdata[wd].windgust) return FALSE;
    if(((wdata[wd].rain-wdata[0].rain)>20.0) && (wdata[0].rain>0.0)) return FALSE;

	// compute additional (derived) temperature data
	// wind chill (formula from http://en.wikipedia.org/wiki/wind_chill)
	if((wdata[wd].windgust<=4.8) || (wdata[wd].temperature>10.0)) wdata[wd].windchill = wdata[wd].temperature;
	else
	{
		float wchill = (13.12 + (wdata[wd].temperature * 0.6215) + (((0.3965 * wdata[wd].temperature) - 11.37) * pow(wdata[wd].windgust, 0.16)));
		wdata[wd].windchill = (wchill < wdata[wd].temperature) ? wchill : wdata[wd].temperature;
	}

	// apparent (formula from http://www.bom.gov.au/info/thermal_stress)
	float vap = (wdata[wd].humidity / 100.0) * 6.105 * exp(17.27 * wdata[wd].temperature / (237.7 + wdata[wd].temperature));
	wdata[wd].apparent = wdata[wd].temperature + (0.33 * vap) - (0.70 * wdata[wd].windspeed) - 4.00;

	// dewpoint (formula from http://en.wikipedia.org/wiki/dew_point)
	const float a = 17.27, b = 237.7;
	float gamma = ((a * wdata[wd].temperature) / (b + wdata[wd].temperature)) + log(wdata[wd].humidity / 100.0);
	wdata[wd].dewpoint = (b * gamma) / (a - gamma);

	// If not metric, change units to imperial or nautical
	if(arg.units==1)
	{
		wdata[wd].temperature = (wdata[wd].temperature * 1.8f) + 32.0f;
		wdata[wd].windchill   = (wdata[wd].windchill * 1.8f) + 32.0f;
		wdata[wd].dewpoint    = (wdata[wd].dewpoint * 1.8f) + 32.0f;
		wdata[wd].apparent    = (wdata[wd].apparent * 1.8f) + 32.0f;
		wdata[wd].windspeed   *= 0.621371;
		wdata[wd].windgust    *= 0.621371;
		wdata[wd].pressure    *= 0.02953;
	}
	else if(arg.units==2)
	{
		wdata[wd].windspeed   *= 0.539957;
		wdata[wd].windgust    *= 0.539957;
		wdata[wd].pressure    *= 0.02953;
	}

	return TRUE;
}



/*****************************************************************************
* Function:	check_min_max
*
* Overview:	This function adjusts the min/max data if necessary
*
******************************************************************************/
void check_min_max()
{
	if(wmm[MIN].temperature>wdata[wd].temperature)
	{
		wmm[MIN].temperature = wdata[wd].temperature;
		strncpy(wmm[MIN].temperature_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].windchill>wdata[wd].windchill)
	{
		wmm[MIN].windchill = wdata[wd].windchill;
		strncpy(wmm[MIN].windchill_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].apparent>wdata[wd].apparent)
	{
		wmm[MIN].apparent = wdata[wd].apparent;
		strncpy(wmm[MIN].apparent_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].dewpoint>wdata[wd].dewpoint)
	{
		wmm[MIN].dewpoint = wdata[wd].dewpoint;
		strncpy(wmm[MIN].dewpoint_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].humidity>wdata[wd].humidity)
	{
		wmm[MIN].humidity = wdata[wd].humidity;
		strncpy(wmm[MIN].humidity_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].windspeed>wdata[wd].windspeed)
	{
		wmm[MIN].windspeed = wdata[wd].windspeed;
		strncpy(wmm[MIN].windspeed_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].windgust>wdata[wd].windgust)
	{
		wmm[MIN].windgust = wdata[wd].windgust;
		strncpy(wmm[MIN].windgust_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MIN].pressure>wdata[wd].pressure)
	{
		wmm[MIN].pressure = wdata[wd].pressure;
		strncpy(wmm[MIN].pressure_t, &wdata[wd].datetime[11], 8);
	}

	if(wmm[MAX].temperature<wdata[wd].temperature)
	{
		wmm[MAX].temperature = wdata[wd].temperature;
		strncpy(wmm[MAX].temperature_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].windchill<wdata[wd].windchill)
	{
		wmm[MAX].windchill = wdata[wd].windchill;
		strncpy(wmm[MAX].windchill_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].apparent<wdata[wd].apparent)
	{
		wmm[MAX].apparent = wdata[wd].apparent;
		strncpy(wmm[MAX].apparent_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].dewpoint<wdata[wd].dewpoint)
	{
		wmm[MAX].dewpoint = wdata[wd].dewpoint;
		strncpy(wmm[MAX].dewpoint_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].humidity<wdata[wd].humidity)
	{
		wmm[MAX].humidity = wdata[wd].humidity;
		strncpy(wmm[MAX].humidity_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].windspeed<wdata[wd].windspeed)
	{
		wmm[MAX].windspeed = wdata[wd].windspeed;
		strncpy(wmm[MAX].windspeed_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].windgust<wdata[wd].windgust)
	{
		wmm[MAX].windgust = wdata[wd].windgust;
		strncpy(wmm[MAX].windgust_t, &wdata[wd].datetime[11], 8);
	}
	if(wmm[MAX].pressure<wdata[wd].pressure)
	{
		wmm[MAX].pressure = wdata[wd].pressure;
		strncpy(wmm[MAX].pressure_t, &wdata[wd].datetime[11], 8);
	}

    if(wmm[MIN].rain==0) wmm[MIN].rain = wdata[wd].rain;
    wmm[MAX].rain = wdata[wd].rain;
}


/*****************************************************************************
* Function:	compute_averages
*
* Overview:	This function averages the received data and stores them in the
*			datafile / database. The calculated values are interpreted as
*			the average over the previous 10 minutes
*
******************************************************************************/
void compute_averages()
{
    uint8_t i;

    strcpy(wdata[0].datetime, now);
    memcpy(&wdata[0].datetime[15], "0:00\0", 5);

    wdata[0].temperature = wdata[1].temperature;
    wdata[0].windchill   = wdata[1].windchill;
    wdata[0].dewpoint    = wdata[1].dewpoint;
    wdata[0].apparent    = wdata[1].apparent;
    wdata[0].humidity    = wdata[1].humidity;
    wdata[0].windspeed   = wdata[1].windspeed;
    wdata[0].windgust    = wdata[1].windgust;
    wdata[0].pressure    = wdata[1].pressure;
    wdata[0].status		 = wdata[1].status;

    for(i=2; i<wd; i++)
    {
        wdata[0].temperature += wdata[i].temperature;
        wdata[0].windchill   += wdata[i].windchill;
        wdata[0].dewpoint    += wdata[i].dewpoint;
        wdata[0].apparent    += wdata[i].apparent;
        wdata[0].humidity    += wdata[i].humidity;
        wdata[0].windspeed   += wdata[i].windspeed;
        wdata[0].windgust    += wdata[i].windgust;
        wdata[0].pressure    += wdata[i].pressure;
		wdata[0].status		 |= wdata[i].status;
    }

    wdata[0].temperature /= (wd-1);
    wdata[0].windchill   /= (wd-1);
    wdata[0].dewpoint    /= (wd-1);
    wdata[0].apparent    /= (wd-1);
    wdata[0].humidity    /= (wd-1);
    wdata[0].windspeed   /= (wd-1);
    wdata[0].windgust    /= (wd-1);
    wdata[0].pressure    /= (wd-1);

	// calculate the rain fallen over the last 10 minutes
    wdata[0].rain = (wdata[wd-1].rain - wdata[1].rain) * 0.3;
    // adjust if not metric
	if(arg.units>0) wdata[0].rain *= 0.0393701;

    compute_average_bearing();

    // store data in file or database
    details_write(0);

    // newest data (not yest used to compute averages) become wdata[1]
    wdata[1].temperature = wdata[wd].temperature;
    wdata[1].windchill   = wdata[wd].windchill;
    wdata[1].dewpoint    = wdata[wd].dewpoint;
    wdata[1].apparent    = wdata[wd].apparent;
    wdata[1].humidity    = wdata[wd].humidity;
    wdata[1].windspeed   = wdata[wd].windspeed;
    wdata[1].windgust    = wdata[wd].windgust;
    wdata[1].windbearing = wdata[wd].windbearing;
    wdata[1].pressure    = wdata[wd].pressure;
    wdata[1].rain        = wdata[wd].rain;

    wd = 1;
}


/*****************************************************************************
* Function:	compute_average_bearing
*
* Overview:	This function computes the average wind bearing
*
* Code:
*
*	u-comp ws = -1 * (ws*sin(wd*PI/180))
*	v-comp ws = -1 * (ws*cos(wd*PI/180))
*
*	where PI = 3.1415926535897
*	assuming six 10-minute values in an hour
*	average the six u-comp WS and six v-comp WS to get an average u-comp and v-comp for the hour
*
*	if average u-comp > 0
*		hourly average WD = (90-180/PI*atan(average v-comp/average u-comp)+180)
*	elseif average u-comp < 0
*	    hourly average WD = (90-180/PI*atan(average v-comp/average u-comp))
*	elseif average u-comp = 0
*	    if average v-comp < 0
*	        hourly average WD = 360
*		elseif average v-comp > 0
*		    hourly average WD = 180
*		else
*		    hourly average WD = 0
*		end
*	end
*
* Example:
*
*	Six WS measurements: [2.3, 2.7, 4.5, 5.2, 10.3, 8.1]
*	Six WD measurements: [220, 174, 43, 356, 99, 67]
*
* Results:
*
*	Average u-comp = -3.18989225
*	Average v-comp = -0.930826755
*	Hourly Average WD = 73.73 degrees
*	Hourly Average WS = 5.5167 m/s
*
******************************************************************************/
void compute_average_bearing()
{
	const float PI = 3.1415926535897;
	uint8_t		i;
	float		u_comp = 0.0, v_comp = 0.0, wdir;

	for(i=1; i<wd; i++)
	{
		wdir = wdata[i].windbearing * 22.5;
		u_comp += -1 * wdata[i].windgust * sin(wdir * PI / 180);
		v_comp += -1 * wdata[i].windgust * cos(wdir * PI / 180);
	}
	u_comp /= (wd-1);
	v_comp /= (wd-1);

	if(u_comp>0.0) wdata[0].windbearing = 90.0 - 180.0 / PI * atan(v_comp / u_comp) + 180.0;
	else if(u_comp<0.0) wdata[0].windbearing = 90.0 - 180.0 / PI * atan(v_comp / u_comp);
	else
	{
		if(v_comp<0.0) wdata[0].windbearing = 360.0;
		if(v_comp>0.0) wdata[0].windbearing = 180.0;
		else wdata[0].windbearing = 0.0;
	}

//    uint8_t i;
//
//    // I know this is wrong!!! But just for now ...
//    float avg = 0;
//    for(i=1; i<wd; i++)
//    {
//        avg += wdata[i].windbearing;
//    }
//    wdata[0].windbearing = avg / (wd-1) * 22.5;
}
