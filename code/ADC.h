/*
 * ADC.h
 *
 * Created: 2/13/2018 11:49:00 AM
 *  Author: BB
 */ 

/***************************************************************************************
*    Title: <ACD.h>
*    Author: <Mayank>
*    Date: <2011>
*    Availability: <http://maxembedded.com/2011/06/the-adc-of-the-avr/>
*
***************************************************************************************/

#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>

void adc_init()
{
	// For Aref=AVcc
	ADMUX = (1 << REFS0);     
	                    
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}

uint16_t adc_read(uint8_t ch)
{
	// select the corresponding channel 0~7
	// ANDing with ’7′ will always keep the value
	// of ‘ch’ between 0 and 7
	
	// AND operation with 7
	ch = ch & 0b00000111;
	
	// clears the bottom 3 bits before ORing
	ADMUX = (ADMUX & 0xF8) | ch;

	//start single conversion
	ADCSRA |= (1 << ADSC);

	// start single conversion
	// write ’1′ to ADSC
	while (ADCSRA & (1 << ADSC));

	// wait for conversion to complete
	// ADSC becomes ’0′ again
	// till then, run loop continuously
	return ADC;
}

#endif /* ADC_H_ */