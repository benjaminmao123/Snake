/*
 * matrix.h
 *
 * Created: 3/8/2018 12:24:49 AM
 *  Author: BB
 */ 


#ifndef MATRIX_H_
#define MATRIX_H_

#include <avr/io.h>

#include "Bits.h"

#define DATA_HIGH() PORTB = setBit(PORTB, 2, 1)
#define DATA_LOW()  PORTB = setBit(PORTB, 2, 0)
#define CS_HIGH()   PORTB = setBit(PORTB, 1, 1)
#define CS_LOW()    PORTB = setBit(PORTB, 1, 0)
#define CLK_HIGH()  PORTB = setBit(PORTB, 0, 1)
#define CLK_LOW()   PORTB = setBit(PORTB, 0, 0)
#define INIT_PORT() DDRB |= setBit(PORTB, 0, 1) | setBit(PORTB, 1, 1) | setBit(PORTB, 2, 1)

void MAX7219transmit_data(unsigned char data)
{
	for (unsigned char i = 0; i < 8; ++i, data <<= 1)
	{
		CLK_LOW();
		
		if (data & 0x80)
		{
			DATA_HIGH();
		}
		else
		{
			DATA_LOW();
		}
		
		CLK_HIGH();
	}
}

void matrix_write(unsigned char x, unsigned char y)
{
	CS_LOW();
	MAX7219transmit_data(x);
	MAX7219transmit_data(y);
	CS_HIGH();
}

void matrix_clear(void)
{
	for (unsigned char i = 0; i < 8; ++i)
	{
		matrix_write(i + 1, 0);
	}
}

void matrix_init(void)
{
	INIT_PORT();
	matrix_write(0x09, 0);
	matrix_write(0x0A, 1);
	matrix_write(0x0B, 7);
	matrix_write(0x0C, 1);
	matrix_write(0x0F, 0);
	matrix_clear();
}

#endif /* MATRIX_H_ */