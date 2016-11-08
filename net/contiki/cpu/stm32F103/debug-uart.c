#include "debug-uart.h"
#include "basictype.h"

#include <string.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"


#define UART_BPS 	921600		/* Bit rate */

void dbg_setup_uart_default(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1|RCC_APB2ENR_AFIOEN ,ENABLE);	                

	//PA9 TX1 复用推挽输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//PA10 RX1 浮动输入
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = UART_BPS;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	//使能USART1
	USART_Cmd(USART1, ENABLE);	
}


int dbg_send_char (int ch) 
{
	while (!(USART1->SR & USART_FLAG_TXE)); // USART1 可换成你程序中通信的串口
	USART1->DR = (ch & 0x1FF);

	return (ch);
}


int dbg_get_key (void) 
{
	while (!(USART1->SR & USART_FLAG_RXNE));
	return ((int)(USART1->DR & 0x1FF));
}

void dgg_uart_putc ( u_char ch)
{
	while (!(USART1->SR & USART_FLAG_TXE)); // USART1 可换成你程序中通信的串口
	USART1->DR = (ch & 0x1FF);
}







