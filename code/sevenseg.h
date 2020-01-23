/*
 * _sevenseg.h
 *
 * Created: 3/14/2018 8:25:19 AM
 *  Author: BB
 */ 


#ifndef SEVENSEG_H_
#define SEVENSEG_H_

#include <avr/io.h>

#include "Bits.h"

#define SER_LOW()    PORTD = setBit(PORTD, 0, 0)
#define SER_HIGH()   PORTD = setBit(PORTD, 0, 1)
#define RCLK_LOW()   PORTD = setBit(PORTD, 1, 0)
#define RCLK_HIGH()  PORTD = setBit(PORTD, 1, 1)
#define SRCLK_LOW()  PORTD = setBit(PORTD, 2, 0)
#define SRCLK_HIGH() PORTD = setBit(PORTD, 2, 1)
#define HC595INIT_PORT()  DDRD |= setBit(PORTA, 0, 1) | setBit(PORTA, 1, 1) | setBit(PORTA, 2, 1)

unsigned char Segments[5] = { 0x00, 0x06, 0x5B, 0x4F, 0x66 };	
//void transmit_data(unsigned char data) {
	//int i;
	//for (i = 0; i < 8 ; ++i) {
		//// Sets SRCLR to 1 allowing data to be set
		//// Also clears SRCLK in preparation of sending data
		//PORTD = 0x01;
		//// set SER = next bit of data to be sent.
		//PORTD |= ((data >> i) & 0x01);
		//// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		//PORTD |= 0x02;
	//}
	//// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	//PORTD |= 0x04;
	//// clears all lines in preparation of a new transmission
	//PORTD = 0x00;
//}

void hc595transmit_data(unsigned char data)
{
	for (unsigned char i = 0; i < 8; ++i, data <<= 1)
	{
		SRCLK_LOW();
		
		if (data & 0x80)
		{
			SER_HIGH();
		}
		else
		{
			SER_LOW();
		}
		
		SRCLK_HIGH();
	}
}

void sevenseg_write(unsigned char number)
{
	RCLK_LOW();
	hc595transmit_data(Segments[number]);
	RCLK_HIGH();
}

void sevenseg_clear(void)
{
	sevenseg_write(0);
}

void sevenseg_init()
{
	HC595INIT_PORT();
	sevenseg_write(8);
	sevenseg_clear();
}

#endif /* SEVENSEG_H_ */