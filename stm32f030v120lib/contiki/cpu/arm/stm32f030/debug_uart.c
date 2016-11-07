#include "stm32f0xx.h"
#include "debug_uart.h"

/*
* \brief Init the uart0
*/
void debug_uart_init(void)
{
	GPIO_InitTypeDef  gpioST;
	USART_InitTypeDef usartST;

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
}


/*
* \brief  send one byte from usart1
*/

void debug_send_one_byte(uint8_t ubch)
{
	while(!((USART1->ISR)&(1 << 7)));
	USART1->TDR = (ubch & (uint16_t)0x01FF);
}


/*
* \brief  send one byte from usart1
*/

void dbg_putchar(const char c)
{
	while(!((USART1->ISR)&(1 << 7)));
	USART1->TDR = (c & (uint16_t)0x01FF);
}

unsigned int dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
	unsigned int i = 0;
	for (i = 0; i < len; i++)
	{
		dbg_putchar((const char)seq[i]);
	}
	return len;
}



