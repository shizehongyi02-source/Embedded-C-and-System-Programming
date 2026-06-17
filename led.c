#include "led.h"
#include <avr/io.h>

static volatile uint8_t* const led_port[] = {
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTD,
    &PORTB,
    &PORTB,
    &PORTC,
    &PORTC
};

static const uint8_t led_pin[] = {
    PD6,
    PD5,
    PD4,
    PD7,
    PB0,
    PB1,
    PC3,
    PC2
};

static void led_hardware_init(void)
{
    for (uint8_t i = 0; i < 8; i++) {
        if (led_port[i] == &PORTD) {
            DDRD |= (1 << led_pin[i]);
        } else if (led_port[i] == &PORTB) {
            DDRB |= (1 << led_pin[i]);
        } else if (led_port[i] == &PORTC) {
            DDRC |= (1 << led_pin[i]);
        }
    }

    for (uint8_t i = 0; i < 8; i++) {
        *led_port[i] |= (1 << led_pin[i]);
    }
}

int8_t sb_led_on(LED led)
{
    if (led > BLUE1) {
        return -1;
    }

    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    *led_port[led] &= ~(1 << led_pin[led]);

    return 0;
}

int8_t sb_led_off(LED led)
{
    if (led > BLUE1) {
        return -1;
    }

    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    *led_port[led] |= (1 << led_pin[led]);

    return 0;
}

int8_t sb_led_toggle(LED led)
{
    if (led > BLUE1) {
        return -1;
    }

    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    *led_port[led] ^= (1 << led_pin[led]);

    return 0;
}

void sb_led_setMask(uint8_t mask)
{
    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    for (uint8_t i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            *led_port[i] &= ~(1 << led_pin[i]);
        } else {
            *led_port[i] |= (1 << led_pin[i]);
        }
    }
}

int8_t sb_led_showLevel(uint8_t level, uint8_t max)
{
    if (max > 8) {
        return -1;
    }
    if (level > max) {
        return -1;
    }
    if (max == 0) {
        return -2;
    }

    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    uint8_t mask = 0;
    for (uint8_t i = 0; i < level; i++) {
        mask |= (1 << i);
    }

    sb_led_setMask(mask);

    return 0;
}
