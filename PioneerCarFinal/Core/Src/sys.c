#include <stdio.h>
#include "sys.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//ϵͳ�жϷ������û�		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/10
//�汾��V1.4
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
//********************************************************************************  
//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
void WFI_SET(void) {
	__ASM volatile("wfi");
}
//�ر������ж�
void INTX_DISABLE(void) {
	__ASM volatile("cpsid i");
}
//���������ж�
void INTX_ENABLE(void) {
	__ASM volatile("cpsie i");
}
int fputc(int ch, FILE *f) {
	uint8_t data[]={ch};
	HAL_UART_Transmit(&huart1,data,1,100);
	return ch;
}
//����ջ����ַ
//addr:ջ����ַ
//__asm void MSR_MSP(u32 addr)
//{
//    MSR MSP, r0 			//set Main Stack value
//    BX r14
//}