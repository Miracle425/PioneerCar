#include "pstwo.h"
#include "usart.h"
typedef uint8_t u8;
typedef uint16_t u16;
/*********************************************************
Copyright (C), 2015-2025, YFRobot.
www.yfrobot.com
File：PS2驱动程序
Author：pinggai    Version:1.0     Data:2015/05/16
Description: PS2驱动程序
**********************************************************/
u16 Handkey;
u8 Comd[2] = {0x01, 0x42};    //开始命令。请求数据
u8 Data[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //数据存储数组

u16 MASK[] = {
	PSB_SELECT,
	PSB_L3,
	PSB_R3,
	PSB_START,
	PSB_PAD_UP,
	PSB_PAD_RIGHT,
	PSB_PAD_DOWN,
	PSB_PAD_LEFT,
	PSB_L2,
	PSB_R2,
	PSB_L1,
	PSB_R1,
	PSB_GREEN,
	PSB_RED,
	PSB_BLUE,
	PSB_PINK
};    //按键值与按键明

//手柄接口初始化    输入  DI->PB12 
//                  输出  DO->PB13    CS->PB14  CLK->PB15
void PS2_Init(void) {
////	//输入  DI->PB12
////	RCC->APB2ENR|=1<<3;     //使能PORTB时钟
////	GPIOB->CRH&=0XFFF0FFFF;//PB12设置成输入，默认下拉
////	GPIOB->CRH|=0X00080000;
//
////	//  DO->PB13    CS->PB14  CLK->PB15
////	RCC->APB2ENR|=1<<3;    //使能PORTB时钟
////	GPIOB->CRH&=0X000FFFFF;
////	GPIOB->CRH|=0X33300000;//PB13、PB14、PB15 推挽输出
//
//	GPIO_InitTypeDef GPIO_InitStructure;
//	//输入  DI->PB12
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);		//使能PORTB时钟
//
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 				 	//设置成上拉、下拉、浮空输入皆可
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
//
//	//输出  DO->PB13    CS->PB14  CLK->PB15
//	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 				//设置成推挽输出
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
/*
 *      0x01  0000 0001
 *            0000 0001
 *            000000001     1 &
 *
 *            0000 0001
 *            0000 0010
 *            0000 0000   0 & 1 = 0
 *            0000 0100
 * */


//向手柄发送命令
void PS2_Cmd(u8 CMD) {
	volatile u16 ref = 0x01;
	Data[1] = 0;
	for (ref = 0x01; ref < 0x0100; ref <<= 1) {
		if (ref & CMD) {
			DO_H;                   //输出以为控制位
		} else
			DO_L;

		CLK_H;                        //时钟拉高
		delay_us(50);
		CLK_L;
		delay_us(50);
		CLK_H;
		if (DI)
			Data[1] = ref | Data[1];
	}
}

//判断是否为红灯模式
//返回值；0，红灯模式
//		  其他，其他模式
u8 PS2_RedLight(void) {
	CS_L;

	PS2_Cmd(Comd[0]);  //开始命令
	PS2_Cmd(Comd[1]);  //请求数据
	CS_H;
	if (Data[1] == 0X73) return 0;
	else return 1;

}
//读取手柄数据
void PS2_ReadData(void) {
	volatile u8 byte = 0;
	volatile u16 ref = 0x01;

	CS_L;

	PS2_Cmd(Comd[0]);  //开始命令
	PS2_Cmd(Comd[1]);  //请求数据

	for (byte = 2; byte < 9; byte++)          //开始接受数据
	{
		for (ref = 0x01; ref < 0x100; ref <<= 1) {
			CLK_H;
			CLK_L;
			delay_us(50);
			CLK_H;
			if (DI)
				Data[byte] = ref | Data[byte];
		}
		delay_us(50);
	}
	CS_H;
}

//对读出来的PS2的数据进行处理      只处理了按键部分         默认数据是红灯模式  只有一个按键按下时
//按下为0， 未按下为1
u8 PS2_DataKey() {
	u8 index;

	PS2_ClearData();
	PS2_ReadData();

	Handkey = (Data[4] << 8) | Data[3];     //这是16个按键  按下为0， 未按下为1
	for (index = 0; index < 16; index++) {
		if ((Handkey & (1 << (MASK[index] - 1))) == 0)
			return index + 1;
	}
	return 0;          //没有任何按键按下
}

//得到一个摇杆的模拟量	 范围0~256
u8 PS2_AnologData(u8 button) {
	return Data[button];
}

//清除数据缓冲区
void PS2_ClearData() {
	u8 a;
	for (a = 0; a < 9; a++)
		Data[a] = 0x00;
}
//格式化打印
int rt_kprintf(const char *fmt, ...) {
	va_list args;
	uint32_t length = 0;
	static char rt_log_buf[RT_CONSOLEBUF_SIZE];

	va_start(args, fmt);
	/* the return value of vsnprintf is the number of bytes that would be
	 * written to buffer had if the size of the buffer been sufficiently
	 * large excluding the terminating null byte. If the output string
	 * would be larger than the rt_log_buf, we have to adjust the output
	 * length. */
	length = vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
	if (length > RT_CONSOLEBUF_SIZE - 1) {
		length = RT_CONSOLEBUF_SIZE - 1;
	}
	HAL_UART_Transmit(&huart1, rt_log_buf, length, 100);
	return length;
}


