#include "stm32f0xx.h"
#include "contiki.h"
#include "basictype.h"


/**
  * @brief UART Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */

void uart_init(u_char ubPort)
{
	USART_InitTypeDef usartST;
	GPIO_InitTypeDef gpioST;
	NVIC_InitTypeDef nvicST;
	if (ubPort > 2)
		return;
	if (1 == ubPort)
	{
		//io clock init 
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
		
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,GPIO_AF_1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10,GPIO_AF_1);
		
		gpioST.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10;
		gpioST.GPIO_Mode = GPIO_Mode_AF;
		gpioST.GPIO_PuPd = GPIO_PuPd_UP;
		gpioST.GPIO_OType = GPIO_OType_PP;
		gpioST.GPIO_Speed = GPIO_Speed_50MHz;
		
		GPIO_Init(GPIOA,&gpioST);
		
		//usart clock init
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		usartST.USART_BaudRate = 230400;
		usartST.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		usartST.USART_WordLength = USART_WordLength_8b;
		usartST.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
		usartST.USART_Parity = USART_Parity_No;
		usartST.USART_StopBits = USART_StopBits_1;
		
		USART_Init(USART1, &usartST);
		USART_Cmd(USART1, ENABLE);

		nvicST.NVIC_IRQChannel = USART1_IRQn;
		nvicST.NVIC_IRQChannelPriority = 0;
		nvicST.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&nvicST);
	}
}










