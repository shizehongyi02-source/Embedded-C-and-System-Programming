#include <led.h>

static void wait(uint32_t time){
    volatile uint32_t count = 0;
    while(count < time){
        count++;
    };
}

void main(void){
    uint32_t u32 = 100000;
    while (1){
        sb_led_off(YELLOW1);
        sb_led_on(YELLOW0);
        wait(u32);
        sb_led_off(YELLOW0);
        sb_led_on(YELLOW1);
        wait(u32);
    }
}
