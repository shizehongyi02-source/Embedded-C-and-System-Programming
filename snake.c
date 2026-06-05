#include <stdint.h>//固定大小整数类型	uint8_t, int16_t, uint32_t
#include <avr/io.h>//AVR 寄存器	led.h / adc.h / button.h 需要
#include <avr/interrupt.h>//中断函数	代码调用 sei()
#include "led.h"//LED 控制	代码调用 sb_led_setMask()
#include "adc.h"//ADC 读取	sb_adc_read(PHOTO), sb_adc_read(POTI)
#include "button.h"//按钮检测	sb_button_getState(BUTTON0)

// ============================================
// 辅助函数 wait 等待函数（主动等待循环）
// 返回：1 = 需要切换模式，0 = 不需要
// ============================================
uint8_t wait(void) {
    // 1. 读取光敏电阻（0-1023）
    int16_t brightness = sb_adc_read(PHOTO);//读取光敏传感器的值，存储到 brightness 变量中。
    // PHOTO	告诉 ADC："我要读取光敏传感器".
    //int16_t	变量类型（16位有符号整数）
    //brightness	变量名  存储亮度值（0~1023）
    //sb_adc_read(PHOTO)	调用 ADC 模块读取光敏传感器. sb_adc_read 来自 adc.h。
    //sb_adc_read 是 ADC 模块提供的函数，用于读取模拟传感器的值
     //PHOTO
     // 光线越亮 → 传感器电阻越小 → 电压越高 → ADC 值越大
      //光线越暗 → 传感器电阻越大 → 电压越低 → ADC 值越小
    if (brightness < 0) brightness = 0;//防止 brightness 变成负数，确保它至少为 0。
    //返回类型是 int16_t（有符号 16 位整数），理论上可能返回 -1 表示错误。
    //0~1023	正常，ADC 测量值
    //-1	错误（无效设备 ID 等）
    //如果 brightness = -1,虽然不会崩溃，但会得到不正确的延时（比实际暗的情况还慢）。
    // 2. 计算等待时间
    // 亮度越高（越亮），等待时间越短（蛇越快）
    // delay = 20000 + (1023 - brightness) * 40
    // 亮度 0 → 约 60,000 次循环（慢）
    // 亮度 1023 → 约 20,000 次循环（快）
    uint32_t delay = 20000UL + (1023 - (uint32_t)brightness) * 40UL;//根据光线亮度计算延时的代码，用来控制贪吃蛇的移动速度。
    //uint32_t delay      延时变量（32位无符号整数）

    //delay = 20000 + (1023 - brightness) × 40
    //20000UL     基础延时（UL = Unsigned Long）
    //1023        ADC 最大值（光敏电阻的最大读数）
    //brightness  强制转换为 32 位无符号整数 当前亮度（0~1023）
    //40          放大系数（让光线对速度的影响更明显）

    //光线越亮 → brightness 越大 → (1023 - brightness) 越小 → delay 越小 → 蛇越快
    //光线越暗 → brightness 越小 → (1023 - brightness) 越大 → delay 越大 → 蛇越慢


    // 限制最小延时（防止太快看不见）
    //40UL	放大系数（UL = Unsigned Long（让光线对速度的影响更明显）
    //当前亮度（0~1023）brightness
    if (delay < 15000UL) {// // delay 太小（蛇太快）→ 强制设为 15000
        delay = 15000UL;//设置延时的最小值，防止蛇移动太快，人眼看不清。  15000UL 不是时间，而是循环次数
    }//这行代码是安全网，确保即使光线再亮，蛇也不会快到人眼无法识别。
    //if (delay < 15000)	15000 是 int，类型不匹配，可能有警告
    //if (delay < 15000UL)	15000UL 是 unsigned long，匹配

    // 3. 主动等待循环  wait() 函数中的变量声明和初始化部分。
    volatile uint32_t i;//delay 最大可达约 60,000，需要 32 位整数。
    //uint16_t  65,535
    //uint32_t  42亿
    uint8_t toggle = 0;           // 初始没有按钮按下，所以设为 0。
    //0	不需要切换模式
    //1	需要切换模式
    uint8_t last_state = 1;       // 上一次按钮状态（1=松开，0=按下）
    uint8_t button_pressed = 0;   // 是否已经处理过本次按下

    for (i = 0; i < delay; i++) {
        // 读取按钮状态
        BUTTONSTATE btn_state = sb_button_getState(BUTTON0);
        uint8_t current_state = (btn_state == PRESSED) ? 0 : 1;

        // 检测下降沿（松开 → 按下）
        if (!button_pressed && (last_state == 1) && (current_state == 0)) {
            toggle ^= 1;           // 翻转标志
            button_pressed = 1;    // 标记已处理
        }

        // 检测按钮松开，重置处理标志
        if (button_pressed && (current_state == 1)) {
            button_pressed = 0;
        }

        last_state = current_state;
    }

    return toggle;
}

// ============================================
// 辅助函数2 绘制蛇
// head: 蛇头位置（0-7）
// length: 蛇的长度（1-5）
// modus: 0 = 亮蛇暗背景，1 = 暗蛇亮背景
// ============================================
void drawSnake(uint8_t head, uint8_t length, uint8_t modus) {
    //head	    蛇头位置	0~7
    //length	蛇的长度	1~5
    //modus	   显示模式	    0 或 1
    uint8_t mask = 0;    //mask 是 8 位数字，每位控制一个 LED

    //初始 0b00000000（所有 LED 熄灭）
    //创建掩码变量

    // 计算蛇身的掩码
    for (uint8_t i = 0; i < length; i++) {//计算蛇身所有 LED 的位置
        uint8_t pos = (head + i) % 8;//计算蛇身每个 LED 的位置（0~7）根据蛇头位置和身体序号，计算出当前这一节身体应该在哪个 LED 上。
        //计算蛇身第 i 节应该在哪个 LED 上，支持环形循环。(head + i) % 8
        mask |= (1 << pos);//把掩码的第 pos 位设为 1
        //口诀：只要有一个是 1，结果就是 1
    }
     //模式0是"亮蛇"，模式1是"暗蛇"，按按钮可以在两种显示方式之间切换
    if (modus == 0) {
        // 亮蛇暗背景：蛇身用 1（点亮），背景用 0（熄灭）
        sb_led_setMask(mask);
    } else {
        // 暗蛇亮背景：蛇身用 0（熄灭），背景用 1（点亮）
        sb_led_setMask(~mask);
    }
}

// ============================================
//辅助函数3 计算蛇的长度（根据电位器）
// POTI 值 0-1023 → 映射到 1-5
// ============================================
static uint8_t get_snake_length(void) {//读取电位器（旋钮）的值，返回蛇的长度（1~5）
    //static	只在本文件内可用
    //uint8_t	返回值类型（0~255）
    //get_snake_length	函数名
    //(void)	没有参数
    int16_t poti = sb_adc_read(POTI);//poti 存储电位器的读数（0~1023），用来决定蛇的长度（1~5）。
    //sb_adc_read() 返回 int16_t
    //ADC 最大值 1023，在范围内  int16_t
    //在所有编译器上都是 16 位
    //poti 是一个变量名，存储电位器的读数。
    if (poti < 0) poti = 0;//防止 poti 变成负数，确保它至少为 0。

    // 分成 5 个等间隔
    if (poti < 205) return 1;
    if (poti < 410) return 2;
    if (poti < 615) return 3;
    if (poti < 820) return 4;
    return 5;
}//旋钮转得越多（poti 越大），蛇越长；旋钮转得越少（poti 越小），蛇越短。

// ============================================
// 主函数
// ============================================
int main(void) {
    uint8_t head = 0;      // 蛇头位置（从 LED0 开始）
    uint8_t length = 1;    // 蛇的长度
    uint8_t modus = 0;     // 显示模式（0=背景亮，1=背景暗）
    uint8_t need_toggle = 0;//// 是否需要切换模式
    //0	  不需要切换模式（保持当前模式）
    //1	  需要切换模式（从背景亮切换到背景暗，或反过来）
    //need_toggle = 0	门铃没响，不需要去开门
    //need_toggle = 1	门铃响了，需要去开门
    //初始状态	门铃没响，所以 = 0  必须初始化为 0，确保程序从正确的状态开始。



    //need_toggle = 0	不需要切换（假
    //need_toggle = 1	需要切换（真）
    //need_toggle = !need_toggle	翻转状态（0→1 或 1→0）

    //need_toggle = 0 表示"一开始没有按钮被按下，不需要切换模式"，这是程序的正确初始状态。


    // 开启全局中断（button 模块需要）
    sei();//开启中断

    while (1) {//无限循环
        // 1. 获取蛇的长度
        //电位器旋钮控制蛇的长度——转左边（小值）→ 蛇短，转右边（大值）→ 蛇长
        length = get_snake_length();// 步骤A：读取蛇的长度   get_snake_length根据电位器的值，决定蛇的长度（1~5）
        // 2. 绘制蛇
        drawSnake(head, length, modus);//绘制蛇

        // 3. 等待并检查是否需要切换模式  等待并检测按钮
        need_toggle = wait();// 等待 + 检测按钮
        //调用 wait() 函数，把返回值存到 need_toggle 变量中。
        //need_toggle = wait() 等待一段时间（根据光线），如果在此期间用户按了按钮，就标记"需要切换模式"，后续代码据此切换显示。
        //0（不切换）或 1（切换)



        //返回值	need_toggle 的值	       含义
        //0	      0	               没有按钮按下，不需要切换模式
        //1	      1	                按钮被按过，需要切换模式


        if (need_toggle) {//如果需要切换（按钮被按过）
            modus = !modus;   // 切换模式  把 modus 的值翻转（0变1，1变0）
        }//每一帧，程序读取蛇长，画蛇，根据光线等待，检测按钮，移动蛇头，然后重复——这就是游戏的核心循环。

        head = (head + 1) % 8;//让蛇头向前移动一个位置，在 8 个 LED 上循环。
        //head	当前蛇头位置（0~7）
        //head + 1	下一个位置
        //% 8  取余是为了让数值永远保持在 0~7 范围内。

    }

    return 0;
}
// 辅助函数（3个）
// wait()           - 延时 + 检测按钮
// drawSnake()      - 绘制蛇
// get_snake_length() - 读取电位器计算蛇长


//主函数 main()



//1. sei() 开启中断
// 2. while(1) 无限循环
// 获取蛇的长度
// 绘制蛇
//等待并检测按钮
//移动蛇头
