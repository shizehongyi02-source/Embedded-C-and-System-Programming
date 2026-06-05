#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <7seg.h>
#include <led.h>

static void init(void){

 DDRD &=~(1<<PD2); //PD2 设为输入.    DDRD = Data Direction Register D（端口 D 的方向寄存器）1 → output 0: input.
 //&两个都是 1，结果才是 1；有一个是 0，结果就是 0。
 PORTD|=(1<<PD2);//启用上拉电阻.     （1=启用，0=禁用）{if pd2 input mode , 控制上拉电阻（1=启用，0=禁用）}
 EICRA&=~(1<<ISC00);//ISC00 = 0.  ，同时不影响其他位。INT0 和 INT1 是两个不同的外部中断，分别连接 BUTTON0（PD2）和 BUTTON1（PD3）。作业4只用到 INT0
//行代码把 EICRA 的第0位（ISC00）清零，为配置下降沿触发做准备，同时不影响其他位。
 EICRA|=(1<<ISC01);//ISC01 = 1.   {ISC01  1 , ISC00 = 0 下降沿触发（按下瞬间触发一次）}
 EIMSK|=(1<<INT0);//启用 INT0 , EIMSK = External Interrupt Mask Register（外部中断屏蔽寄存器）
}
static volatile uint8_t button_event= 0;//uint8_t IS 0~255 的无符号整数
ISR(INT0_vect){//ISR 是一个宏，它帮你自动生成正确的中断函数代码，INT0 INT0_vect中断向量名 <avr/interrupt.h>//中断服务函数的开头，告诉编译器："这个函数是 INT0 中断的处理程序"。

    button_event=1;//设置一个标志，告诉主程序"按钮被按下了"
}
void main (void){//让单片机能够：检测按钮按下（中断）进入省电模式（睡眠）被按钮唤醒
init();//配置按钮中断的硬件设置。 先安装门铃、布线、设置
sleep_enable();//作用：允许 CPU 进入睡眠模式。类比：把床铺好，允许自己睡觉。

sei();//作用：开启全局中断总开关。 类比：打开总电闸。

uint8_t counter =0;
sb_7seg_showNumber(counter);
while (1){

cli();
while(button_event !=1){
	 sei();
     sleep_cpu();
	 cli();


    }
	button_event=0;
	sei();


	counter = (counter + 1)%100;
	sb_7seg_showNumber(counter);
    }

}
