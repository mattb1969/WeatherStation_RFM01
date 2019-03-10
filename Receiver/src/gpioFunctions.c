/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "../inc/gpioFunctions.h"
#include "../inc/gpio_control.h"
#include "../inc/utilities.h"

/*!
 * need functions to read each of the GPIO pins and return when they change state
 * 
 * Also need to drive the sample and hold pin
 */



int setupGpioFunctions(void) {
	
	gpio_init();
	
	set_gpio_for_input(IF_OUT1);
	set_gpio_for_input(IF_OUT_TO_PI);
	set_gpio_for_output(SAMPLE_HOLD);

	return 0;
};

void setSampleHoldForRun(void) {
	
	set_gpio_value(SAMPLE_HOLD,0);
	return;
};

void setSampleHoldForHold(void) {
	
	set_gpio_value(SAMPLE_HOLD,1);
	return;
};

void readHalfFrequency(void) {
	
	int			measuring_pin;			// The GPIO pin to measure
	float		time_period = 0.00;		// The time between state changes
	time_t		currenttime, starttime;
	int			currentstate, newstate;				// the current GPIO state
	float		timeout = (float) MAX_WAIT_TIME;
	
	systemloop=true;
	
	setupGpioFunctions();
	
	setSampleHoldForRun();
	
	// menu to choose GPIO pin to read
	measuring_pin = chooseGPIOPin();
	
	// loop to show frequency
	printf("CTRL - C to end loop (Half Freq)\n");
	
	// read current state
	currentstate = read_gpio_value(measuring_pin);
	
    // load the current time into the starting time
    starttime = clock(); 

	do {
        currenttime = clock();
        //printf("%ld\n", currenttime);         // debug to check it ran for the right time
		newstate = read_gpio_value(measuring_pin);
		if ( newstate != currentstate) {
			// GPIO has changed state
			usleep(FREQ_DEBOUNCE_TIME);
			newstate = read_gpio_value(measuring_pin);
			if ( newstate != currentstate) {
				currentstate = newstate;
				//printf("DEBUG: GPIO has changed state\n");
				time_period = (float)(currenttime - starttime)/ CLOCKS_PER_SEC;		// Convert the number of ticks to the time
				printf("Frequency:%f\n", (1 / (time_period * 2)));
				starttime = clock();
			};
		};
		if (currenttime > (starttime + (timeout * CLOCKS_PER_SEC))) {
			// timeout has been reached
			//printf("DEBUG: Timeout has been reached, starting again\n");
			printf(".\n");
			starttime = clock();			// reset the timeout clock
		};
	} while (systemloop);
	
	printf("Frequency Measuring completed.\n");
	
	return;
};

void readFullFrequency(void) {
	
	int			measuring_pin;			// The GPIO pin to measure
	float		time_period = 0.00;		// The time between state changes
	time_t		currenttime, starttime, halftime;
	int			startstate, newstate;				// the current GPIO state
	float		timeout = (float) MAX_WAIT_TIME;
	int			halfcycledetected = false;					//flag to identify when the hlaf cycle has been detected
	
	systemloop=true;
	
	setupGpioFunctions();
	
	setSampleHoldForRun();
	
	// menu to choose GPIO pin to read
	measuring_pin = chooseGPIOPin();
	
	// loop to show frequency
	printf("CTRL - C to end loop (Full Freq)\n");
	
	// read current state
	startstate = read_gpio_value(measuring_pin);
	
    // load the current time into the starting time
    starttime = clock();
	halftime = 0;

	do {
        currenttime = clock();
        //printf("%ld\n", currenttime);         // debug to check it ran for the right time
		newstate = read_gpio_value(measuring_pin);
		if ( newstate != startstate) {
			// GPIO has changed state
			usleep(FREQ_DEBOUNCE_TIME);
			newstate = read_gpio_value(measuring_pin);
			if ( newstate != startstate) {
				// I've detected half a cycle after de-bounce
				//printf("DEBUG: GPIO has changed state\n");
				startstate = newstate;
				if (halfcycledetected) {
					// the state has now changed twice as i detected half a cycle beforehand
					time_period = (float)((currenttime - starttime) + halftime)/ CLOCKS_PER_SEC;		// Convert the number of ticks to the time
					printf("Frequency:%f\n", (1 / time_period));
					halfcycledetected = false;	
				}
				else {
					// the state has changed for the first time
					halftime = (currenttime - starttime);
					halfcycledetected = true;	
				}
				starttime = clock();				
			};
		};
		if (currenttime > (starttime + (timeout * CLOCKS_PER_SEC))) {
			// timeout has been reached
			//printf("DEBUG: Timeout has been reached, starting again\n");
			printf(".\n");
			starttime = clock();			// reset the timeout clock
			halfcycledetected = false;
		};
	} while (systemloop);
	
	printf("Frequency Measuring completed.\n");
	
	return;
};

float returnRawFrequency(int measuring_pin) {
	
	float		time_period = 0.00;		// The time between state changes
	float		frequency = 0.00;
	time_t		currenttime, starttime;
	int			currentstate, newstate;				// the current GPIO state
	float		timeout = (float) MAX_WAIT_TIME;
	int			completed = false;					// set to true when complete, either via timeout or trigger
	
	// read current state
	currentstate = read_gpio_value(measuring_pin);
	
    // load the current time into the starting time
    starttime = clock(); 

	do {
        currenttime = clock();
        //printf("%ld\n", currenttime);         // debug to check it ran for the right time
		newstate = read_gpio_value(measuring_pin);
		if ( newstate != currentstate) {
			// GPIO has changed state
			currentstate = newstate;
			//printf("DEBUG: GPIO has changed state\n");
			time_period = (float)(currenttime - starttime)/ CLOCKS_PER_SEC;		// Convert the number of ticks to the time
			frequency = (1 / (time_period * 2));
			completed = true;
		};
		if (currenttime > (starttime + (timeout * CLOCKS_PER_SEC))) {
			// timeout has been reached
			//printf("DEBUG: Timeout has been reached, starting again\n");
			completed = true;
			frequency = 0;
			starttime = clock();			// reset the timeout clock
		};
	} while (completed == false);
	
	return frequency;
};

float returnFullFrequency(int measuring_pin) {
	
	float		time_period = 0.00;		// The time between state changes
	float		frequency = 0.00;
	time_t		currenttime, starttime, halftime;
	int			startstate, newstate;				// the current GPIO state
	float		timeout = (float) MAX_WAIT_TIME;
	int			halfcycledetected = false;					//flag to identify when the half cycle has been detected
	int			completed = false;					// set to true when complete, either via timeout or 
	
	//printf("DEBUG: returning Full Frequency\n");
	// read current state
	startstate = read_gpio_value(measuring_pin);
	
    // load the current time into the starting time
    starttime = clock();
	halftime = 0;
	
	do {
        currenttime = clock();
        //printf("%ld\n", currenttime);         // debug to check it ran for the right time
		newstate = read_gpio_value(measuring_pin);
		if ( newstate != startstate) {
			// GPIO has changed state
			usleep(FREQ_DEBOUNCE_TIME);
			newstate = read_gpio_value(measuring_pin);
			if ( newstate != startstate) {
				// I've detected half a cycle after de-bounce
				//printf("DEBUG: GPIO has changed state\n");
				startstate = newstate;
				if (halfcycledetected) {
					// the state has now changed twice as i detected half a cycle beforehand
					time_period = (float)((currenttime - starttime) + halftime)/ CLOCKS_PER_SEC;		// Convert the number of ticks to the time
					frequency = (1 / (time_period * 2));
					halfcycledetected = false;
					completed = true;
				}
				else {
					// the state has changed for the first time
					halftime = (currenttime - starttime);
					halfcycledetected = true;	
				}
				starttime = clock();				
			};
		};
		if (currenttime > (starttime + (timeout * CLOCKS_PER_SEC))) {
			// timeout has been reached
			//printf("DEBUG: Timeout has been reached\n");
			completed = true;
		};
	} while (completed == false);
	
	return frequency;
};

int chooseGPIOPin(void) {

	int		chosen_pin = 0;			// the GPIO pin selected, zero if not chosen
	char	option;
	char	manual_no[5];
	
	printf("Please choose which GPIO Pin to measure\n");
	printf("1 - IF Out1\n");
	printf("2 - IF Out to Pi\n");
	printf("3 - Manual Entry\n");
	printf("e - Return to main menu\n");
	
	option = getchar();
	switch (option)	{
		case '1':
			chosen_pin = IF_OUT1;
			break;
		case '2':
			chosen_pin = IF_OUT_TO_PI;
			break;
		case '3':
			printf("Please enter the GPIO number:\n");
			fgets(manual_no, 5, stdin);
			chosen_pin = atoi(manual_no);
			break;
		case 'e':
			chosen_pin=0;
			break;
	};
	return chosen_pin;
};

