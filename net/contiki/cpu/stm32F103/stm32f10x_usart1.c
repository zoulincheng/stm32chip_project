/****************************************************************************
*  This file is part of the Ethernut port for the LPC2XXX
*
*  Copyright (c) 2005 by Michael Fischer. All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright 
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the 
*     documentation and/or other materials provided with the distribution.
*  3. Neither the name of the author nor the names of its contributors may 
*     be used to endorse or promote products derived from this software 
*     without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
*  THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
*  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF 
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
*  SUCH DAMAGE.
*
****************************************************************************
*
*  History:
*
*  24.09.05  mifi   First Version
*                   The CrossWorks for ARM toolchain will be used.
*  05.11.12  William Guo for STM32F10X application
*
****************************************************************************/
#include "contiki.h"
#include <string.h>
#include "stm32f10x_usart.h"
#include "basictype.h"
#include "stm32f10x_it.h"

#include "lib/ringbuf.h"
//#include "stm32f10x_usart1.h"
#include "list.h"

//#include "vfile.h"
//#include "device.h"
#include "userdev.h"
#include "uart.h"

#include "stm32f10x_it.h"
#include "sysprintf.h"







#define MAX_USART_PORT 5


//nPort: 0~4
static USART_TypeDef *uart_base[]={USART1,USART2,USART3,UART4,UART5};
static uart_handler_t uart_stdhandler = NULL;
static uart_handler_t uart_485handler = NULL;
static uart_handler_t uart_gprshandler = NULL;
static uart_handler_t uart_mp3handler = NULL;
static uart_handler_t uart_rfhandler = NULL;


const USART_TypeDef * UartGetBaseAddr(int nPort)
{
	if (nPort < 1 || nPort > 5)
		return NULL;
	return uart_base[(nPort-1)];
}




/*!
 * \brief Send a single character to debug device 0.
 *
 * A carriage return character will be automatically appended 
 * to any linefeed.
 */
static int UartOutput(int nPort,char ch)
{
	USART_TypeDef*USARTx = NULL;

	if(nPort < 1 || nPort >5)		
		return -1;
	USARTx = uart_base[nPort-1];
	
	while (!(USARTx->SR & USART_FLAG_TXE)); 
	USARTx->DR = (ch & 0x1FF);

	return 1;
}

static void uart_copy_rxdata(u_char *pBuf, const u_char *piBuf, u_char ubDataL)
{
	if ((NULL!= pBuf)&&(NULL != piBuf))
	{
		memcpy(pBuf, piBuf, ubDataL);
	}
}

void USART1_IRQHandler(void)
{
 	uart_handler_t handler = uart_stdhandler;
	USART_TypeDef *USARTx = (USART_TypeDef *)uart_base[0];
	u_long  csr = USARTx->SR;
	u_short  wRxDat;
//	u_char d;

	if (csr & USART_SR_RXNE) 
	{
		if (handler)
		{
			handler(USARTx->DR);
			//XPRINTF((0, "%02x", USARTx->DR));
		}
	}
	else if(csr&USART_FLAG_ORE)
	{ 	
		//溢出处理-如果发生溢出需要先清除ORE,再读DR寄存器 则可清除不断入中断的问题
		USARTx->SR = (u_short)~USART_FLAG_ORE;
		wRxDat = USARTx->DR;                                //读DR
		//if(USARTx != USART1)
		//	PRINTF((0,"uart%c USART_FLAG_ORE\r\n",dev->dev_name[4]));
	}	
}


void USART2_IRQHandler(void)
{
 	uart_handler_t handler = uart_485handler;
	USART_TypeDef *USARTx = (USART_TypeDef *)uart_base[1];
	u_long  csr = USARTx->SR;
	u_short  wRxDat;
	static u_char buf[128];
	static int ptr = 0;
	static u_char frameEnd = 0;
	
	if (csr & USART_SR_RXNE) 
	{
		u_char ubCH = (u_char)(USARTx->DR);		
		
		if (handler)
		{
			u_char ubCH = (u_char)(USARTx->DR);
			if (handler != NULL)
			{
				handler(ubCH);
			}
		}
	
	}
	else if(csr&USART_FLAG_ORE)
	{ 	
		//溢出处理-如果发生溢出需要先清除ORE,再读DR寄存器 则可清除不断入中断的问题
		USARTx->SR = (u_short)~USART_FLAG_ORE;
		wRxDat = USARTx->DR;                                //读DR
		//if(USARTx != USART1)
		//	PRINTF((0,"uart%c USART_FLAG_ORE\r\n",dev->dev_name[4]));
		//XPRINTF((10, "error "));
	}		
}

//for 485
void Uart_485SetInput(uart_handler_t handler)
{
	/* store the setting */
	uart_485handler = handler;
	if(handler == NULL) 
	{
	} 
	else 
	{
	}
}


//gprs
void Uart_GprsSetInput(uart_handler_t handler)
{
	/* store the setting */
	uart_gprshandler = handler;
	if(handler == NULL) 
	{
	} 
	else 
	{
	}
}


//mp3
void Uart_Mp3SetInput(uart_handler_t handler)
{
	/* store the setting */
	uart_mp3handler = handler;
	if(handler == NULL) 
	{
	} 
	else 
	{
	}
}



//rf
void Uart_RfSetInput(uart_handler_t handler)
{
	/* store the setting */
	uart_rfhandler = handler;
	if(handler == NULL) 
	{
	} 
	else 
	{
	}
}


void USART3_IRQHandler(void)
{
 	uart_handler_t handler = uart_rfhandler;
	USART_TypeDef *USARTx = (USART_TypeDef *)uart_base[2];
	u_long  csr = USARTx->SR;
	u_short  wRxDat;
	
	if (csr & USART_SR_RXNE) 
	{
		//XPRINTF((0, "%02x\r\n", (u_char)(USARTx->DR)));
		if (handler)
		{
			u_char ubCH = (u_char)(USARTx->DR);
			if (handler != NULL)
			{
				handler(ubCH);
			}
		}
	}
	else if(csr&USART_FLAG_ORE)
	{ 	
		//溢出处理-如果发生溢出需要先清除ORE,再读DR寄存器 则可清除不断入中断的问题
		USARTx->SR = (u_short)~USART_FLAG_ORE;
		wRxDat = USARTx->DR;                                //读DR
		//if(USARTx != USART1)
		//	PRINTF((0,"uart%c USART_FLAG_ORE\r\n",dev->dev_name[4]));
		//XPRINTF((10, "error "));
	}	
}


void UART4_IRQHandler(void)
{
 	uart_handler_t handler = uart_gprshandler;
	USART_TypeDef *USARTx = UART4;
	u_long  csr = USARTx->SR;
	u_short  wRxDat;

	if (csr & USART_SR_RXNE) 
	{
		//XPRINTF((0, "%02x\r\n", (u_char)(USARTx->DR)));
		if (handler)
		{
			u_char ubCH = (u_char)(USARTx->DR);
			if (handler != NULL)
			{
				handler(ubCH);
			}
		}
	}
	else if(csr&USART_FLAG_ORE)
	{ 	
		//溢出处理-如果发生溢出需要先清除ORE,再读DR寄存器 则可清除不断入中断的问题
		USARTx->SR = (u_short)~USART_FLAG_ORE;
		wRxDat = USARTx->DR;                                //读DR
		//if(USARTx != USART1)
		//	PRINTF((0,"uart%c USART_FLAG_ORE\r\n",dev->dev_name[4]));
		//XPRINTF((10, "error "));
	}		
}



void UART5_IRQHandler(void)
{
 	uart_handler_t handler = uart_mp3handler;
	USART_TypeDef *USARTx = UART5;
	u_long  csr = USARTx->SR;
	u_short  wRxDat;

	if (csr & USART_SR_RXNE) 
	{
		//XPRINTF((0, "%c\r\n", (u_char)(USARTx->DR)));
		if (handler)
		{
			u_char ubCH = (u_char)(USARTx->DR);
			if (handler != NULL)
			{
				handler(ubCH);
				//XPRINTF((0, "%c\r\n", (u_char)(USARTx->DR)));
			}
		}
	}
	else if(csr&USART_FLAG_ORE)
	{ 	
		//溢出处理-如果发生溢出需要先清除ORE,再读DR寄存器 则可清除不断入中断的问题
		USARTx->SR = (u_short)~USART_FLAG_ORE;
		wRxDat = USARTx->DR;                                //读DR
		//if(USARTx != USART1)
		//	PRINTF((0,"uart%c USART_FLAG_ORE\r\n",dev->dev_name[4]));
		//XPRINTF((10, "error "));
	}	
}


void Uart_Init(int nPort)
{
	//USART_InitTypeDef InitStru;
	USART_InitTypeDef stInitStru;
	USART_TypeDef *USARTx = NULL;

	if ( (nPort < 1)|| (nPort > 5))
		return;
	
	USARTx = uart_base[nPort-1];
	//default param for USART1
	if (1 == nPort)
	{
		//pInitStru = &InitStru;
		//USART_StructInit(pInitStru);
		stInitStru.USART_BaudRate = 921600;
		stInitStru.USART_WordLength = USART_WordLength_8b;
		stInitStru.USART_StopBits = USART_StopBits_1;
		stInitStru.USART_Parity = USART_Parity_No;
		stInitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		stInitStru.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	}
	else if (4 == nPort) //for gprs sim900a 115200
	{
		//pInitStru = &InitStru;
		//USART_StructInit(pInitStru);
		stInitStru.USART_BaudRate = 115200;
		stInitStru.USART_WordLength = USART_WordLength_8b;
		stInitStru.USART_StopBits = USART_StopBits_1;
		stInitStru.USART_Parity = USART_Parity_No;
		stInitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		stInitStru.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;		
	}
	else if (2 == nPort)
	{
		stInitStru.USART_BaudRate = 9600;
		stInitStru.USART_WordLength = USART_WordLength_9b;
		stInitStru.USART_StopBits = USART_StopBits_1;
		stInitStru.USART_Parity = USART_Parity_Even;
		stInitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		stInitStru.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;	
		XPRINTF((0, "UART2 init\n"));
	}
	else
	{	
		//default param for meter usart
		/*
		stInitStru.USART_BaudRate = 9600;
		stInitStru.USART_WordLength = USART_WordLength_9b;
		stInitStru.USART_StopBits = USART_StopBits_1;
		stInitStru.USART_Parity = USART_Parity_Even;
		stInitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		stInitStru.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
		*/
		stInitStru.USART_BaudRate = 9600;
		stInitStru.USART_WordLength = USART_WordLength_8b;
		stInitStru.USART_StopBits = USART_StopBits_1;
		stInitStru.USART_Parity = USART_Parity_No;
		stInitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		stInitStru.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;		
	}

	//Init USARTX
	USART_DeInit(USARTx);
	USART_Init(USARTx,&stInitStru);	

	//Open interrupt rx  add  st lib funtion
	/* Enable USARTy Receive and Transmit interrupts */
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);	

	#if 1
	if (2 == nPort )
	{
		#if 0
		//采用DMA方式发送  
		XPRINTF((10, "uart2 dma enable\r\n"));
		USART_DMACmd(USARTx,USART_DMAReq_Tx,ENABLE); 
		#endif
	}
	#endif
	USART_Cmd(USARTx,ENABLE);
}

void Uart_StdSetInput(uart_handler_t handler)
{
	/* store the setting */
	uart_stdhandler = handler;
	if(handler == NULL) 
	{
	} else 
	{
	}
}

void uart2_cfg(u_char ubIndex)
{
	//USART_InitTypeDef InitStru;
	USART_InitTypeDef stInitStru;
	if (ubIndex == 0x00)//9600
	{
		stInitStru.USART_BaudRate = 9600;
	}
	else if (ubIndex == 0x01)
	{
		stInitStru.USART_BaudRate = 19200;
	}
	else if (ubIndex == 0x02)
	{
		stInitStru.USART_BaudRate = 38400;
	}
	else if (ubIndex == 0x03)
	{
		stInitStru.USART_BaudRate = 57600;
	}
	else if (ubIndex == 0x04)
	{
		stInitStru.USART_BaudRate = 115200;
	}

	stInitStru.USART_WordLength = USART_WordLength_8b;
	stInitStru.USART_StopBits = USART_StopBits_1;
	stInitStru.USART_Parity = USART_Parity_No;
	stInitStru.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	stInitStru.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	USART_Init(USART2,&stInitStru);	
}







int uart2_send_char (int ch) 
{
	while (!(USART2->SR & USART_FLAG_TXE)); // USART1 可换成你程序中通信的串口
	USART2->DR = (ch & 0x1FF);

	return (ch);
}





int uart2_send_bytes(u_char *pBuf, u_char ubLength)
{
	int i = 0;
	
	for (i = 0; i < ubLength; i++)
	{
		uart2_send_char(pBuf[i]);
	}

	return 0;
}




void uart4_send_char(u_char ch)
{
	while (!(UART4->SR & USART_FLAG_TXE)); // 
	UART4->DR = (ch & 0x1FF);
	//XPRINTF((0, "u4 %02x\n", ch));
}

int uart4_send_bytes(u_char *pBuf, u_char ubLength)
{
	int i = 0;
	
	for (i = 0; i < ubLength; i++)
	{
		uart4_send_char(pBuf[i]);
	}
	
	return 0;
}


void uart5_send_char(u_char ch)
{
	while (!(UART5->SR & USART_FLAG_TXE)); // 
	UART5->DR = (ch & 0x1FF);
	//XPRINTF((0, "u4 %02x\n", ch));
}

int uart5_send_bytes(u_char *pBuf, u_char ubLength)
{
	int i = 0;
	
	for (i = 0; i < ubLength; i++)
	{
		uart5_send_char(pBuf[i]);
	}
	
	return 0;
}



