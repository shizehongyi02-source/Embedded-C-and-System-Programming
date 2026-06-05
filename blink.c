#include <led.h>

// Simple delay function using active waiting
static void wait(uint32_t time){// wait function with parameter time

    volatile uint32_t count = 0;// VOLATILE TO PREVENT OPTIMIZATION


    while(count < time){ //WHILE U8<TIME  ADD 1 TO U8
        count++; //COUNTER STEPPING U8
    };
    }

void main(void){//void main	主函数，无返回值 (void) 不接受任何参数
    uint32_t u32 = 100000;//uint32 32 位无符号整数（0~42亿）,u32	变量名     尝试过 500、5000、50000、5000000   ,5000000 太大（等待时间太长）,最后选了 100000（约 500ms，取决于 CPU 速度）
    while (1) {//while(1) = 永远循环，程序不会退出
        sb_led_off(YELLOW1);//led1 off
        sb_led_on(YELLOW0); // led0 on
        wait(u32);         // 调用延时函数，等待 u32 次循环（约 500ms）保持 YELLOW0 亮 500ms
        sb_led_off(YELLOW0); // led0 off
        sb_led_on(YELLOW1); // led1 on
        wait(u32);    // call of  wait  function  to wait  500ms  after led1 iso       // COUNTER STE
    }
}
