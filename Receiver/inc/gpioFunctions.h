/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gpioFunctions.h
 * Author: Matthew Bennett <matthew.bennett@bostintechnology.com>
 *
 * Created on 18 January 2019, 22:14
 */

#ifndef GPIOFUNCTIONS_H
#define GPIOFUNCTIONS_H

// The various GPIO Pins that can be read

#define		IF_OUT1				27	// The GPIO pin used for the frequency input, was 4 originally and also tried 11
#define		IF_OUT_TO_PI		22
#define		SAMPLE_HOLD			17	// 

#define		MAX_WAIT_TIME		3		// The maximum time allowed to register a GPIO state time in seconds.
#define		FREQ_DEBOUNCE_TIME	500		// The time in microseconds to wait for the check time




/*!**************************************************************************
 * Overview:  Set the GPIO pins ready for use
 *  
 * Description: This method sets up the GPIO pins for input and output
 *				Sets IF_OUT1 and IF_OUT_TO_PI for input
 *				Sets SAMPLE_HOLD for output
 *
 * Parameters:
 * param[in]	pin		: gpio pin to monitor
 *		[in]	timeout	: float of the maximum time to wait
 *
 * return		0       : no time
 *				float	: duration
 *****************************************************************************/
int setupGpioFunctions(void);

/*!**************************************************************************
 * Overview:  Set the Sample & Hold pin to running state
 *  
 * Description: This method sets up the Sample & Hold pin low so that it
 *				It is in running state
 *
 * Parameters:
 * param[in]	??		: none
 *
 * return		??      : nothing
 *****************************************************************************/
void setSampleHoldForRun(void);

/*!**************************************************************************
 * Overview:  Set the Sample & Hold pin to HOLD state
 *  
 * Description: This method sets up the Sample & Hold pin high so that it
 *				It is in HOLD state
 *
 * Parameters:
 * param[in]	??		: none
 *
 * return		??      : nothing
 *****************************************************************************/
void setSampleHoldForHold(void);

/*!**************************************************************************
 * Overview: Function to read the frequency from the GPIO pins
 *  
 * Description: This method first calls the menu choice and then measures the
 *				time and displays the frequency determined.
 *				Note: The reading is only half the time period, which is then
 *				multiplied to give a frequency reading
 *
 * Parameters:
 * param[in]	none	: 
 *
 * return		0       : nothing
 *****************************************************************************/
void readHalfFrequency(void);

/*!**************************************************************************
 * Overview: Function to read the frequency from the GPIO pins
 *  
 * Description: This method first calls the menu choice and then measures the
 *				time and displays the frequency determined.
 *				Note: The method waits for the whole cycle to be competed before 
 *				returning the frequency.
 *
 * Parameters:
 * param[in]	none	: 
 *
 * return		0       : nothing
 *****************************************************************************/
void readFullFrequency(void);

/*!**************************************************************************
 * Overview: Function to return the frequency from the GPIO pins
 *  
 * Description: This method only uses half the cycle and no debounce, but is
 *				similar to readFullFrequency above.
 *
 * Parameters:
 * param[in]	measuring_pin	: the pin to be used.
 *
 * return		float       : frequency
 *****************************************************************************/
float returnRawFrequency(int measuring_pin);

/*!**************************************************************************
 * Overview: Function to return the frequency from the GPIO pins
 *  
 * Description: This method waits for the whole cycle to be competed before 
 *				returning the frequency.
 *				Requires setupGpioFunctions() & setSampleHoldForRun() to be run 
 *				first
 *
 * Parameters:
 * param[in]	measuring_pin	: the pin to be used.
 *
 * return		float       : frequency
 *****************************************************************************/
float returnFullFrequency(int measuring_pin);

/*!**************************************************************************
 * Overview: Menu function for user selection of the GPIO pin
 *  
 * Description: This method provides the user with a menu of choice to choose
 *				which GPIO pin is to be measured
 *
 * Parameters:
 * param[in]	none	: 
 *
 * return		gpio_pin    : GPIO pin
 *				0			: zero if no pin selected
 *****************************************************************************/
int chooseGPIOPin (void);

#endif /* GPIOFUNCTIONS_H */

