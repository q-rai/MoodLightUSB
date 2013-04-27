#define F_CPU 16000000ul
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// volatile variables!
volatile uint16_t redCycle, greenCycle, blueCycle; 

/**
 * Set PORTB each cycle.
 * PORTB = RGB00000, so red=0x80, green=0x40, blue=0x20.
 */
ISR(TIMER0_OVF_vect)
{
   static uint8_t counter = 0;
   counter++;
   //PORTB ^= 0xA0;
   if(counter < redCycle)
      PORTB |= 0x80;
   else
      PORTB &= ~(0x80);
   if(counter < greenCycle)
      PORTB |= 0x40;
   else
      PORTB &= ~(0x40);
   if(counter < blueCycle)
      PORTB |= 0x20;
   else
      PORTB &= ~(0x20);
}

int main()
{
   // WGM0 = 01: phase-correct pwm 
   TCCR0A |= (0 << WGM01) | (1 << WGM00);
   // clock prescaler needs to be set to 0x80 
   // before setting it desired value within 4 cycles
   // (see data sheet page 39)
   CLKPR = 0x80;
   CLKPR = 0x00;
   // timer/counter0 control register A 
   // CS1 = 001: clk_{I/O}/(no prescaler)
   TCCR0B |= (0 << CS02) | (0<< CS01) | (1 << CS00);
   // set Timer/Counter0 Overflow Interrupt Enable
   // so interrupt TIMER0_OVF_vect is used
   TIMSK0 |= (1 << TOIE0);
   // set the colour as output
   DDRB |= (1 << PB7) | (1 << PB6) | (1 << PB5);
   sei();

    while(1)
    {
       /* // deactivate interrupts
        cli();
        // set colour
        redCycle = 0xac;
        greenCycle = 0x58;
        blueCycle = 0xfa;
        sei();
        _delay_ms(1000);
        cli();
        redCycle = 0xf7;
        greenCycle = 0xd3;
        blueCycle = 0x58;
        sei();
        _delay_ms(1000);
        cli();
        redCycle = 0x01;
        greenCycle = 0xdf;
        blueCycle = 0xa5;
        sei();
        _delay_ms(1000); */
	cli();
	PORTB = 0b10000000;
	_delay_ms(10000);
	PORTB = 0b11000000;
	_delay_ms(10000);
	PORTB = 0b01000000;
	_delay_ms(10000);
	PORTB = 0b01100000;
	_delay_ms(10000);
	PORTB = 0b00100000;
	_delay_ms(10000);
	PORTB = 0b10100000;
	_delay_ms(10000);
    }
}
