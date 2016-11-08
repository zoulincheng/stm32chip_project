#include "contiki.h"
#include "basictype.h"
#include <string.h>
#include <stdio.h>
//替换头文件

#include "stm32f10x.h"
#include "uart_dma.h"

#include "sysprintf.h"

#if 0 
// the the dma uart  
#ifndef DBG_UART
#define DBG_UART USART1
#endif


#ifndef DBG_DMA_NO
#define DBG_DMA_NO 1
#endif

#ifndef DBG_DMA_CHANNEL_NO
#define DBG_DMA_CHANNEL_NO 4
#endif

#define _DBG_DMA_NAME(x) 			DMA##x
#define DBG_DMA_NAME(x) 			_DBG_DMA_NAME(x)
#define DBG_DMA 					DBG_DMA_NAME(DBG_DMA_NO)

#define _DMA_CHANNEL_NAME(x,c) 		DMA ## x ## _Channel ## c
#define DMA_CHANNEL_NAME(x,c) 		_DMA_CHANNEL_NAME(x,c)
#define DBG_DMA_CHANNEL 			 DMA_CHANNEL_NAME(DBG_DMA_NO, DBG_DMA_CHANNEL_NO)

#define _DBG_DMA_CHANNEL_IFCR_CGIF(c) 	DMA_IFCR_CGIF ## c
#define _XDBG_DMA_CHANNEL_IFCR_CGIF(c) 	_DBG_DMA_CHANNEL_IFCR_CGIF(c)
#define DBG_DMA_CHANNEL_IFCR_CGIF 		_XDBG_DMA_CHANNEL_IFCR_CGIF(DBG_DMA_CHANNEL_NO)

#ifndef DBG_XMIT_BUFFER_LEN
#define DBG_XMIT_BUFFER_LEN 512
#endif

static unsigned char xmit_buffer[DBG_XMIT_BUFFER_LEN];
#define XMIT_BUFFER_END &xmit_buffer[DBG_XMIT_BUFFER_LEN]


/* Valid data in head to tail-1 */
/* Read position */
static unsigned char * volatile xmit_buffer_head = xmit_buffer;

/* Write position */
static unsigned char * volatile xmit_buffer_tail = xmit_buffer;

/* xmit_buffer_head == xmit_buffer_tail means empty so we can only store
   DBG_XMIT_BUFFER_LEN-1 characters */
volatile unsigned char dma_running = 0;
static unsigned char * volatile dma_end;

void
dbg_setup_uart1_default(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA \
	                | RCC_APB2Periph_USART1|RCC_APB2ENR_AFIOEN ,ENABLE);

	                

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

	USART_InitStructure.USART_BaudRate = 921600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(DBG_UART, &USART_InitStructure);

#if 1
    //采用DMA方式发送  
    USART_DMACmd(DBG_UART,USART_DMAReq_Tx,ENABLE); 
#endif
	USART_ITConfig(DBG_UART, USART_IT_RXNE, ENABLE);
	//使能USART1
	USART_Cmd(DBG_UART, ENABLE);	
}



/**
  * @brief  Configures the DMA.
  * @param  None
  * @retval None
  */
void dbguart_DMA_Configuration(void)
{

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  

#if 1
  DBG_DMA_CHANNEL->CCR = (DMA_Priority_Low |
			  DMA_PeripheralDataSize_Byte |
			  DMA_MemoryDataSize_Byte |
			  DMA_PeripheralInc_Disable |
			  DMA_MemoryInc_Enable |
			  DMA_Mode_Normal |
			  DMA_DIR_PeripheralDST |
			  DMA_CCR4_TCIE
			  );
  DBG_DMA_CHANNEL->CPAR = (u32)&DBG_UART->DR;
  //DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer_head;
  DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer;
#endif

 
    DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);  
      
    //使能通道4  
    DMA_Cmd(DMA1_Channel4, ENABLE);  
}

void dbguart_DMA_NVIC_Config(void)
{
    //DMA发送中断设置  
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}






static void
update_dma(void)
{
	if (xmit_buffer_tail == xmit_buffer_head) return;
#if  1
	DBG_DMA_CHANNEL->CCR = (DMA_Priority_Low |
						DMA_PeripheralDataSize_Byte |
						DMA_MemoryDataSize_Byte |
						DMA_PeripheralInc_Disable |
						DMA_MemoryInc_Enable |
						DMA_Mode_Normal |
						DMA_DIR_PeripheralDST |
						DMA_CCR4_TCIE
	);
	DBG_DMA_CHANNEL->CPAR = (u32)&DBG_UART->DR;
	DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer_head;
//	DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer;
#endif
    //dbguart_DMA_Configuration( );
	//DBG_DMA_CHANNEL->CPAR = (u32)&DBG_UART->DR;
	//DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer_head;
	//dbguart_DMA_NVIC_Config( );
	if (xmit_buffer_head < xmit_buffer_tail) 
	{
		DBG_DMA_CHANNEL->CNDTR = xmit_buffer_tail - xmit_buffer_head;
		dma_end = xmit_buffer_tail;    
	} 
	else 
	{
		DBG_DMA_CHANNEL->CNDTR =  XMIT_BUFFER_END - xmit_buffer_head;
		dma_end = xmit_buffer;
		//dma_end = xmit_buffer_tail;    
	}
//	NVIC_ENABLE_INT(DMA1_Channel4_IRQn);
//	NVIC_SET_PRIORITY(DMA1_Channel4_IRQn, 2);
    	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);  

	DBG_DMA_CHANNEL->CCR |=DMA_CCR4_EN;
}


void
DMA1_Channel4_IRQHandler()
{
	DBG_DMA->IFCR = DBG_DMA_CHANNEL_IFCR_CGIF;
	xmit_buffer_head = dma_end;
	if (xmit_buffer_tail == xmit_buffer_head) 
	{
		dma_running = 0;
		return;
	}
	update_dma();
}



unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
	/* Since each of the pointers should be read atomically
	there's no need to disable interrupts */
	unsigned char *head = xmit_buffer_head;
	unsigned char *tail = xmit_buffer_tail;
	if (tail >= head) 
	{
		/* Free space wraps */
		unsigned int xfer_len = XMIT_BUFFER_END - tail; //free space at the tail
		unsigned int free = DBG_XMIT_BUFFER_LEN - (tail - head) - 1;//all free space 
		if (len > free) len = free;
		if (xfer_len < len) 
		{
			memcpy(tail, seq, xfer_len);
			seq += xfer_len;
			xfer_len = len - xfer_len;
			memcpy(xmit_buffer, seq, xfer_len);
			tail = xmit_buffer + xfer_len;
		} 
		else 
		{
			memcpy(tail, seq, len);
			tail += len;
			if (tail == XMIT_BUFFER_END) tail = xmit_buffer;
		}
	} 
	else 
	{
		/* Free space continuous */
		unsigned int free = (head - tail) - 1;
		if (len > free) len = free;
		memcpy(tail, seq, len);
		tail += len;
	}
	xmit_buffer_tail = tail;
	if (!dma_running) 
	{
		dma_running = 1;
		update_dma();
	}
	return len;
}

static unsigned char uart_dma_write_overrun = 0;

void uart_dma_putchar(u_char ch)
{
	#if 1
	if (uart_dma_write_overrun) 
	{
		//if (dbg_send_bytes((const unsigned char*)"^",1) != 1) return;
		if (dbg_send_bytes((const unsigned char*)"^",1) != 1) return;
	}
	#endif
	
	uart_dma_write_overrun = 0;
	
	if (dbg_send_bytes((const unsigned char*)&ch,1) != 1) 
	{ 
		uart_dma_write_overrun = 1;
	}
}

void uart_dma_blocking_putchar(const char ch)
{
	if (uart_dma_write_overrun) 
	{
		while (dbg_send_bytes((const unsigned char*)"^",1) != 1);
	}
	uart_dma_write_overrun = 0;
	while (dbg_send_bytes((const unsigned char*)&ch,1) != 1);
}

void dbg_drain()
{
  while(xmit_buffer_tail != xmit_buffer_head);
}

#else
// the the dma uart  
#ifndef DBG_UART
#define DBG_UART USART2
#endif


#ifndef DBG_DMA_NO
#define DBG_DMA_NO 1
#endif

#ifndef DBG_DMA_CHANNEL_NO
#define DBG_DMA_CHANNEL_NO 7
#endif

#define _DBG_DMA_NAME(x) 			DMA##x
#define DBG_DMA_NAME(x) 			_DBG_DMA_NAME(x)
#define DBG_DMA 					DBG_DMA_NAME(DBG_DMA_NO)

//DMA1_Channel4
#define _DMA_CHANNEL_NAME(x,c) 		DMA ## x ## _Channel ## c
#define DMA_CHANNEL_NAME(x,c) 		_DMA_CHANNEL_NAME(x,c)
#define DBG_DMA_CHANNEL 			 DMA_CHANNEL_NAME(DBG_DMA_NO, DBG_DMA_CHANNEL_NO)

#define _DBG_DMA_CHANNEL_IFCR_CGIF(c) 	DMA_IFCR_CGIF ## c
#define _XDBG_DMA_CHANNEL_IFCR_CGIF(c) 	_DBG_DMA_CHANNEL_IFCR_CGIF(c)
#define DBG_DMA_CHANNEL_IFCR_CGIF 		_XDBG_DMA_CHANNEL_IFCR_CGIF(DBG_DMA_CHANNEL_NO)

#ifndef DBG_XMIT_BUFFER_LEN
#define DBG_XMIT_BUFFER_LEN 32
#endif

static unsigned char xmit_buffer[DBG_XMIT_BUFFER_LEN];
#define XMIT_BUFFER_END &xmit_buffer[DBG_XMIT_BUFFER_LEN]


/* Valid data in head to tail-1 */
/* Read position */
static unsigned char * volatile xmit_buffer_head = xmit_buffer;

/* Write position */
static unsigned char * volatile xmit_buffer_tail = xmit_buffer;

/* xmit_buffer_head == xmit_buffer_tail means empty so we can only store
   DBG_XMIT_BUFFER_LEN-1 characters */
volatile unsigned char dma_running = 0;
static unsigned char * volatile dma_end;






/**
  * @brief  Configures the DMA.
  * @param  None
  * @retval None
  */
void dbguart_DMA_Configuration(void)
{

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  

#if 1
  DBG_DMA_CHANNEL->CCR = (DMA_Priority_Low |
			  DMA_PeripheralDataSize_Byte |
			  DMA_MemoryDataSize_Byte |
			  DMA_PeripheralInc_Disable |
			  DMA_MemoryInc_Enable |
			  DMA_Mode_Normal |
			  DMA_DIR_PeripheralDST |
			  DMA_CCR4_TCIE
			  );
  DBG_DMA_CHANNEL->CPAR = (u32)&DBG_UART->DR;
  //DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer_head;
  DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer;
#endif

    DMA_ITConfig(DBG_DMA_CHANNEL,DMA_IT_TC,ENABLE);       
    DMA_Cmd(DBG_DMA_CHANNEL, ENABLE);  
}

void dbguart_DMA_NVIC_Config(void)
{
    //DMA发送中断设置  
	NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
    NVIC_Init(&NVIC_InitStructure);  
}


static void
update_dma(void)
{
	if (xmit_buffer_tail == xmit_buffer_head) return;
#if  1
	DBG_DMA_CHANNEL->CCR = (DMA_Priority_Low |
						DMA_PeripheralDataSize_Byte |
						DMA_MemoryDataSize_Byte |
						DMA_PeripheralInc_Disable |
						DMA_MemoryInc_Enable |
						DMA_Mode_Normal |
						DMA_DIR_PeripheralDST |
						DMA_CCR4_TCIE
	);
	DBG_DMA_CHANNEL->CPAR = (u32)&DBG_UART->DR;
	DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer_head;
//	DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer;
#endif
    //dbguart_DMA_Configuration( );
	//DBG_DMA_CHANNEL->CPAR = (u32)&DBG_UART->DR;
	//DBG_DMA_CHANNEL->CMAR = (u32)xmit_buffer_head;
	//dbguart_DMA_NVIC_Config( );
	if (xmit_buffer_head < xmit_buffer_tail) 
	{
		DBG_DMA_CHANNEL->CNDTR = xmit_buffer_tail - xmit_buffer_head;
		dma_end = xmit_buffer_tail;    
	} 
	else 
	{
		DBG_DMA_CHANNEL->CNDTR =  XMIT_BUFFER_END - xmit_buffer_head;
		dma_end = xmit_buffer;
		//dma_end = xmit_buffer_tail;    
	}
//	NVIC_ENABLE_INT(DMA1_Channel4_IRQn);
//	NVIC_SET_PRIORITY(DMA1_Channel4_IRQn, 2);
    DMA_ITConfig(DBG_DMA_CHANNEL,DMA_IT_TC,ENABLE);  

	DBG_DMA_CHANNEL->CCR |=DMA_CCR4_EN;
}


void
DMA1_Channel7_IRQHandler()
{
	DBG_DMA->IFCR = DBG_DMA_CHANNEL_IFCR_CGIF;
	xmit_buffer_head = dma_end;
	if (xmit_buffer_tail == xmit_buffer_head) 
	{
		dma_running = 0;
		return;
	}
	update_dma();
}



unsigned int
dbg_send_bytes(const unsigned char *seq, unsigned int len)
{
	/* Since each of the pointers should be read atomically
	there's no need to disable interrupts */
	unsigned char *head = xmit_buffer_head;
	unsigned char *tail = xmit_buffer_tail;
	if (tail >= head) 
	{
		/* Free space wraps */
		unsigned int xfer_len = XMIT_BUFFER_END - tail; //free space at the tail
		unsigned int free = DBG_XMIT_BUFFER_LEN - (tail - head) - 1;//all free space 
		if (len > free) len = free;
		if (xfer_len < len) 
		{
			memcpy(tail, seq, xfer_len);
			seq += xfer_len;
			xfer_len = len - xfer_len;
			memcpy(xmit_buffer, seq, xfer_len);
			tail = xmit_buffer + xfer_len;
		} 
		else 
		{
			memcpy(tail, seq, len);
			tail += len;
			if (tail == XMIT_BUFFER_END) tail = xmit_buffer;
		}
	} 
	else 
	{
		/* Free space continuous */
		unsigned int free = (head - tail) - 1;
		if (len > free) len = free;
		memcpy(tail, seq, len);
		tail += len;
	}
	xmit_buffer_tail = tail;
	if (!dma_running) 
	{
		dma_running = 1;
		update_dma();
	}
	return len;
}

static unsigned char uart_dma_write_overrun = 0;

void uart_dma_putchar(u_char ch)
{
	#if 1
	if (uart_dma_write_overrun) 
	{
		//if (dbg_send_bytes((const unsigned char*)"^",1) != 1) return;
		if (dbg_send_bytes((const unsigned char*)"^",1) != 1) return;
	}
	#endif
	
	uart_dma_write_overrun = 0;
	
	if (dbg_send_bytes((const unsigned char*)&ch,1) != 1) 
	{ 
		uart_dma_write_overrun = 1;
	}
}

void uart_dma_blocking_putchar(const char ch)
{
	if (uart_dma_write_overrun) 
	{
		while (dbg_send_bytes((const unsigned char*)"^",1) != 1);
	}
	uart_dma_write_overrun = 0;
	while (dbg_send_bytes((const unsigned char*)&ch,1) != 1);
}

void dbg_drain()
{
  while(xmit_buffer_tail != xmit_buffer_head);
}


#endif

