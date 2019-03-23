/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   weatherReceiver.c
 * Author: Matthew Bennett <matthew.bennett@bostintechnology.com>
 *
 * Created on 10 March 2019, 13:45
 */

#include <stdio.h>
#include <stdlib.h>
#include "../inc/gpio_control.h"
#include "../inc/ioctl_spi_comms.h"


/*
 * 
 */
int main(int argc, char** argv) {

    uint8_t         msgLen = 4;               // The length of the message 
    uint8_t         txBuf[msgLen];            // The outgoing message
    uint8_t         rxBuf[msgLen];            // The reply from the A-D
	
	spi_init();
	
	txBuf[0] = 0x00;
	txBuf[1] = 0x00;
	txBuf[2] = 0x00;
	txBuf[3] = 0x00;
	
	spiTxRx(txBuf, rxBuf, msgLen);
	
    // For debug purposes
	uint8_t i;
    printf("DEBUG:Response:");
    for (i=0; i < msgLen; i++)
    {
        printf("%02x ", rxBuf[i]);
    }
    printf("\n");
	
	return (EXIT_SUCCESS);
}

