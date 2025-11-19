#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <stdio.h>

#include "avr_mcu_section.h"
#ifdef F_CPU
  #undef F_CPU
#endif
#define F_CPU 1000000
AVR_MCU(F_CPU, "atmega328");

/*
const struct avr_mmcu_vcd_trace_t _mytrace[]  _MMCU_ = {
	{ AVR_MCU_VCD_SYMBOL("MYPIN"), .mask = (1 << 0), .what = (void*)&PIND, },
};
*/

static int uart_putchar(char c, FILE *stream) {
  if (c == '\n')
    uart_putchar('\r', stream);
  loop_until_bit_is_set(UCSR0A, UDRE0);
  UDR0 = c;
  return 0;
}
static FILE mystdout = FDEV_SETUP_STREAM(uart_putchar, NULL,
                                         _FDEV_SETUP_WRITE);


void
receiver_int_init(void)
{
	// Set INT0 pin as input
	DDRD &= ~(1 << PD2);
	// Falling edge on INT0 generates an interrupt (see DS p.54)
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);
	// Enable interrupt pin PD2
	EIMSK |= (1 << INT0);
}

void
transmitter_int_init(void)
{
    //timer0_init
    //   Set Timer0 to CTC Mode
    //   Set the WGM01 bit in TCCR0A to enable CTC mode [1, 6, 10]
    TCCR0A |= 1<<WGM01;
    // Enable the Timer0 Compare Match A Interrupt
    //   This enables the interrupt service routine (ISR) to be called when TCNT0 matches OCR0A [1, 6]
    TIMSK0 |= (1 << OCIE0A);
    TCNT0 = 0; //reset timer 0
    TCCR0B = (1 << CS02); //start timer0 mit Prescaler 256
}




int
main()
{
  stdout = &mystdout;
  uint8_t avrIdx;
  _EEGET(avrIdx,0);
  printf("%02X: Hallo\n",avrIdx);

	// Disable interrupts whilst configuring them to avoid false triggers
	cli();
/*	if(avrIdx == 0) {
	  receiver_int_init();
    DDRB |= (1<<PB0);
    PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB0);
    PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB0);
    PORTB |= (1<<PB0);
    PORTB &= ~(1<<PB0);
	  sei();
	  while (1) {
	    sleep_mode();
		  //printf("R\n");
	  }
  }
	else { */
	  transmitter_int_init();
	  DDRD |= 1;
	  sei();
	  uint8_t int_cnt=0;
	  while (1) {
	    sleep_mode();
		  if( int_cnt++ == 20 ) break;
	  }
//  }
	cli();
	sleep_mode();
}

ISR(INT0_vect)
{
  //Toggle PB0 on each Interrupt
  static uint8_t phase = 1;
  if(phase) PORTB |= (1<<PB0);
  else PORTB &= ~(1<<PB0);
  phase ^= 1;
}

ISR(TIMER0_COMPA_vect) {
  //Toggle DDR of the BUS_PIN on every Interrupt
  if(PORTD & 1) { // Pin ist HIGH -> muss LOW werden
    PORTD &= ~1; // LOW
  }
  else {
    PORTD |= 1; // HIGH
  }
}

