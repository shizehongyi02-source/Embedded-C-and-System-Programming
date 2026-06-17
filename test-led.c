#include "led.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

static void wait_ms(uint16_t ms)
{
    sb_timer_delay(ms);
}

static void check_result(int8_t result)
{
    if (result != 0) {
        while (1) {
            for (uint8_t i = 0; i < 8; i++) {
                sb_led_on((LED)i);
            }
            sb_timer_delay(100);
            for (uint8_t i = 0; i < 8; i++) {
                sb_led_off((LED)i);
            }
            sb_timer_delay(100);
        }
    }
}

int main(void)
{
    int8_t result;

    sei();

    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_on(led);
        check_result(result);
        wait_ms(200);
    }
    wait_ms(500);

    for (LED led = RED0; led <= BLUE1; led++) {
        sb_led_off(led);
    }

    for (LED led = RED0; led <= BLUE1; led++) {
        sb_led_on(led);
    }
    wait_ms(500);

    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_off(led);
        check_result(result);
        wait_ms(200);
    }

    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_toggle(led);
        check_result(result);
        wait_ms(100);
    }
    wait_ms(500);

    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_toggle(led);
        check_result(result);
        wait_ms(100);
    }

    result = sb_led_on((LED)(BLUE1 + 1));
    if (result != -1) while (1);

    result = sb_led_off((LED)(BLUE1 + 1));
    if (result != -1) while (1);

    result = sb_led_toggle((LED)(BLUE1 + 1));
    if (result != -1) while (1);

    result = sb_led_on((LED)255);
    if (result != -1) while (1);

    sb_led_setMask(0x00);
    wait_ms(500);

    sb_led_setMask(0xFF);
    wait_ms(500);

    sb_led_setMask(0x55);
    wait_ms(1000);

    sb_led_setMask(0xAA);
    wait_ms(1000);

    sb_led_setMask(0x01);
    wait_ms(500);

    sb_led_setMask(0x80);
    wait_ms(500);

    sb_led_setMask(0x00);
    wait_ms(500);

    for (uint8_t level = 0; level <= 8; level++) {
        result = sb_led_showLevel(level, 8);
        check_result(result);
        wait_ms(300);
    }

    sb_led_showLevel(5, 5);
    wait_ms(500);
    sb_led_showLevel(3, 5);
    wait_ms(500);
    sb_led_showLevel(1, 5);
    wait_ms(500);
    sb_led_showLevel(0, 5);
    wait_ms(500);

    result = sb_led_showLevel(5, 9);
    if (result != -1) while (1);

    result = sb_led_showLevel(6, 5);
    if (result != -1) while (1);

    while (1) {
        for (LED led = RED0; led <= BLUE1; led++) {
            sb_led_on(led);
            wait_ms(100);
            sb_led_off(led);
        }
        for (int led = BLUE1; led >= RED0; led--) {
            sb_led_on((LED)led);
            wait_ms(100);
            sb_led_off((LED)led);
        }
    }

    return 0;
}
