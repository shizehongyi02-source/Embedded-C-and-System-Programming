#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <7seg.h>
#include <led.h>

static void init(void){
    DDRD &= ~(1<<PD2);
    PORTD |= (1<<PD2);
    EICRA &= ~(1<<ISC00);
    EICRA |= (1<<ISC01);
    EIMSK |= (1<<INT0);
}

static volatile uint8_t button_event = 0;

ISR(INT0_vect){
    button_event = 1;
}

void main (void){
    init();
    sleep_enable();
    sei();

    uint8_t counter = 0;
    sb_7seg_showNumber(counter);

    while (1){
        cli();
        while(button_event != 1){
            sei();
            sleep_cpu();
            cli();
        }
        button_event = 0;
        sei();

        counter = (counter + 1) % 100;
        sb_7seg_showNumber(counter);
    }
}
