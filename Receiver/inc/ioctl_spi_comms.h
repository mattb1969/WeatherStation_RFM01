 /*
 *  \brief Function declarations to initilalize platform SPI interface and to communicate
 *	with ST25R3911XX using SPI interface.
 *  
 */
#ifndef PLATFORMSPI_H
#define PLATFORMSPI_H

/* SPI COMMS SETUP
 * Shifted in on Rising Edge of the Clock
 * Shifted out on falling edge of clock signal
 *	either mode 0 or mode 3, using at mode 0
 * nSEL is active LOW
 * number of bits is integer of 8
 * MSB first
 * max frequency is 50nS - 20Mhz
 * 
 */

/*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <stdint.h>
//#include "st_errno.h"

/*
 ******************************************************************************
 * GLOBAL TYPES
 ******************************************************************************
 */
/* Enumeration for SPI communication status */
typedef enum
{
	HAL_OK		= 0x00,
	HAL_ERROR	= 0x01,
	HAL_BUSY	= 0x02,
	HAL_TIMEOUT	= 0x03
} HAL_statusTypeDef;


typedef enum  {
    ERR_NONE                            =  0, /*! No error occurred */
    ERR_INITIALISATION                  =  1, /*! Unable to initialise the SPI comms */
    ERR_COMMS                           =  2, /*! Communications failure has occurred */
    ERR_PARAMETERS                      =  3, /*! Incorrect parameters provided to the function */
	ERR_IO								=  4, /*! Error occured during setup if io ctl */

} ReturnCode;
/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define SPI_MODE_CONFIG		SPI_MODE_0
#define SPI_BITS_PER_WORD	8
#define SPI_MAX_FREQ		20000000

/*
 ******************************************************************************
 * GLOBAL FUNCTIONS
 ******************************************************************************
 */

/*! 
 *****************************************************************************
 * \brief  Initialize SPI Interface
 *  
 * This methods initialize SPI interface so that Linux host can use SPI interface
 *	to communicate with ST25R3911XX.
 *
 * \return ERR_IO	: SPI interface not initialized successfuly 
 * \return ERR_NONE	: No error
 *****************************************************************************
 */
ReturnCode spi_init(void);

/*! 
 *****************************************************************************
 * \brief  SPI Interface
 *  
 * This methods initialize SPI interface so that Linux host can use SPI interface
 *	to communicate with ST25R3911XX.
 *
 * \return ERR_IO	: SPI interface not initialized successfuly 
 * \return ERR_NONE	: No error
 *****************************************************************************
 */
/* function for full duplex SPI communication */
HAL_statusTypeDef spiTxRx(const uint8_t *txData, uint8_t *rxData, uint8_t length);


#endif /*PLATFORMSPI_H */