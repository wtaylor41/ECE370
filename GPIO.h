/*
 * GPIO.h
 *
 *  Created on: Jun 7, 2017
 *      Author: Jason Forsyth
 *
 *      Modified June 5, 2018
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_

#include <em_gpio.h>

#include "Helper.h"

///Useful location for #define statement to help with masks
/// #define here
///////
#define GPIO_PINMODE_INPUT 0x1
#define GPIO_PINMODE_OUTPUT 0x4

/**
 * Below are suggested functions to help you organize your code. You DO NOT have to use
 * these functions but they were helpful for my implementation. Encapsulating repeated and high-level
 * functionality here will make the main.cpp file earlier to debug and understand.
 */

/**
 * Set pin output value. Only works if correct Port address is provided
 * @param port A GPIO_Port_TypeDef object for the port you wish to modify (gpioPortA, gpioPortB...etc.)
 * @param pinNum The pin number whose output you wish to change/set
 * @param value The value desired of the pin. Should be 0x0 or 0x1.
 */
void setPinOutputValue(GPIO_Port_TypeDef port, uint32_t pinNum, uint32_t value){
	GPIO->P[port].DOUT = (GPIO->P[port].DOUT & ~(0x1 << pinNum)) | (value << pinNum);
}


/**
 * Set pin mode. Only works if correct Port Mode register address is provided
 * @param port A GPIO_Port_TypeDef object for the port you wish to modify (gpioPortA, gpioPortB...etc.)
 * @param pinNum The pin number whose outputa you wish to change/set
 * @param mode The mode (as expressed in hex) that you wish to place the pin into.
 */
void setPinMode(GPIO_Port_TypeDef port, uint32_t pinNum, uint32_t mode){
	if(pinNum < 8) {

		GPIO->P[port].MODEL = (GPIO->P[port].MODEL & ~(0xF << (pinNum << 2))) | (mode << (pinNum << 2));
	}

	else if(pinNum > 7){

		pinNum = pinNum % 8;

		GPIO->P[port].MODEH = (GPIO->P[port].MODEH & ~(0xF << (pinNum << 2))) | (mode << (pinNum << 2));
	}
}

/**
 * Toggle the output state of a pin
 * @param port A GPIO_Port_TypeDef object for the port you wish to modify (gpioPortA, gpioPortB...etc.)
 * @param pinNum The pin number whose output you wish to toggle
 */
void toggleLED(GPIO_Port_TypeDef port, uint32_t pinNum){
	GPIO->P[port].DOUTTGL = (GPIO->P[port].DOUTTGL & ~(0x1 << pinNum)) | (0x1 << pinNum);
}

/**
 * Sets up pin as external interrupt. Enables pin as input, sets edge detection and
 * enables interrupt in NVIC controller
 * @param port A GPIO_Port_TypeDef object for the port you wish to modify (gpioPortA, gpioPortB...etc.)
 * @param pinNum The pin number for which you want to enable interrupts
 * @param risingEdge Enable/Disable rising edge interrupts
 * @param fallingEdge Enable/Disable falling edge interrupts
 */
void setupExternalInterrupt(GPIO_Port_TypeDef port, uint32_t pinNum, bool risingEdge, bool fallingEdge){
	unsigned int shift = pinNum % 8;

	GPIO->IEN = (GPIO->IEN & ~(0x1 << pinNum));

	if(risingEdge == true){
		GPIO->EXTIFALL = (GPIO->EXTIFALL & ~(0x1 << pinNum));
		GPIO->EXTIRISE = (GPIO->EXTIRISE & ~(0x1 << pinNum)) | (0x1 << pinNum);
	}

	else if(fallingEdge == true){
		GPIO->EXTIRISE = (GPIO->EXTIRISE & ~(0x1 << pinNum));
		GPIO->EXTIFALL = (GPIO->EXTIFALL & ~(0x1 << pinNum)) | (0x1 << pinNum);
	}

	GPIO->EXTIPSELH = (GPIO->EXTIPSELH & ~(0x7 << (shift << 2))) | (port << (shift << 2));

	GPIO->IEN = (GPIO->IEN & ~(0x1 << pinNum)) | (0x1 << pinNum);
}



#endif /* SRC_GPIO_H_ */
