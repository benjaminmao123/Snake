/*
 * Bits.h
 *
 * Created: 1/17/2018 6:20:13 PM
 *  Author: BB
 */ 

#ifndef BITS_H_
#define BITS_H_

unsigned char setBit(unsigned char port, unsigned char bitPos, unsigned char bitNum)
{
	unsigned char temp = 0x00;
	
	if (bitNum == 1)
	{
		temp = port | (0x01 << bitPos);
	}
	else if (bitNum == 0)
	{
		temp = port & ~(0x01 << bitPos);
	}
	
	return temp;
}

unsigned char getBit(unsigned char port, unsigned char bitPos)
{
	unsigned char temp = 0x00;
	
	port = port >> bitPos;
	
	temp = port & 0x01;
	
	return temp;
}

#endif /* BITS_H_ */