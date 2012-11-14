// ======================================================================
// Li-ion Voltage Meter
//
// Copyright 2012 Hell-Prototypes
//
// http://code.google.com/p/hell-prototypes/
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#define PIN_LOW(pin)		(PORTB &= ~(1<<pin))
#define PIN_HIGH(pin)		(PORTB |= (1<<pin))

#define PIN_INPUT(pin)		(DDRB &= ~(1<<pin))
#define PIN_OUTPUT(pin)		(DDRB |= (1<<pin))

#define RES_DIV					4
#define CAP_FULL_ADC_VALUE		((42*1023)/(11*RES_DIV))	//4.2V/1.1V/4 * 1023
#define CAP_EMPTE_ADC_VALUE		((30*1023)/(11*RES_DIV))	//3V/1.1V/4 * 1023
#define CAP_RANGE_ADC_VALUE		(((42-30)*1023)/(11*RES_DIV))


void ADC_init(void)
{
	ADCSRA=0x00;
	ADMUX = (1 << REFS0) | (1 << MUX1);//1.1V ref, ADC2
	ACSR = (1 << ACD);

	ADCSRA = (1 << ADEN) | (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
}

uint8_t get_vol(void)
{
	volatile uint16_t adc_val;
	volatile uint8_t vol;

	ADC = 0;

	ADCSRA  |=  (1 << ADSC);

    loop_until_bit_is_clear(ADCSRA, ADSC);

	adc_val = ADC;

	if(adc_val > CAP_EMPTE_ADC_VALUE) {
		vol = ((adc_val - CAP_EMPTE_ADC_VALUE) * 12) / (CAP_RANGE_ADC_VALUE);
	} else {
		vol = 0;
	}

	return vol;
}

void sys_init(void)
{
	//Clock
	CLKPR = 1<<CLKPCE;
	CLKPR = (1<<CLKPS1) | (1<<CLKPS0);	//1200KHz
	//CLKPR = (1<<CLKPS2) | (1<<CLKPS0);	//300KHz

	DDRB = 0x0F;
	PORTB = 0;

	sleep_enable();

	sei();	//Enables global interrupt
	//cli();	//Disables global interrupt
}
/* WDT interrupt */
ISR(WDT_vect, ISR_NAKED)
{
	reti();
}

int main(void)
{
	uint8_t vol;

	sys_init();
	ADC_init();

	while(1) {
		//PORTB = ~PORTB;
		vol = get_vol();
		PORTB = (~vol) & 0x0F;
		// set watchdog in interrupt mode and 128k cycles
		WDTCR = (0<<WDE) | (1<<WDTIE) | (1<<WDP2) | (1<<WDP1);//1S

		//reset the watchdog timer to full value and sleep until it pops an interrupt
		wdt_reset();
		sleep_cpu();
	}
	return 0;
}
