/**
 * LIS23DH.cpp
 *
 *  Created on: <DATE>
 *      Author: <NAME>
 */

#include "LIS3DH.h"
#include "GPIO.h"
#include "Helper.h"

/**
 * Default constructor. Sets up default values
 */
LIS3DH::LIS3DH() {
	/**
	 * Default bus speed of 2MHz
	 */
	busSpeed=2000000;

	/**
	 * Default Pin Location #1
	 */
	pinLocation=1;

	/**
	 * Default USART1
	 */
	usart=USART1;

	/**
	 * Default scale range of +/- 2G
	 */
	scaleRange=2;

	/**
	 * Default sensitivity of 1mg/digit
	 */
	sensitivity=1;
}

/**
 * Destructor. Does nothing.
 */
LIS3DH::~LIS3DH() {

}

/**
 * Initialize SPI device to sample at 50Hz, enable
 * all axis, full power on. Set the FIFO to Bypass Mode (default). High Resolution Mode.
 *
 *@return result returns true if initialization completed successfully, otherwise false
 */

bool LIS3DH::initialize()
{
	/**
	 * ensure device has sufficient time to power on
	 * and load configuration information. See datasheet.
	 */
	delay(5);

	/**
	 * Since there is significant setup required for this device
	 * I suggest you break it down into sections/functions to perform each operation.
	 * Below are suggested functions that you can implement. Must declare these functions in the
	 * header file and then implement here.
	 */
	//configure the required clocks
	setupClocks();

	//configure the required GPIO
	setupGPIO();

	//configure the USART
	setupUSART();

	/**
	 * Configure the LIS3DH itself. When switching over to
	 * High Resolution Mode make sure to delay 7ms after switching modes.
	 * See application note for details.
	 */
	setupLIS3DH();

	return true;
}

/**
 * Perform a self-test by reading the WHO_AM_I
 * register.
 * @return returns true is self-test was completed successfully, otherwise false
 */
bool LIS3DH::selfTest()
{
	/**
	 * Perform a SPI read from the WHO_AM_I register.
	 * The returned value should be 0x33 if successful.
	 * Return true if successful, false otherwise.
	 */
	uint8_t whoAmI = readRegister(WHO_AM_I_REG);
	if(whoAmI == 0x33){
		return true;

	} else {
		return false;
	}

}

/**
 * A simple delay function. Do not use for any accurate timing. Not guaranteed to work :)
 * @param num number of milliseconds to delay (approximately)
 */
void LIS3DH::delay(int num)
{
	int counter=0;
	for(int i=0;i<num*1000;i++)
	{
		counter++;
	}
}


void LIS3DH::setupClocks()
{
	CMU_ClockEnable(cmuClock_HFPER, true);

	//enable GPIO clock
	CMU_ClockEnable(cmuClock_GPIO,true);

	//enable USART1 clock
	CMU_ClockEnable(cmuClock_USART1,true);
}

void LIS3DH::setupGPIO()
{
	setPinMode(gpioPortD, 0, GPIO_PINMODE_OUTPUT);
	setPinMode(gpioPortD, 1, GPIO_PINMODE_INPUT);
	setPinMode(gpioPortD, 2, GPIO_PINMODE_OUTPUT);
	setPinMode(gpioPortD, 3, GPIO_PINMODE_OUTPUT);
	setPinOutputValue(gpioPortD, 3, 1);


}

void LIS3DH::setupUSART()
{
	//enable Synchronous Mode
	modifyBitField(&USART1->CTRL, 1, 1, 0);

	//set Clock Rate to 2MHz
	modifyBitField(&USART1->CLKDIV, 0, 0x280, 0);

	//enable TX Buffer Level
	modifyBitField(&USART1->CTRL, 1, 0x1, 12);

	//set Clock Phase to Trailing Edge
	modifyBitField(&USART1->CTRL, 1, 0x1, 9);

	//set Clock Polarity to Idle High
	modifyBitField(&USART1->CTRL, 1, 0x1, 8);

	//set Packet Framing to 8 data bits; no stop; no parity
	modifyBitField(&USART1->FRAME, 0, 0x5, 0);

	//set Bit Order to MSBF
	modifyBitField(&USART1->CTRL, 1, 0x1, 10);

	//enable TX Pin
	modifyBitField(&USART1->ROUTE, 1, 0x1, 1);

	//enable RX Pin
	modifyBitField(&USART1->ROUTE, 1, 0x1, 0);

	//enable CLK Pin
	modifyBitField(&USART1->ROUTE, 1, 0x1, 3);

	//set Pin Location to Loc#1
	modifyBitField(&USART1->ROUTE, 7, 0x1, 8);

	modifyBitField(&USART1->CMD, 1, 0x1, 2);

	modifyBitField(&USART1->CMD, 1, 0x1, 0);

	modifyBitField(&USART1->CMD, 1, 0x1, 4);
}

void LIS3DH::setupLIS3DH(){

	writeRegister(0x20, 0b01000111);
	writeRegister(0x23, (0x1 << 3));

}
uint8_t LIS3DH::readRegister(uint8_t address)
{
	//pull ClkSel low
	setPinOutputValue(gpioPortD, 3, 0);

	//SPI Read address transfer
	SPI_Transfer(address | 0x80);

	//SPI dummy data transfer
	SPI_Transfer(0x00);

	//raise CS line
	setPinOutputValue(gpioPortD, 3, 1);

	return USART1->RXDATA;
}

void LIS3DH::writeRegister(uint8_t address , uint8_t data)
{
	//lower CS line
	setPinOutputValue(gpioPortD, 3, 0);

	//write address and data
	SPI_Transfer(address);
	SPI_Transfer(data);

	//raise CS line
	setPinOutputValue(gpioPortD, 3, 1);

	return;
}

uint8_t LIS3DH::SPI_Transfer(uint8_t data)
{
	//booleans for buffer empty and buffer complete
	bool TXBufEmpty = false;
	bool TXComplete = false;

	while(TXBufEmpty == false){
		int TXBuf = readBit(&USART1->STATUS, 6);
		if (TXBuf == 1){
			TXBufEmpty = true;
		}
	}

	//put data in TX register
	USART1->TXDATA = data;

	while(TXComplete == false){
		int TXC = readBit(&USART1->STATUS, 5);
		if(TXC == 1){
			TXComplete = true;
		}
	}

	//return data in RXDATA
	return USART1->RXDATA;
}
float LIS3DH::getXAcceleration(){
	uint8_t high = (readRegister(0x29));
	uint8_t low = readRegister(0x28);
	return ((uint16_t)(high << 8) | (uint16_t)low);
}
float LIS3DH::getYAcceleration(){
	uint8_t high = (readRegister(0x2A));
	uint8_t low = readRegister(0x2B);

	return ((uint16_t)(high << 8) | (uint16_t)low);

}
float LIS3DH::getZAcceleration(){
	uint8_t high = (readRegister(0x2C));
	uint8_t low = readRegister(0x2D);
	return ((uint16_t)(high << 8) | (uint16_t)low);
}
