#include <stdint.h>// 标准整数类型（uint8_t, uint16_t等）
#include <avr/io.h>// AVR寄存器定义（DDRD, PORTD, EICRA等）
#include <avr/interrupt.h>// 中断相关（sei, cli, ISR）
//#include <avr/sleep.h>// 睡眠模式（实际没直接用，但为了完整性）
#include "led.h" // LED模块（sb_led_setMask, sb_led_on/off）
#include "7seg.h"// 7段数码管（sb_7seg_showNumber）
#include "timer.h"// 定时器模块（sb_timer_delay）
#define initial_speed 500
#define min_speed 50
#define step_delay 100
// 全局变量（volatile 理由：在中断中修改）

static volatile uint8_t g_button_pressed = 0;//uint8_t 数据类型 0~255 的无符号整数;static限制可见性,volatile禁止编译器优化,//g_button_pressed = 0把按钮按下标志清零

// 中断服务程序（最短实现）

ISR(INT0_vect) {//ISR 是定义中断服务程序的宏//INT0_vect是INT0 中断的中断向量名，告诉编译器"这个函数是处理 INT0 中断的//中断向量 = 中断函数的"门牌号"或"地址标签"//INT0_vect 是 INT0 中断的"门牌号"，当 PD2 按钮按下时，CPU 会自动跳转到你用 ISR(INT0_vect) 定义的函数。
    g_button_pressed = 1;   // 1按钮刚刚被按下了（有事件待处理）//0 没有按钮按下（初始状态）
}
// 初始化按钮中断
static void init_button_int0(void) {//init_button_int0这个函数负责初始化,自己写初始化函数.. BUTTON0 按钮的中断功能，让单片机能够检测到按钮按下。(void)这个函数不接受任何参数。

    //void没有返回值
    // PD2 设为输入
    DDRD &= ~(1 << PD2);//Data Direction Register D
    // 启用内部上拉电阻 &两个都是 1，结果才是 1；有一个是 0，结果就是 0。
    //DDRD 是控制 PD0~PD7 引脚方向的开关，1=输出，0=输入。按钮需要设为输入。
    PORTD |= (1 << PD2);
    //Port D Output Register PORTD 是 AVR 单片机中端口 D 的数据寄存器。
    //启用 PD2 引脚的内部上拉电阻。
    //| 只要有一个是 1，结果就是 1
    //控制上拉电阻（1=启用，0=禁用）
    //DDRD &= ~(1 << PD2);   // 第1步：PD2 设为输入
    //PORTD |= (1 << PD2);   // 第2步：启用上拉电阻 ← 就是这行！1=启用，0=禁用）
    EICRA &= ~(1 << ISC00);//ISC00 是宏，在 <avr/io.h> 中通常定义为 0
    //把 ISC00 位设为 0，为配置下降沿触发做准备。
    //#define ISC00   0   // EICRA 的第0位
    //#define ISC01   1   // EICRA 的第1位
    //1 << 0 = 1（没有移动  1 << 0 = 0b00000001
    //&= 是按位与赋值,有一个是 0，结果就是 0。，把 EICRA 的第0位清零，其他位不变。
    //EICRA External Interrupt Control Register A 外部中断控制寄存器 A 设置 INT0 和 INT1 的触发方式
    //位	名称	控制的中断
    //0	ISC00	INT0（低位）
    //1	ISC01	INT0（高位）
    //2	ISC10	INT1（低位）
    //3	ISC11	INT1（高位）
    //4-7	-	保留，不用

    //INT0 触发方式表
    //ISC01	ISC00	触发方式	效果
    //1	    0	   下降沿触发	按下瞬间触发一次

    //下降沿触发的配置
    //EICRA &= ~(1 << ISC00);    //ISC00 = 0
    //EICRA |= (1 << ISC01);    // ISC01 = 1
    //        EICRA 控制盒
    //          INT0                            INT1
    //ISC01 高位    ISC00低位           ISC11 高位   ISC10低位
    EICRA |= (1 << ISC01);//| 只要有一个是 1，结果就是 1
    // 启用 INT0 中断
    EIMSK |= (1 << INT0);
    //DDRD —— 决定按钮是"输入"还是"输出"
    //PORTD —— 安装"上拉电阻"（稳定电平）
    //EICRA —— 设置"门铃怎么响"
    //EIMSK —— 打开"门铃开关"

}


// 根据 LED 状态数组生成掩码

static uint8_t get_state_mask(const uint8_t led_state[]) {//const 承诺：这个函数不会修改数组的内容
    //uint8_t	数组元素类型（0~255 的整数）
    //led_state	参数的名字
    //[]	表示这是一个数组（不是单个数字）
    uint8_t mask = 0;//创建一个叫 mask 的变量，并把它初始化为 0
    for (uint8_t i = 0; i < 8; i++) {//for 循环，用来重复执行某段代码 8 次。
        if (led_state[i]) {// 如果第 i 个 LED 是亮的
            //led_state[i] 的值	含义
            //      0	       第 i 个 LED （灭）
            //      1	       第 i 个 LED 亮）
            //在 C 语言中，任何非 0 的值都被视为"真"，0 被视为"假"
            //if (led_state[i])    如果 LED 是亮的（值为 1），条件为真
            //if (!led_state[i])   如果 LED 是灭的（值为 0），条件为真
            mask |= (1 << i);// // 就把掩码的第 i 位设为 1
        }
    }
    return mask;
    //返回类型	   return 什么	  例子
    //uint8_t	 0~255 的整数	   return mask;
    //void	    什么都不返回	     不需要 return
    //int	      整数	        return 0;
}
// 创建光标掩码（在原有状态上翻转光标位置）
static uint8_t create_cursor_mask(const uint8_t led_state[], uint8_t cursor_pos) {
    //create_cursor_mask：生成一个"光标效果"的掩码，让光标所在的 LED 临时翻转。
    //const uint8_t led_state[]	参数1：LED 状态数组（只读）
    //
    uint8_t mask = get_state_mask(led_state);
    mask ^= (1 << cursor_pos);   // 翻转光标位置的 LED
    return mask;
}

// 检查是否所有 LED 都已点亮
static uint8_t is_all_lit(const uint8_t led_state[]) {
    for (uint8_t i = 0; i < 8; i++) {
        if (!led_state[i]) {
            return 0;
        }
    }
    return 1;
}

// 计算当前关卡的移动速度
// fl = a/l + b （参考文档第28页）
static uint16_t calculate_speed(uint8_t level) {



    // 速度随关卡增加而加快
    uint16_t speed = initial_speed / (1 + (level - 1) / 3);

    if (speed < min_speed) {
        speed = min_speed;
    }
    return speed;
}

// 游戏逻辑：玩一关
static void play(uint16_t speed) {
    uint8_t led_state[8] = {0};    // LED 状态（0=灭，1=亮）
    uint8_t cursor_pos = 0;         // 光标当前位置
    int8_t direction = 1;           // 移动方向（1=右，-1=左）
    uint8_t level_complete = 0;

    while (!level_complete) {
        // 1. 显示光标效果（临时翻转）
        sb_led_setMask(create_cursor_mask(led_state, cursor_pos));

        // 2. 被动等待（使用定时器，CPU睡眠）
        sb_timer_delay(speed);

        // 3. 恢复（取消光标效果）
        sb_led_setMask(get_state_mask(led_state));

        // 4. 检查按钮是否按下

        if (g_button_pressed) {
            g_button_pressed = 0;           // 清除标志
              // 如果这个LED还没亮
                led_state[cursor_pos] = !led_state[cursor_pos];  // 永久点亮
                sb_led_setMask(get_state_mask(led_state));

        }

        // 5. 检查是否通关
        if (is_all_lit(led_state)) {
            level_complete = 1;
            break;
        }

        // 6. 移动光标到下一个位置
        cursor_pos += direction;

        // 7. 边界处理（两端不等待两次）
        if (cursor_pos == 7) {
            direction = -1;   // 到头，往回走
        } else if (cursor_pos == 0) {
            direction = 1;    // 到头，往回走
        }
    }
}

// 胜利动画
static void show_win(void) {
    uint8_t i;


    // (a) LED7 → LED0 逐个熄灭，从 LED7 开始
    for (i = 7; i < 8; i--) {
        sb_led_off((LED)i);
        sb_timer_delay(step_delay);
        if (i == 0) break;
    }

    // (b) 光标从 LED7 移动到 LED0 再返回
    // 从 LED7 到 LED0
    for (i = 7; i < 8; i--) {
        sb_led_on((LED)i);
        sb_timer_delay(step_delay / 2);
        sb_led_off((LED)i);
        if (i == 0) break;
    }
    // 从 LED0 到 LED7
    for (i = 0; i <= 7; i++) {
        sb_led_on((LED)i);
        sb_timer_delay(step_delay / 2);
        sb_led_off((LED)i);
    }

    // (c) 从两端向中间点亮
    for (i = 0; i < 4; i++) {
        sb_led_on((LED)i);           // 左边
        sb_led_on((LED)(7 - i));     // 右边
        sb_timer_delay(step_delay);
    }

    // (d) 从中间向外熄灭
    for (int i = 3; i >= 0; i--) {
    sb_led_off((LED)i);
    sb_led_off((LED)(7 - i));
    sb_timer_delay(step_delay);
}

    // 确保所有 LED 熄灭
    sb_led_setMask(0x00);
}

// 主函数
int main(void) {
    uint8_t level = 1;
    uint16_t speed;

    // 1. 初始化按钮中断
    init_button_int0();

    // 2. 确保所有 LED 熄灭
    sb_led_setMask(0x00);

    // 3. 开启全局中断（数码管需要）
    sei();

    // 4. 显示初始关卡
    sb_7seg_showNumber(level);

    // 5. 游戏主循环
    while (1) {
        // 计算当前关卡的速度
        speed = calculate_speed(level);

        // 玩一关
        play(speed);

        // 胜利动画
        show_win();

        // 升级
        level++;

        // 显示新关卡
        sb_7seg_showNumber(level);
    }

    return 0;
}
