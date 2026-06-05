#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include "led.h"
#include "7seg.h"
#include "adc.h"

/*
 * ============================================
 * volatile 使用理由：
 * 以下变量在中断服务程序中被修改，而主程序需要读取它们。
 * volatile 防止编译器优化掉对它们的重复读取。
 * ============================================
 */
static volatile uint8_t g_button0_pressed = 0;
static volatile uint8_t g_button1_pressed = 0;
static volatile uint8_t g_timer_1s = 0;

// ============================================
// 状态机枚举
// ============================================
typedef enum {
    STATE_OFF,      // 关闭状态
    STATE_BOOT,     // 启动显示 "So"
    STATE_7SEG,     // 7段数码管模式（数码管亮，LED灭）
    STATE_LED       // LED灯带模式（LED亮，数码管灭）
} panel_state_t;

// ============================================
// 中断服务程序
// ============================================

// Button0 (PD2) - 下降沿触发 - 激活面板
ISR(INT0_vect) {
    g_button0_pressed = 1;
}

// Button1 (PD3) - 下降沿触发 - 切换模式
ISR(INT1_vect) {
    g_button1_pressed = 1;
}

// Timer0 溢出中断（约 15.6ms 一次）
ISR(TIMER0_OVF_vect) {
    static uint8_t tick_1s = 0;

    tick_1s++;
    if (tick_1s >= 64) {        // 64 × 15.6ms ≈ 1秒
        tick_1s = 0;
        g_timer_1s = 1;
    }
}

// ============================================
// 硬件初始化
// ============================================

static void init_buttons(void) {
    // Button0 (PD2) - INT0
    DDRD &= ~(1 << PD2);
    PORTD |= (1 << PD2);
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);
    EIMSK |= (1 << INT0);

    // Button1 (PD3) - INT1
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

    // 丢弃前几次读数，让 ADC 稳定
    for (int i = 0; i < 10; i++) {
        ADCSRA |= (1 << ADSC);
        while (ADCSRA & (1 << ADSC));
        (void)ADC;
    }
}

// ============================================
// 读取光敏传感器，返回百分比 (0-99)
// ============================================
static uint8_t get_percentage(void) {
    int16_t adc_value = sb_adc_read(PHOTO);
    if (adc_value < 0) adc_value = 0;

    uint32_t percent = ((uint32_t)adc_value * 100) / 1024;
    if (percent > 99) percent = 99;
    return (uint8_t)percent;
}

// ============================================
// 设置显示模式（互斥：只能一个亮）
// ============================================
static void set_mode(panel_state_t mode, uint8_t percentage) {
    if (mode == STATE_7SEG) {
        // 7SEG 模式：数码管亮，LED 全灭
        sb_7seg_showNumber(percentage);
        sb_led_setMask(0x00);
    } else if (mode == STATE_LED) {
        // LED 模式：LED 亮，数码管关闭
        uint8_t level = (percentage * 8) / 100;
        sb_led_showLevel(level, 8);
        sb_7seg_disable();
    }
}

// ============================================
// 关闭所有显示
// ============================================
static void all_disable(void) {
    sb_7seg_disable();
    sb_led_setMask(0x00);
}

// ============================================
// 主函数
// ============================================
int main(void) {
    panel_state_t state = STATE_OFF;
    uint8_t percentage = 0;
    uint8_t boot_timer = 0;
    uint8_t update_timer = 0;
    uint8_t inactivity_timer = 0;

    // 初始化硬件
    adc_init();
    init_buttons();
    init_timer0();
    sei();

    // 初始状态：全部关闭
    all_disable();

    while (1) {
        // ========== 处理定时器（每秒事件）==========
        if (g_timer_1s) {
            g_timer_1s = 0;

            if (boot_timer < 2) boot_timer++;
            if (update_timer < 2) update_timer++;
            if (inactivity_timer < 30) inactivity_timer++;
        }

        // ========== 处理 Button0（激活面板）==========
        if (g_button0_pressed) {
            g_button0_pressed = 0;
            if (state == STATE_OFF) {
                state = STATE_BOOT;
                sb_7seg_showString("So");
                boot_timer = 0;
                inactivity_timer = 0;
            }
        }

        // ========== 处理 Button1（切换模式）==========
        if (g_button1_pressed) {
            g_button1_pressed = 0;

            if (state == STATE_7SEG) {
                // 切换到 LED 模式
                state = STATE_LED;
                percentage = get_percentage();
                set_mode(state, percentage);
                inactivity_timer = 0;
            } else if (state == STATE_LED) {
                // 切换到 7SEG 模式
                state = STATE_7SEG;
                percentage = get_percentage();
                set_mode(state, percentage);
                inactivity_timer = 0;
            }
        }

        // ========== 状态机 ==========
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
                // 确保 LED 熄灭（互斥）
                sb_led_setMask(0x00);

                // 持续刷新数码管（保持显示）
                sb_7seg_showNumber(percentage);

                // 每秒更新数值
                if (update_timer >= 1) {
                    update_timer = 0;
                    percentage = get_percentage();
                    sb_7seg_showNumber(percentage);
                }

                // 15秒无操作自动关闭
                if (inactivity_timer >= 15) {
                    state = STATE_OFF;
                }
                break;

            case STATE_LED:
                // 确保数码管关闭（互斥）
                sb_7seg_disable();

                // 持续刷新 LED（保持显示）
                {
                    uint8_t level = (percentage * 8) / 99;
                    sb_led_showLevel(level, 8);
                }

                // 每秒更新数值
                if (update_timer >= 1) {
                    update_timer = 0;
                    percentage = get_percentage();
                    uint8_t level = (percentage * 8) / 99;
                    sb_led_showLevel(level, 8);
                }

                // 15秒无操作自动关闭
                if (inactivity_timer >= 15) {
                    state = STATE_OFF;
                }
                break;
        }
    }

    return 0;
}
