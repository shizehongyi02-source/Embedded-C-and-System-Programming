#include "led.h"
#include <avr/io.h>

// volatile 使用理由：
// led_port 数组中存储的是硬件端口寄存器的地址。
// 这些寄存器可能被硬件或中断异步改变。
// volatile 防止编译器优化掉对这些寄存器的读写。

// 端口地址映射表
//DDRD	Data Direction Register D	设置方向（输入/输出）
static volatile uint8_t* const led_port[] = {//led_port[] 是一个端口地址映射表    端口地址
    &PORTD,  // RED0    (PD6)
    &PORTD,  // YELLOW0 (PD5)
    &PORTD,  // GREEN0  (PD4)
    &PORTD,  // BLUE0   (PD7)
    &PORTB,  // RED1    (PB0)
    &PORTB,  // YELLOW1 (PB1)
    &PORTC,  // GREEN1  (PC3)
    &PORTC   // BLUE1   (PC2)
    // for volatile :led_port[] 存的是硬件地址（可能被硬件改变），所以需要 volatile；led_pin[] 存的是普通数字（永远不会变），所以不需要 volatile。
};
//数组	        类型	    存的内容
//led_port[]	指针数组	存的是地址（指针）
//led_pin[]	    整数数组	存的是数字（引脚编号）
// uint8_t* 表示"指向 uint8_t 的指针"（存地址），uint8_t 表示"uint8_t 整数"（存数字），位置不同是因为定义的东西类型不同。。
// 引脚位映射表
// ============================================
static const uint8_t led_pin[] = {
    PD6,  // RED0    (位6)
    PD5,  // YELLOW0 (位5)
    PD4,  // GREEN0  (位4)
    PD7,  // BLUE0   (位7)
    PB0,  // RED1    (位0)
    PB1,  // YELLOW1 (位1)
    PC3,  // GREEN1  (位3)
    PC2   // BLUE1   (位2)
};
//LED	     端口	       引脚宏	实际值	位掩码
//RED0	      D	         PD6	6	1<<6 = 0b01000000
//YELLOW0	  D	         PD5	5	1<<5 = 0b00100000
//GREEN0	  D	         PD4	4	1<<4 = 0b00010000
//BLUE0	      D	         PD7	7	1<<7 = 0b10000000
//RED1	      B	         PB0	0	1<<0 = 0b00000001
//YELLOW1	  B	         PB1	1	1<<1 = 0b00000010
//GREEN1	  C	         PC3	3	1<<3 = 0b00001000
//BLUE1	      C	         PC2	2	1<<2 = 0b00000100
// ============================================
// 内部硬件初始化函数
// ============================================
static void led_hardware_init(void)//内部初始化函数，负责把 LED 引脚设置为输出模式。
{
    // 将所有 LED 引脚设为输出模式
    //1	输出 可以控制亮灭
    for (uint8_t i = 0; i < 8; i++) {// for 循环，用来重复执行某段代码 8 次
        //uint8_t i = 0	初始化：创建计数器 i，从 0 开始
        //i < 8	条件：只要 i < 8，就继续循环
        //i++	更新：每次循环结束后，i 加 1
        if (led_port[i] == &PORTD) {//if (led_port[i] == &PORTD) 检查第 i 个 LED 是否接在 PORTD 端口，以便设置对应的方向寄存器。
            //led_port[i]  第 i 个 LED 的端口地址
            //==           比较运算符

            //i	       LED	             led_pin[i]	     设置哪个位
            //0	       RED0	              PD6 = 6	     第6位
            //1	       YELLOW0	          PD5 = 5	     第5位
            //2	       GREEN0	          PD4 = 4	     第4位
            //3	       BLUE0	          PD7 = 7	     第7位
            //4	       RED1	              PB0 = 0	     第0位

            DDRD |= (1 << led_pin[i]);//把 PORTD 的某个引脚设为输出模式。
        } else if (led_port[i] == &PORTB) {
            DDRB |= (1 << led_pin[i]);
        } else if (led_port[i] == &PORTC) {
            DDRC |= (1 << led_pin[i]);
        }
    }//|有 1 就得 1，全 0 才是 0

    // 初始状态：所有 LED 熄灭（active-low: 高电平 = 熄灭）
    for (uint8_t i = 0; i < 8; i++) {
        *led_port[i] |= (1 << led_pin[i]);//熄灭一个 LED（设为高电平）。 熄灭 LED（设 1）


        // 点亮 LED（设 0）
        //*led_port[i] &= ~(1 << led_pin[i]);
//         先得到掩码，再取反

// 熄灭 LED（设 1）
//*led_port[i] |= (1 << led_pin[i]);
//         直接使用掩码

// 翻转 LED
//*led_port[i] ^= (1 << led_pin[i]);
//         直接使用掩码





        //led_pin[i] 是引脚位数组，存储每个 LED 对应的引脚编号。
        //static const uint8_t led_pin[] = {
        //PD6,  // RED0    → 6
        //PD5,  // YELLOW0 → 5
        //PD4,  // GREEN0  → 4
        //PD7,  // BLUE0   → 7
        //PB0,  // RED1    → 0
        //PB1,  // YELLOW1 → 1
        //PC3,  // GREEN1  → 3
        //PC2   // BLUE1   → 2
};
        //低电平(0V) → 电流从 VCC 流进单片机 → LED 亮
         //高电平(5V) → 两端都是 5V，没电流 → LED 灭


        //把指定引脚设为高电平（1），因为 LED 是 active-low，所以 LED 熄灭。
        //led_port[0] = &PORTD   // 存的是地址（门牌号）
        //*led_port[0] = PORTD   // 解引用，得到房子本身
        //|= 把某一位设为 1 → 高电平 → 熄灭
    }


// 点亮指定的 LED

int8_t sb_led_on(LED led)//sb_led_on 是一个公共函数（模块的接口），所以不能用 static。它有返回值，所以不能用 void。
//这是函数声明，告诉编译器：有一个名叫 sb_led_on 的函数，它接收一个 LED 类型的参数，返回一个 int8_t 类型的值。
{
    // 参数检查（枚举最小值是 0，只检查上限）
    if (led > BLUE1) {// 等价于 if (led > 7)
        return -1;// 返回 -1 表示错误
        //0	成功
        //-1	失败（无效的 LED ID）
    }
    //from led.h  typedef enum {
    //    RED0    = 0, /**< Upper red led    **/
    //    YELLOW0 = 1, /**< Upper yellow led **/
    //    GREEN0  = 2, /**< Upper green led  **/
    //    BLUE0   = 3, /**< Upper blue led   **/
    //    RED1    = 4, /**< Lower red led    **/
    //    YELLOW1 = 5, /**< Lower yellow led **/
    //    GREEN1  = 6, /**< Lower green led  **/
    //    BLUE1   = 7  /**< Lower blue led   **/
    // 延迟初始化（第一次调用时自动初始化）
    static uint8_t is_initialized = 0;//创建静态变量，初始为 0（未初始化）
    if (!is_initialized) {//如果没有初始化（值为 0）
        led_hardware_init();//执行硬件初始化
        is_initialized = 1;//标记为已初始化
    }

    // 点亮（active-low: 低电平点亮）
    *led_port[led] &= ~(1 << led_pin[led]);

    return 0;
}

// 熄灭指定的 LED
int8_t sb_led_off(LED led)//int8_t	返回值类型（0=成功，-1=失败）
{
    if (led > BLUE1) {
        return -1;
    }

    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    // 熄灭（active-low: 高电平熄灭）
    *led_port[led] |= (1 << led_pin[led]);

    return 0;
}

// 翻转指定的 LED

int8_t sb_led_toggle(LED led)
{
    if (led > BLUE1) {
        return -1;
    }

    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    // 翻转状态
    *led_port[led] ^= (1 << led_pin[led]);

    return 0;
}


// 用掩码设置所有 LED

void sb_led_setMask(uint8_t mask)
{
    // 第1部分：延迟初始化（只执行一次）
    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    // 第2部分：循环遍历 8 个 LED
    for (uint8_t i = 0; i < 8; i++) {
        //第3部分：判断并执行
        //if (mask & (1 << i))
        //位=1（点亮）              位=0（熄灭
        // *port &= ~(pin)       *port |= pin
        if (mask & (1 << i)) {//这是一个条件判断语句，用来检查 mask 的第 i 位是 1 还是 0
            //1 << i	生成只有第 i 位是 1 的掩码
            //1 << 0	0b00000001
            //1 << 1	0b00000010
            //1 << 2	0b00000100
            //1 << 3	0b00001000

            //mask & (1 << i)
            //mask 的第 i 位	   结果	   条件
            //      1	         1	     真（True）
            //      0	         0	     假（False）


            //假设 mask = 0b01010101
            //i	           1 << i	      mask & (1<<i)	     结果	     执行哪个分支
            //0	           0b00000001	0b00000001	         ≠ 0	if（点亮）
            //1	           0b00000010	0b00000000	         = 0	else（熄灭）
            //2	           0b00000100	0b00000100	         ≠ 0	if（点亮）
            //3	           0b00001000	0b00000000	         = 0	else（熄灭）
            //4	           0b00010000	0b00010000	         ≠ 0	if（点亮）
            //5	           0b00100000	0b00000000	         = 0	else（熄灭）
            // 6	       0b01000000	0b01000000	         ≠ 0	if（点亮）
            //7	           0b10000000	0b00000000	         = 0	else（熄灭）
            //
            // 这一位是 1 → 点亮 LED
            // active-low: 低电平点亮
            //& 口诀：两个都是 1，结果才是 1。
            *led_port[i] &= ~(1 << led_pin[i]);//把第 i 个 LED 对应的引脚设为 0（低电平），因为 LED 是低电平点亮，所以这个 LED 就亮了。
            //把第 i 个 LED 对应的引脚设为低电平（0），因为 active-low，所以 LED 点亮。


            //点亮用 &= ~，设 0 低电平。
            //熄灭用 |=，设 1 高电平。
            //Active-Low 要记牢，0 亮 1 灭不能倒。


            //led_port[0]	&PORTD（PORTD 的地址）
            //*led_port[0]	PORTD（解引用，得到寄存器本身）
            //led_port[0] = &PORTD   // 存的是 PORTD 的地址（门牌号）
            //*led_port[0] = PORTD   // 解引用，得到 PORTD 寄存器本身（房子）
            //led_pin[0]	PD6 = 6
            //1 << 6	0b01000000
            //~(1 << 6)	0b10111111
            //1 << led_pin[i] 生成一个"只有目标引脚位是 1"的掩码，用来单独控制那个 LED。
            //
        } else {
            // 这一位是 0 → 熄灭 LED
            // active-low: 高电平熄灭
            *led_port[i] |= (1 << led_pin[i]);//把第 i 个 LED 对应的引脚设为 1（高电平），因为 LED 是低电平点亮，所以这个 LED 就熄灭了
        }
    }
}

//mask = 0x55 = 0b01010101  检查 i = 2：
//mask:       0 1 0 1 0 1 0 1
//1 << 2:     0 0 0 0 0 1 0 0
//───────────────────────────
//AND:        0 0 0 0 0 1 0 0  ← 结果 ≠ 0，说明第2位是 1


//检查 i = 3：
//mask:       0 1 0 1 0 1 0 1
//1 << 3:     0 0 0 0 1 0 0 0
//───────────────────────────
//AND:        0 0 0 0 0 0 0 0  ← 结果 = 0，说明第3位是 0

//if (mask & (1 << i)) {
    // 这一位是 1 → 点亮
//} else {
    // 这一位是 0 → 熄灭
//}

// 显示进度/电量（前 level 个 LED 点亮）
int8_t sb_led_showLevel(uint8_t level, uint8_t max)//sb_led_showLevel(level, max) 把 LED 当作进度条，前 level 个 LED 点亮，总共使用前 max 个 LED。

//int8_t	返回值类型（0=成功，-1/-2=错误）
//uint8_t level	要点亮的 LED 数量
//uint8_t max	总共使用的 LED 数量
{
    // 参数检查
    if (max > 8) {
        return -1;  // 最多只有 8 个 LED
    }
    if (level > max) {
        return -1;  // level 不能超过 max
    }
    if (max == 0) {
        return -2;  // max 不能为 0（根据 led.h 文档）
        //max = 8	使用全部 8 个 LED 有效
        //max = 5	只使用前 5 个 LED 有效
        //max = 1	只使用 1 个 LED  有效
        //max = 0	不使用任何 LED	 无效  max = 0 没有意义，因为没有 LED 可以显示
    }
    //错误类型	       返回值
    //level > max	   -1
    //max > 8	       -1
    //max == 0	       -2

    ///**
 //* \return the number of LEDs turned on on success, negative value on error
 //*
 //* \retval >=0  success
 //* \retval -1   level exceeds max
 //* \retval -2   max is 0
 //*/

    // 延迟初始化
    static uint8_t is_initialized = 0;
    if (!is_initialized) {
        led_hardware_init();
        is_initialized = 1;
    }

    // 计算掩码：前 level 个 LED 点亮
    uint8_t mask = 0;//创建一个叫 mask 的变量，并把它初始化为 0
    //mask = 0 表示 0b00000000（所有 LED 熄灭）。
    //mask = 0;           // 0b00000000
    //mask |= (1 << 0);   // 0b00000001（点亮 RED0）
    //mask |= (1 << 1);   // 0b00000011（点亮 RED0, YELLOW0）
    //mask |= (1 << 2);   // 0b00000111（点亮 RED0, YELLOW0, GREEN0）
    for (uint8_t i = 0; i < level; i++) {
        //uint8_t i = 0	初始化：创建计数器 i，从 0 开始
        //i < level	条件：只要 i < level，就继续循环
        //i++	更新：每次循环结束后，i 加 1
        mask |= (1 << i);
    }//for (uint8_t i = 0; i < level; i++) 循环 level 次，i 从 0 到 level-1，用来设置前 level 个 LED。

    sb_led_setMask(mask);

    return 0;
}
