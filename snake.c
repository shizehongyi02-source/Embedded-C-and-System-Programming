#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "led.h"
#include "adc.h"
#include "button.h"

uint8_t wait(void) {
    int16_t brightness = sb_adc_read(PHOTO);
    if (brightness < 0) brightness = 0;

    uint32_t delay = 20000UL + (1023 - (uint32_t)brightness) * 40UL;

    if (delay < 15000UL) {
        delay = 15000UL;
    }

    volatile uint32_t i;
    uint8_t toggle = 0;
    uint8_t last_state = 1;
    uint8_t button_pressed = 0;

    for (i = 0; i < delay; i++) {
        BUTTONSTATE btn_state = sb_button_getState(BUTTON0);
        uint8_t current_state = (btn_state == PRESSED) ? 0 : 1;

        if (!button_pressed && (last_state == 1) && (current_state == 0)) {
            toggle ^= 1;
            button_pressed = 1;
        }

        if (button_pressed && (current_state == 1)) {
            button_pressed = 0;
        }

        last_state = current_state;
    }

    return toggle;
}

void drawSnake(uint8_t head, uint8_t length, uint8_t modus) {
    uint8_t mask = 0;

    for (uint8_t i = 0; i < length; i++) {
        uint8_t pos = (head + i) % 8;
        mask |= (1 << pos);
    }

    if (modus == 0) {
        sb_led_setMask(mask);
    } else {
        sb_led_setMask(~mask);
    }
}

static uint8_t get_snake_length(void) {
    int16_t poti = sb_adc_read(POTI);
    if (poti < 0) poti = 0;

    if (poti < 205) return 1;
    if (poti < 410) return 2;
    if (poti < 615) return 3;
    if (poti < 820) return 4;
    return 5;
}

int main(void) {
    uint8_t head = 0;
    uint8_t length = 1;
    uint8_t modus = 0;
    uint8_t need_toggle = 0;

    sei();

    while (1) {
        length = get_snake_length();
        drawSnake(head, length, modus);
        need_toggle = wait();

        if (need_toggle) {
            modus = !modus;
        }

        head = (head + 1) % 8;
    }

    return 0;
}
