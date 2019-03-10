/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   gpio_control.h
 * Author: Matthew
 *
 * Created on 22 August 2018, 15:54
 */



#ifndef GPIO_CONTROL_HEADER
#define GPIO_CONTROL_HEADER

#define BLOCK_SIZE		(4*1024)

/* The range of GPIO pins available to use*/
#define LOWEST_GPIO_PIN					0			// Lowest acceptable GPIO pin
#define HIGHEST_GPIO_PIN				53			// Highest acceptable GPIO pin

/* Offset to the registers. The first register is zero, the rest are listed below
 * The offset is to the first register in a type, e.g. The PIn register for GPIO's 0 - 31
 * There is no offset for the first register as this starts at zero */
#define GPIO_SET_OFFSET					7			// GPSET0 Pin Output Set registers
#define GPIO_CLEAR_OFFSET				10			// GPCLR0 Pin Output Clear registers
#define GPIO_LEVEL_OFFSET				13			// GPLEV0 Pin Level register


// functions
/* Setup the GPIO system ready for use
 * extracts the map and check it is ready to use
 * Doesn't include specific pin assignments*/
int gpio_init();							// Initialise the gpio system

/* Function to set the GPIO pin to be an input.
 * - Requires an int for the GPIO pin to be set
 * - Returns 0 or -1 if error
 * The first 5 blocks of the map are the function select pins, each block being 32bits (4 bytes)
 * long and consisting of 3 bits per gpio pin, addressed as a single 32bit.
 * Address for gpio0 is 0 in the gpio_mmap, address for gpio9 is also 0, but different bits
 * address for gpio15 is 1 as it is the next block
 */
int set_gpio_for_input(int pin_no);			// Set a GPIO pin for input

/* Function to set the GPIO pin to be an output
 * - Requires and int for the GPIO pin number
 * - returns 0
 * Very similar to the set_gpio_for_input above except it is writing a different value
 * For an explanation of how it works, please refer to above.
 * value to be written for output is 001, not 000 for input
 */
int set_gpio_for_output(int pin_no);		// Set a GPIO pin for output

/* For setting the output value of a GPIO pin.
 * - Requires an int for pin number and int for the value to be written (1 or 0)
 * - returns 0 or -1 if there is an error
 * To set the GPIO pin one of 2 different operations are required as the 
 * registers are organised as Set and Clear registers
 * For example, to turn the GPIO pin on, write 1 to the set register, but to turn
 * it off write 1 to the clear register
 * There are 2 registers per Set & Clear and are organised as GPIO pins 0 - 31 and 32 - 53
 */
int set_gpio_value (int pin_no, int value);	// Set the value (0 or 1) of a GPIO pin

/* For reading of the GPIO value
 * - Requires an int pin number in range 0 - 53
 * - Returns an int with the value either 1 or 0 or a negative number if an error
 * To read the GPIO pin, select the register (13 or 14) and return the required bit
 */
int read_gpio_value(int pin_no);			// Read a GPIO pin value




#endif


