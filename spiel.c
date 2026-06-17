#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "led.h"
#include "7seg.h"
#include "timer.h"

#define initial_speed 500
#define min_speed 50
#define step_delay 100

static volatile uint8_t g_button_pressed = 0;

ISR(INT0_vect) {
    g_button_pressed = 1;
}

static void init_button_int0(void) {
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);
    EICRA &= ~(1 << ISC00);
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
}

static uint8_t get_state_mask(const uint8_t led_state[]) {
    uint8_t mask = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (led_state[i]) {
            mask |= (1 << i);
        }
    }
    return mask;
}

static uint8_t create_cursor_mask(const uint8_t led_state[], uint8_t cursor_pos) {
    uint8_t mask = get_state_mask(led_state);
    mask ^= (1 << cursor_pos);
    return mask;
}

static uint8_t is_all_lit(const uint8_t led_state[]) {
    for (uint8_t i = 0; i < 8; i++) {
        if (!led_state[i]) {
            return 0;
        }
    }
    return 1;
}

static uint16_t calculate_speed(uint8_t level) {
    uint16_t speed = initial_speed / (1 + (level - 1) / 3);
    if (speed < min_speed) {
        speed = min_speed;
    }
    return speed;
}

static void play(uint16_t speed) {
    uint8_t led_state[8] = {0};
    uint8_t cursor_pos = 0;
    int8_t direction = 1;
    uint8_t level_complete = 0;

    while (!level_complete) {
        sb_led_setMask(create_cursor_mask(led_state, cursor_pos));
        sb_timer_delay(speed);
        sb_led_setMask(get_state_mask(led_state));

        if (g_button_pressed) {
            g_button_pressed = 0;
            led_state[cursor_pos] = !led_state[cursor_pos];
            sb_led_setMask(get_state_mask(led_state));
        }

        if (is_all_lit(led_state)) {
            level_complete = 1;
            break;
        }

        cursor_pos += direction;

        if (cursor_pos == 7) {
            direction = -1;
        } else if (cursor_pos == 0) {
            direction = 1;
        }
    }
}

static void show_win(void) {
    uint8_t i;

    for (i = 7; i < 8; i--) {
        sb_led_off((LED)i);
        sb_timer_delay(step_delay);
        if (i == 0) break;
    }

    for (i = 7; i < 8; i--) {
        sb_led_on((LED)i);
        sb_timer_delay(step_delay / 2);
        sb_led_off((LED)i);
        if (i == 0) break;
    }

    for (i = 0; i <= 7; i++) {
        sb_led_on((LED)i);
        sb_timer_delay(step_delay / 2);
        sb_led_off((LED)i);
    }

    for (i = 0; i < 4; i++) {
        sb_led_on((LED)i);
        sb_led_on((LED)(7 - i));
        sb_timer_delay(step_delay);
    }

    for (int i = 3; i >= 0; i--) {
        sb_led_off((LED)i);
        sb_led_off((LED)(7 - i));
        sb_timer_delay(step_delay);
    }

    sb_led_setMask(0x00);
}

int main(void) {
    uint8_t level = 1;
    uint16_t speed;

    init_button_int0();
    sb_led_setMask(0x00);
    sei();
    sb_7seg_showNumber(level);

    while (1) {
        speed = calculate_speed(level);
        play(speed);
        show_win();
        level++;
        sb_7seg_showNumber(level);
    }

    return 0;
}
