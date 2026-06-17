#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "led.h"
#include "7seg.h"
#include "adc.h"

static volatile uint8_t g_button0_pressed = 0;
static volatile uint8_t g_button1_pressed = 0;
static volatile uint8_t g_timer_1s = 0;

typedef enum {
    STATE_OFF,
    STATE_BOOT,
    STATE_7SEG,
    STATE_LED
} panel_state_t;

ISR(INT0_vect) {
    g_button0_pressed = 1;
}

ISR(INT1_vect) {
    g_button1_pressed = 1;
}

ISR(TIMER0_OVF_vect) {
    static uint8_t tick_1s = 0;

    tick_1s++;
    if (tick_1s >= 64) {
        tick_1s = 0;
        g_timer_1s = 1;
    }
}

static void init_buttons(void) {
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);
    EIMSK |= (1 << INT0);

    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);
    EICRA |= (1 << ISC11);
    EICRA &= ~(1 << ISC10);
    EIMSK |= (1 << INT1);
}

static void init_timer0(void) {
    TCCR0B |= (1 << CS00) | (1 << CS02);
    TIMSK0 |= (1 << TOIE0);
}

static void adc_init(void) {
    ADMUX = (1 << REFS0);
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    for (int i = 0; i < 10; i++) {
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        (void)ADC;
    }
}

static uint8_t get_percentage(void) {
    int16_t adc_value = sb_adc_read(PHOTO);
    if (adc_value < 0) adc_value = 0;

    uint32_t percent = ((uint32_t)adc_value * 100) / 1024;
    if (percent > 99) percent = 99;
    return (uint8_t)percent;
}

static void set_mode(panel_state_t mode, uint8_t percentage) {
    if (mode == STATE_7SEG) {
        sb_7seg_showNumber(percentage);
        sb_led_setMask(0x00);
    } else if (mode == STATE_LED) {
        uint8_t level = (percentage * 8) / 100;
        sb_led_showLevel(level, 8);
        sb_7seg_disable();
    }
}

static void all_disable(void) {
    sb_7seg_disable();
    sb_led_setMask(0x00);
}

int main(void) {
    panel_state_t state = STATE_OFF;
    uint8_t percentage = 0;
    uint8_t boot_timer = 0;
    uint8_t update_timer = 0;
    uint8_t inactivity_timer = 0;

    adc_init();
    init_buttons();
    init_timer0();
    sei();

    all_disable();

    while (1) {
        if (g_timer_1s) {
            g_timer_1s = 0;

            if (boot_timer < 2) boot_timer++;
            if (update_timer < 2) update_timer++;
            if (inactivity_timer < 30) inactivity_timer++;
        }

        if (g_button0_pressed) {
            g_button0_pressed = 0;
            if (state == STATE_OFF) {
                state = STATE_BOOT;
                sb_7seg_showString("So");
                boot_timer = 0;
                inactivity_timer = 0;
            }
        }

        if (g_button1_pressed) {
            g_button1_pressed = 0;

            if (state == STATE_7SEG) {
                state = STATE_LED;
                percentage = get_percentage();
                set_mode(state, percentage);
                inactivity_timer = 0;
            } else if (state == STATE_LED) {
                state = STATE_7SEG;
                percentage = get_percentage();
                set_mode(state, percentage);
                inactivity_timer = 0;
            }
        }

        switch (state) {
            case STATE_OFF:
                all_disable();
                sleep_enable();
                sleep_cpu();
                sleep_disable();
                break;

            case STATE_BOOT:
                if (boot_timer >= 1) {
                    state = STATE_7SEG;
                    percentage = get_percentage();
                    set_mode(state, percentage);
                    update_timer = 0;
                    inactivity_timer = 0;
                }
                break;

            case STATE_7SEG:
                sb_led_setMask(0x00);
                sb_7seg_showNumber(percentage);

                if (update_timer >= 1) {
                    update_timer = 0;
                    percentage = get_percentage();
                    sb_7seg_showNumber(percentage);
                }

                if (inactivity_timer >= 15) {
                    state = STATE_OFF;
                }
                break;

            case STATE_LED:
                sb_7seg_disable();

                {
                    uint8_t level = (percentage * 8) / 99;
                    sb_led_showLevel(level, 8);
                }

                if (update_timer >= 1) {
                    update_timer = 0;
                    percentage = get_percentage();
                    uint8_t level = (percentage * 8) / 99;
                    sb_led_showLevel(level, 8);
                }

                if (inactivity_timer >= 15) {
                    state = STATE_OFF;
                }
                break;
        }
    }

    return 0;
}
