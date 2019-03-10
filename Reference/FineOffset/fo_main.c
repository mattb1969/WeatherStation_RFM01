/*****************************************************************************
* File:		fo_main.c
*
* Overview:	This programm reads the data transmitted from a Fine Offset type
*			wireless weather station (Maplin, Nevada, Watson, Ambient,  Clas
*			Ohlson, National Geographic, ...) that use FSK transmission. For
*			stations that use the OOK transmission protocol, please use the
*			original program written by Kevin Sangeelee as documented on his
*			blog (http://www.susa.net/wordpress/2012/08/raspberry-pi-
*			reading-wh1081-weather-sensors-using-an-rfm01-and-rfm12b/)
*
* Note:		This program needs to be run with root privileges
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// C library for Broadcom BCM2835 (http://www.open.com.au/mikem/bcm2835)
#include <bcm2835.h>
// #include "fo_bcm2835.h"
// #include "bcm2835.h"

#include "fo.h"
#include "fo_rfm01.h"

void	spi_init();
void	rfm01_set_register(uint16_t cmd);
void	rfm01_init();
void	receive_records();
void	get_args(int argc, char *argv[]);
void	pgm_init();
void	pgm_end(int sig);


/******************************************************************************
* Global data
*******************************************************************************/
RX			rx[recordsMAX][bytesMAX];
ARG 		arg;
uint8_t 	received[bytesMAX];
WDATA		wdata[20];
uint8_t		wd;
MINMAX		wmm[2];
FILE		*logfile;
char		now[20], today[20];


/*****************************************************************************
* Function:	spi_init
*
* Overview:	This function uses the bcm2835 library to set up the SPI
*
******************************************************************************/
void spi_init()
{
	printf("Initialising BCM 2835\n");
	if (!bcm2835_init())
	{
		printf("Error initializing bmc2835 library\n");
		fflush(stdout); exit(1);
	}
	bcm2835_spi_begin();
	printf(".");
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
	printf(".");
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	printf(".");
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);
	printf(".");
	bcm2835_spi_chipSelect(RFM01_CE);
	printf(".");
	bcm2835_spi_setChipSelectPolarity(RFM01_CE, LOW);
	printf(".");
	bcm2835_gpio_fsel(RFM01_IRQ, BCM2835_GPIO_FSEL_INPT);
	printf(".");
	bcm2835_gpio_set_pud(RFM01_IRQ, BCM2835_GPIO_PUD_UP);
	printf(".");
	// As we use FIFO, the DATA line requires pull-up
	bcm2835_gpio_fsel(RFM01_DATA, BCM2835_GPIO_FSEL_INPT);
	printf(".");
	bcm2835_gpio_set_pud(RFM01_DATA, BCM2835_GPIO_PUD_UP);
	printf(". - last SPI init\n");
}


/*****************************************************************************
* Function:	rfm01_set_register
*
* Overview:	This function sends configuration commands to the RFM01 module
*
******************************************************************************/
void rfm01_set_register(uint16_t cmd)
{
    char buffer[2];
    buffer[0] = cmd >> 8;
    buffer[1] = cmd;
    bcm2835_spi_transfern(buffer,2);
}


/*****************************************************************************
* Function:	rfm01_init
*
* Overview:	This function initializes the RFM01 module. This sequence is based
*			on the power up sequence of a Clas Ohlson WH1080 (probed by cii, see
*			http://www.raspberrypi.org/phpBB3/viewtopic.php?f=37&t=14777&start=57)
*			and the HopeRF document "RFM01 Universal ISM Band FSK Receiver"
*			(http://www.hoperf.com/upload/rf/RFM01.pdf)
* Note:		This configuration sequence is for a 915MHz National Geograpgic
*			station. If your station uses the 868MHz band, you have to change
*			the frequency band to 'BAND_868' and the frequency setting to '0x067c'
*
******************************************************************************/
void rfm01_init()
{
    printf("Initialising RFM 01\n");

    rfm01_set_register(CMD_CONFIG |		// -------- Configuration Setting Command --------
		BAND_433 |                  	// selects the 915 MHz frequency band
		// BAND_915 |                  	// selects the 915 MHz frequency band
		//BAND_868 |                  	// selects the 868 MHz frequency band
		LOWBATT_EN |                	// enable the low battery detector
		CRYSTAL_EN |                	// the crystal is active during sleep mode
		LOAD_CAP_12C5 |             	// 12.5pF crystal load capacitance
		BW_134);                    	// 134kHz baseband bandwidth

    rfm01_set_register(CMD_FREQ |		// -------- Frequency Setting Command --------
		0x0620);				// 433,92Mhz
		// 0x07d0);                    	// 915.0 MHz --> F = ((915/(10*3))-30)*4000 = 2000 = 0x07d0
		// 0x067c);						// 868.3 MHz --> F = ((868.3/(10*2))-43)*4000 = 1660 = 0x067c


    rfm01_set_register(CMD_WAKEUP |		// -------- Wake-Up Timer Command --------
		1<<8 |                      	// R = 1
		0x05);                      	// M = 5
										// T_wake-up = (M * 2^R) = (2 * 5) = 10 ms

    rfm01_set_register(CMD_LOWDUTY |	// -------- Low Duty-Cycle Command --------
		0x0e);                      	// (this is the default setting)

    rfm01_set_register(CMD_AFC |		// -------- AFC Command --------
		AFC_VDI |						// drop the f_offset value when the VDI signal is low
		AFC_RL_15 |						// limits the value of the frequency offset register to +15/-16
		AFC_STROBE |					// the actual latest calculated frequency error is stored into the output registers of the AFC block
		AFC_FINE |						// switches the circuit to high accuracy (fine) mode
		AFC_OUT_ON |					// enables the output (frequency offset) register
		AFC_ON);						// enables the calculation of the offset frequency by the AFC circuit

    rfm01_set_register(CMD_DFILTER |	// -------- Data Filter Command --------
		CR_LOCK_FAST |              	// clock recovery lock control, fast mode, fast attack and fast release
		FILTER_DIGITAL |            	// select the digital data filter
		DQD_4);                     	// DQD threshold parameter

    rfm01_set_register(CMD_DRATE |		// -------- Data Rate Command --------
		0<<7 |                      	// cs = 0
		0x13);                      	// R = 18 = 0x12
										// BR = 10000000 / 29 / (R + 1) / (1 + cs*7) = 18.15kbps

    rfm01_set_register(CMD_LOWBATT |	// -------- Low Battery Detector and Microcontroller Clock Divider Command --------
		2<<5 |                      	// d = 2, 1.66MHz Clock Output Frequency
		0x00);                      	// t = 0, determines the threshold voltage V_lb

    rfm01_set_register(CMD_RCON |		// -------- Receiver Setting Command --------
		VDI_CR_LOCK |              		// VDI (valid data indicator) signal: clock recovery lock
		LNA_0 |                     	// LNA gain set to 0dB
		RSSI_103);                  	// threshold of the RSSI detector set to 103dB

    rfm01_set_register(CMD_FIFO |		// -------- Output and FIFO Mode Command --------
		8<<4 |                      	// f = 8, FIFO generates IT when number of the received data bits reaches this level
		1<<2 |                      	// s = 1, set the input of the FIFO fill start condition to sync word
		0<<1 |                      	// Disables FIFO fill after synchron word reception
		0);                         	// Disables the 16bit deep FIFO mode

    rfm01_set_register(CMD_FIFO |		// -------- Output and FIFO Mode Command --------
		8<<4 |                      	// f = 8, FIFO generates IT when number of the received data bits reaches this level
		1<<2 |                      	// s = 1, set the input of the FIFO fill start condition to sync word
		1<<1 |                      	// Enables FIFO fill after synchron word reception
		1);                         	// Ensables the 16bit deep FIFO mode

    rfm01_set_register(CMD_RCON |		// -------- Receiver Setting Command ---------
		VDI_CR_LOCK |               	// VDI (valid data indicator) signal: clock recovery lock
		LNA_0 |                     	// LNA gain set to 0dB
		RSSI_103 |                  	// threshold of the RSSI detector set to 103dB
		1);                         	// enables the whole receiver chain
}


/*****************************************************************************
* Function:	receive_records
*
* Overview:	This function monitors the RFM01 interrupt signal. When an interrupt
*			is received, the FIFO register is read out and the received data are
*			processed. The transmitter sends new data every 48 seconds, so after
*			processing the data this function goes into sleep mode for 40 seconds
*			(to be on the safe side) before waiting for the next interrupt.
*
******************************************************************************/
void receive_records()
{
    int8_t record, byte;
    extern uint8_t received[bytesMAX];

    const uint16_t clearfifo  = CMD_FIFO | 8<<4 | 1<<2 | 0<<1 | 0;
    const uint16_t enablefifo = CMD_FIFO | 8<<4 | 1<<2 | 1<<1 | 1;

    printf("Receive Records\n");
    while(1)
    {
        scheduler(REALTIME);													// set the scheduler policy to realtime
	printf(".1\n");
        for(record=0; record<recordsMAX; record++)
        {
	    printf(".2\n");
            rfm01_set_register(clearfifo);										// disable (clear) FIFO
            printf(".3\n");
	    rfm01_set_register(enablefifo);										// re-enable FIFO
            printf(".4\n");

	    for(byte=0; byte<bytesMAX; byte++)									// we only read the first byteMAX bytes
            {
                rx[record][byte].status = 0;									// set command to 0 (read status, FIFO)
		printf(".5\n");
                while(bcm2835_gpio_lev(RFM01_IRQ));								// wait for interrupt
		printf(".6\n");

		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);   					// set SPI speed to 2MHz for FIFO read
		printf(".7\n");
                bcm2835_spi_transfern(rx[record][byte].buffer, 3);						// read FIFO and store in rx
		printf(".8\n");
		bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32);    					// reset SPI speed to 8MHz
		printf(".9\n");

                if(rx[record][0].fifo==0) byte--;								// ignore occasional leading 0
			else if((rx[record][0].fifo>>4)!=0x0a)							// high nibble of first byte must be 1010b
			{
				record--;											   		// ignore if record is not weather data
				break;														// continue record loop
			}
			else received[byte] = rx[record][byte].fifo;					// store received data in received array
            }
			if(byte==0) continue;												// this is necessary because of the previous break!
			if(rx[record][9].fifo==crc8(received, 9)) break;					// if the CRC is OK, don't read any further
        }
	printf(".A\n");
        process_data(record);													// process the received data
	printf(".B\n");
        fflush(stdout);															//
		scheduler(STANDARD);													// re-set the scheduler policy to standard
	printf(".delay\n");
        bcm2835_delay(40000);													// idle loop until next possible transmission
   }
}


/*****************************************************************************
* Function:	get_args
*
* Overview:	This function processes possible command line parameters
*
******************************************************************************/
void get_args(int argc, char *argv[])
{
    int opt;

	// set default values
    arg.logging = TRUE;
    arg.altitude = 0;
    arg.wspeedm = 1;
    arg.units = METRIC;
    arg.verbose = FALSE;
    arg.printopts[0] = 0;

	// process all passed options
    while ((opt = getopt(argc, argv, "l:a:w:u:p:hv")) != -1)
	{
        switch (opt)
        {
			case 'l':
				arg.logging = atoi(optarg);
				break;
#ifdef BMP085
			case 'a':
				arg.altitude = atof(optarg);
				break;
#endif
			case 'w':
				arg.wspeedm = atof(optarg);
				break;
			case 'u':
				if(strchr(optarg, 'i')) arg.units = IMPERIAL;
				else if(strchr(optarg, 'n')) arg.units = NAUTICAL;
				break;
			case 'p':
				strcpy(arg.printopts, optarg);
				if(strchr(optarg, 's')) arg.print_s = TRUE;
				if(strchr(optarg, 'f')) arg.print_f = TRUE;
				if(strchr(optarg, 'c')) arg.print_c = TRUE;
				if(strchr(optarg, 'd')) arg.print_d = TRUE;
				if(strchr(optarg, 'a')) arg.print_a = TRUE;
				break;
			case 'v':
				arg.verbose = TRUE;
				break;
			case 'h':
			default:
				printf("Usage: fopi [OPTIONS]\n");
				printf("  -l   enable/disable logging\n");
				printf("       0 = logging disabled\n");
				printf("       1 = logging enabled (default)\n");
#ifdef BMP085
				printf("  -a   altitude in m (for BMP085)\n");
#endif
				printf("  -w   wind speed multiplier, default = 0\n");
				printf("       see: http://wiki.sandaysoft.com/a/Wind_measurement\n");
				printf("  -u   units, default = metric\n");
				printf("       m = use metric system\n");
				printf("       i = use imperial system\n");
				printf("       n = use nautical system\n");
				printf("  -p   print options, default = d\n");
				printf("       s = print Status data\n");
				printf("       f = print FIFO data\n");
				printf("       c = print corrected FIFO data\n");
				printf("       d = print decoded data\n");
				printf("       a = print averaged data\n");
				printf("  -v   verbose, display messages\n");
				printf("  -h   display this message\n");
				fflush(stdout);
				exit(1);
        }
    }
}


/*****************************************************************************
* Function:	pgm_init
*
* Overview:	This function sets start values for (global) variables, creates a
*			connection to a datafile/database, optionally prints some messages
*			and creates an intercept for the Ctrl-C keyboard sequence to exit
*			the receive_records() loop and to securely terminate the program
*
******************************************************************************/
void pgm_init()
{
	time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);
    strftime(today, 20, "%Y-%m-%d %H:%M:00", tm_info);

    char msg1[] = "\n%s ----- Program start -----\n";
	char msg2[] = "%s\tPrint Options: %s\n";
#ifdef DATAFILE
	char msg3[] = "%s\tWrite data to 'fopi.data' and 'fopi.dayfile'\n";
#endif
#ifdef MYSQL5
	char msg3[] = "%s\tUse MySQL database\n";
#endif
#ifdef SQLITE3
	char msg3[] = "%s\tUse Sqlite3 database\n";
#endif

#ifdef BMP085
	char msg4[] = "%s\tUse BMP085, Altitude: %dm\n";
#else
	char msg4[] = "%s\tNo BMP%d85\n"; // well that's tricky
#endif
	char msg5[] = "%s\tWind Speed Multiplier: %.1f\n";
	char msg6[] = "%s\tUnits: %d\n\n";

    if(arg.logging)
    {
    	logfile = fopen("fopi.log","a+");
		fprintf(logfile, msg1, today);
		fprintf(logfile, msg2, today, arg.printopts);
        fprintf(logfile, msg3, today);
		fprintf(logfile, msg4, today, (int)arg.altitude);
		fprintf(logfile, msg5, today, arg.wspeedm);
		// fprintf(logfile, msg6, today, arg.logging);
		fprintf(logfile, msg6, today, arg.units); // changed to units MB
    }

    if(arg.verbose)
    {
		printf(msg1, today);
		printf(msg2, today, arg.printopts);
		printf(msg3, today);
		printf(msg4, today, (int)arg.altitude);
		printf(msg5, today, arg.wspeedm);
		// printf(msg6, today, arg.logging);
		printf(msg6, today, arg.units); //changed to units MB
    }

	strcpy(wdata[0].datetime, today);

	// Create connection to datafile / database
	create_conn();

	printf("Created Connection\n");

	// wd is the index into the wdata array
	// wdata[0] holds the averages, actual data start at 1
    wd = 1;

	// intercept the Ctrl-C signal to exit the receive_records() loop
	signal(SIGINT, pgm_end);
    printf("Press Ctrl-C to end this program\n");
}


/*****************************************************************************
* Function:	pgm_end
*
* Overview:	This function is called by pressing Ctrl-C on your keyboard. This
*			interrupt handler was set up in pgm_ini() and is reset here to its
*			default action. All SPI pins on the RasPi are reset to default, the
*			min/max values are written and the datafile/database is closed.
*			Finally the program exits here, controll does not return to main()!
*
******************************************************************************/
void pgm_end(int sig)
{
    // reset signal() to default
	signal(SIGINT, SIG_DFL);

    bcm2835_spi_end();
	dayfile_write();
	close_conn();
	exit(0);
}


/*****************************************************************************
* Function:	main
*
* Overview:	This function calls the various _init() functions and than call the
*			receive_records() loop. This loop can only be treminated by pressing
*			Ctrl-C.
*
******************************************************************************/
int main(int argc, char *argv[])
{
    get_args(argc, argv);
    pgm_init();
    printf("about to SPI Init\n");
    spi_init();
    rfm01_init();

    // this is the main program loop
    receive_records();

	// unreachable, just to prevent compiler warnings ...
	return 0;
}
