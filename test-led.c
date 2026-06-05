#include "led.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

// ============================================
// 辅助函数
// ============================================

static void wait_ms(uint16_t ms)
{
    sb_timer_delay(ms);
}

static void check_result(int8_t result)
{
    if (result != 0) {
        // 测试失败：LED 快速闪烁
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

// ============================================
// 主函数
// ============================================

int main(void)
{
    int8_t result;

    // 开启全局中断（定时器需要）
    sei();

    // ========================================
    // Test 1: sb_led_on（逐个点亮）
    // ========================================
    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_on(led);
        check_result(result);
        wait_ms(200);
    }
    wait_ms(500);

    // 全部熄灭
    for (LED led = RED0; led <= BLUE1; led++) {
        sb_led_off(led);
    }

    // ========================================
    // Test 2: sb_led_off（逐个熄灭）
    // ========================================
    // 先全部点亮
    for (LED led = RED0; led <= BLUE1; led++) {
        sb_led_on(led);
    }
    wait_ms(500);

    // 再逐个熄灭
    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_off(led);
        check_result(result);
        wait_ms(200);
    }

    // ========================================
    // Test 3: sb_led_toggle（翻转两次）
    // ========================================
    // 第一次翻转（灭 → 亮）
    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_toggle(led);
        check_result(result);
        wait_ms(100);
    }
    wait_ms(500);

    // 第二次翻转（亮 → 灭）
    for (LED led = RED0; led <= BLUE1; led++) {
        result = sb_led_toggle(led);
        check_result(result);
        wait_ms(100);
    }

    // ========================================
    // Test 4: 无效 LED ID（应返回 -1）
    // ========================================
    result = sb_led_on((LED)(BLUE1 + 1));
    if (result != -1) while (1);

    result = sb_led_off((LED)(BLUE1 + 1));
    if (result != -1) while (1);

    result = sb_led_toggle((LED)(BLUE1 + 1));
    if (result != -1) while (1);

    result = sb_led_on((LED)255);
    if (result != -1) while (1);

    // ========================================
    // Test 5: sb_led_setMask
    // ========================================
    sb_led_setMask(0x00);  // 全灭
    wait_ms(500);

    sb_led_setMask(0xFF);  // 全亮
    wait_ms(500);

    sb_led_setMask(0x55);  // 交替亮（RED0, GREEN0, RED1, GREEN1）
    wait_ms(1000);

    sb_led_setMask(0xAA);  // 交替亮（YELLOW0, BLUE0, YELLOW1, BLUE1）
    wait_ms(1000);

    sb_led_setMask(0x01);  // 只亮 RED0
    wait_ms(500);

    sb_led_setMask(0x80);  // 只亮 BLUE1
    wait_ms(500);

    sb_led_setMask(0x00);  // 清理
    wait_ms(500);

    // ========================================
    // Test 6: sb_led_showLevel
    // ========================================
    // 有效参数测试（0~8 格）
    for (uint8_t level = 0; level <= 8; level++) {
        result = sb_led_showLevel(level, 8);
        check_result(result);
        wait_ms(300);
    }

    // 边界情况
    sb_led_showLevel(5, 5);
    wait_ms(500);
    sb_led_showLevel(3, 5);
    wait_ms(500);
    sb_led_showLevel(1, 5);
    wait_ms(500);
    sb_led_showLevel(0, 5);
    wait_ms(500);

    // 无效参数测试
    result = sb_led_showLevel(5, 9);   // max > 8
    if (result != -1) while (1);

    result = sb_led_showLevel(6, 5);   // level > max
    if (result != -1) while (1);

    // ========================================
    // Test 7: 流水灯效果（综合测试）
    // ========================================
    while (1) {
        // 从左到右
        for (LED led = RED0; led <= BLUE1; led++) {
            sb_led_on(led);
            wait_ms(100);
            sb_led_off(led);
        }
        // 从右到左
        for (int led = BLUE1; led >= RED0; led--) {
            sb_led_on((LED)led);
            wait_ms(100);
            sb_led_off((LED)led);
        }
    }

    return 0;
}
