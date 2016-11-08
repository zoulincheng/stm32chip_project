#include "stm32f10x.h"
#include "iodef.h"






void sysinit(void)
{
	GPIO_InitTypeDef    tgGPIO_InitStru;
	NVIC_InitTypeDef    tgNVIC_InitStru;  

	//Periph clock enable 
	RCC_APB1PeriphClockCmd(
		  RCC_APB1Periph_USART2
		|RCC_APB1Periph_USART3
//		|RCC_APB1Periph_WWDG
//		|RCC_APB1Periph_UART4
//		|RCC_APB1Periph_UART5
//		|RCC_APB1Periph_BKP
		,ENABLE);
	//------------------------------------------------------------

	//------------------------------------------------------------
	//periph clk enable
	RCC_APB2PeriphClockCmd(
					  RCC_APB2Periph_GPIOA
					|RCC_APB2Periph_GPIOB
					|RCC_APB2Periph_GPIOC
					|RCC_APB2Periph_USART1
					|RCC_APB2Periph_AFIO
					, ENABLE);	
	// NVIC Configuration 
//	NVIC_DeInit( );
#ifdef  VECT_TAB_IN_RAM  
	// Set the Vector Table base location at 0x20000000  
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else// VECT_TAB_FLASH  
	// Set the Vector Table base location at 0x08000000  
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
#endif

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	// Enable the USART1 gloabal Interrupt 
	tgNVIC_InitStru.NVIC_IRQChannel = USART1_IRQn;
	tgNVIC_InitStru.NVIC_IRQChannelPreemptionPriority = 0;
	tgNVIC_InitStru.NVIC_IRQChannelSubPriority = 1;
	tgNVIC_InitStru.NVIC_IRQChannelCmd = ENABLE;//DISABLE;
	NVIC_Init(&tgNVIC_InitStru);

	/*config uart2 Interrupt*/
	tgNVIC_InitStru.NVIC_IRQChannel = USART2_IRQn;
	tgNVIC_InitStru.NVIC_IRQChannelPreemptionPriority = 0;
	tgNVIC_InitStru.NVIC_IRQChannelSubPriority = 2;
	tgNVIC_InitStru.NVIC_IRQChannelCmd = ENABLE;//DISABLE;
	NVIC_Init(&tgNVIC_InitStru);

	/*config uart3 Interrupt*/
	tgNVIC_InitStru.NVIC_IRQChannel = USART3_IRQn;
	tgNVIC_InitStru.NVIC_IRQChannelPreemptionPriority = 0;
	tgNVIC_InitStru.NVIC_IRQChannelSubPriority = 3;
	tgNVIC_InitStru.NVIC_IRQChannelCmd = ENABLE;//DISABLE;
	NVIC_Init(&tgNVIC_InitStru);



	//GPIO Config
	//USART PIN initialize
	//A9,A10 	TX1,RX1
	//A2,A3  	TX2,RX2	
	tgGPIO_InitStru.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_AF_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &tgGPIO_InitStru);

	tgGPIO_InitStru.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_10;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &tgGPIO_InitStru);

	//B10,B11 	TX3,RX3
	tgGPIO_InitStru.GPIO_Pin = GPIO_Pin_10;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_AF_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &tgGPIO_InitStru);
	tgGPIO_InitStru.GPIO_Pin = GPIO_Pin_11;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &tgGPIO_InitStru);


	// LED pin init
	//LED1 LED2 LED3 LED4 LED_G 
	tgGPIO_InitStru.GPIO_Pin = LED_SAT_1|LED_SAT_2|LED_SAT_3|LED_SAT_4;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &tgGPIO_InitStru);

	tgGPIO_InitStru.GPIO_Pin = LED_G;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &tgGPIO_InitStru);
	
	//485 direction pin control
	tgGPIO_InitStru.GPIO_Pin = TX_RX_DIR;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &tgGPIO_InitStru);

	//si4432 chip control pin interface
	tgGPIO_InitStru.GPIO_Pin = SI4432_NSEL;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &tgGPIO_InitStru);

	tgGPIO_InitStru.GPIO_Pin = SI4432_SDN;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
	tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &tgGPIO_InitStru);

	tgGPIO_InitStru.GPIO_Pin = SI4432_NIRQ|SI4432_GPIO_0;
	tgGPIO_InitStru.GPIO_Mode = GPIO_Mode_IPU;
	//tgGPIO_InitStru.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &tgGPIO_InitStru);

}




