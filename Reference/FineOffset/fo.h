/*****************************************************************************
* File:		fo.h
*
* Overview: defines, structs, function declarations, ... for the fopi program
*
******************************************************************************/
#include <stdint.h>

#define DATAFILE		// no database
//#define MYSQL5			// MySQL database
//#define SQLITE3			// Sqlite3 database
// #define BMP085			// comment out this line if you don't use a BMP085

#define recordsMAX      5   // loop counter for bytesMAX bytes to read
#define bytesMAX        10  // bytes to read

/* commented out as for V1 board - MB
#define RFM01_IRQ       RPI_GPIO_P1_15          // SPI IRQ GPIO pin.
#define RFM01_CE        BCM2835_SPI_CS0         // SPI chip select
#define RFM01_DATA 	RPI_GPIO_P1_13   		// Data
*/
#define RFM01_IRQ       RPI_V2_GPIO_P1_15          // SPI IRQ GPIO pin.
#define RFM01_CE        BCM2835_SPI_CS0         // SPI chip select
#define RFM01_DATA 	RPI_V2_GPIO_P1_13   		// Data

#define TRUE	        1
#define FALSE	        0

#define REALTIME        1
#define STANDARD        0

#define MIN				0
#define MAX				1

#define INSERT			0
#define UPDATE			1

#define METRIC			0
#define IMPERIAL		1
#define NAUTICAL		2

void     	close_conn();
void     	create_conn();

uint8_t 	crc8(uint8_t *addr, uint8_t len);

void 		dayfile_read();
void     	dayfile_read_query();
void     	dayfile_write();
void     	dayfile_write_query(uint8_t mode, uint8_t type);

void 		details_write(uint8_t wd);
void     	details_write_query(uint8_t wd);

void     	print_averages();
void     	print_corrected(uint8_t biterrors);
void     	print_decoded(uint8_t wd, uint8_t rec);
void     	print_fifo(uint8_t rec);
void     	print_status(uint8_t rec);

void     	process_data(uint8_t rec);

void 		reset_min_max();

float 		read_bmp085(float altitude);

void     	scheduler(uint8_t mode);


/*****************************************************************************
* The RX union is used in the FIFO tranfer from the RFM01 module. You can read
* the FIFO one byte at a time by sending a status command (0x00) to the RFM01.
* After the execution, the first two bytes of the buffer (rx.status) contains
* status information, the 3rd byte (rx.fifo) contains the FIFO data.
* See: "RFM01 Universal ISM Band FSK Receiver" pg. 17, Status Read Command
* (http://www.hoperf.com/upload/rf/RFM01.pdf)
******************************************************************************/
typedef union
{
    char buffer[3];
    struct
    {
		uint16_t status;
        uint8_t  fifo;
    };
} RX;


/*****************************************************************************
* The WDATA struct holds the received weather data. The weather statation send
* new data every 48 sec. To hold 10 minutes worth of data, we need an wdata[15]
* array. wdata[1] to wdata[wd] hold the received data, wdata[0] holds the 10
* minute averages
******************************************************************************/
typedef struct
{
    char   	datetime[20];
    float   temperature;
    float   windchill;
    float   apparent;
    float   dewpoint;
    float 	humidity;
    float   windspeed;
    float   windgust;
    float   windbearing;
    float   pressure;
    float   rain;
    uint8_t status;
} WDATA;


/*****************************************************************************
* The MINMAX struct holds the min/max values for each day
******************************************************************************/
typedef struct
{
	char	 date[11];
    float    temperature;
    char   	 temperature_t[9];
    float    windchill;
    char   	 windchill_t[9];
    float    apparent;
    char   	 apparent_t[9];
    float    dewpoint;
    char   	 dewpoint_t[9];
    float 	 humidity;
    char   	 humidity_t[9];
    float    windspeed;
    char   	 windspeed_t[9];
    float    windgust;
    char   	 windgust_t[9];
    float    pressure;
    char   	 pressure_t[9];
    uint16_t rain;	// can hold 0xfff bucket tips, enough for 20m of rainfall
} MINMAX;


/*****************************************************************************
* The ARG struct stores the command line arguments at program start
******************************************************************************/
typedef struct
{
    uint8_t logging;
    float   altitude;
    uint8_t units;
    char    printopts[6];
    uint8_t print_s;
    uint8_t print_f;
    uint8_t print_c;
    uint8_t print_d;
    uint8_t print_a;
    float   wspeedm;
    uint8_t verbose;
} ARG;

