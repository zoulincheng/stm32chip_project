#include "em32lg_config.h"

#include "basictype.h"
#include "sysprintf.h"
#include "em32_uart.h"



#include <string.h>

int uart0_send_char (int ch);


static uart_handler_t uart_stdhandler = NULL;
static uart_handler_t leuart_handler = NULL;






static u_char ubaRxToProcessBuf[128] = {0};
static u_char ubaCenter3762Buf[128] = {0};



void USART1_RX_IRQHandler(void)
{
	u_char ubrx;
	uart_handler_t handler = uart_stdhandler;
	
	//if (USART1->IF & USART_IF_RXDATAV)
	if (USART1->STATUS & USART_STATUS_RXDATAV)
	{
		ubrx = (u_char)USART_Rx(USART1);
		if (handler)
		{
			handler(ubrx);
		}
		USART_IntClear(USART1, USART_IF_RXDATAV);
	}
}


void LEUART0_IRQHandler(void)
{
	u_char ubrx;
 	uart_handler_t handler = leuart_handler;

	//if (LEUART0->STATUS & LEUART_STATUS_RXDATAV)
	if (LEUART0->IF & LEUART_IF_RXDATAV)
	{
		ubrx = (u_char)LEUART0->RXDATA;
		if (handler!=NULL)
		{
			handler(ubrx);
		}
	}	
}


//for debug
void usart1_init(void)
{
	USART_TypeDef           *usart = USART1;
	USART_InitAsync_TypeDef init   = USART_INITASYNC_DEFAULT;

	/* To avoid false start, configure output as high */
	GPIO_PinModeSet(US1_TX_PORT, US1_TX_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(US1_RX_PORT, US1_RX_PIN, gpioModeInput, 0);

	//config usart1 param
	USART_Reset(USART1);
	
	/* Configure USART for basic async operation */
	init.enable = usartDisable;
	init.refFreq = 0;
	init.baudrate = 115200;
	init.oversampling = usartOVS16;
	init.databits = usartDatabits8;
	init.parity = usartNoParity;
	init.stopbits = usartStopbits1;
	init.mvdis = false;
	init.prsRxEnable = false;
	init.prsRxCh = usartPrsRxCh0;
	
	USART_InitAsync(usart, &init);	
	
	usart->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN | USART_ROUTE_LOCATION_LOC1;

	/* Clear previous RX interrupts */
	USART_IntClear(USART1, _USART_IF_MASK);
	//USART_IntClear(USART1, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(USART1_RX_IRQn);

	/* Enable RX interrupts */
	USART_IntEnable(USART1, USART_IF_RXDATAV);
	NVIC_EnableIRQ(USART1_RX_IRQn);

	/* Finally enable it */
	USART_Enable(usart, usartEnable);
}

int usart1_sendbyte(char c)
{
	USART_Tx(USART1, c);
	return c;
}



uart_handler_t getUartStdHandler(void)
{
	return uart_stdhandler;
}

uart_handler_t getLeuartHandler(void)
{
	return leuart_handler;
}

void UartSetInputHandler(uart_handler_t dsthandler, uart_handler_t srchandler)
{
	dsthandler = srchandler;
}



void Uart_StdSetInput(uart_handler_t handler)
{
	/* store the setting */
	uart_stdhandler = handler;
	if(handler == NULL) 
	{
		//IE2 &= ~URXIE1;			/* Disable USART1 RX interrupt */
		//PRINTF("--------------------------------ERROR\r\n");
		
	} else 
	{
		//IE2 |= URXIE1;			/* Enable USART1 RX interrupt */
		//PRINTF("********************************RIGHT\r\n");
	}
}

void leuart0_init(void)
{
  LEUART_TypeDef      *leuart = LEUART0;
  LEUART_Init_TypeDef init    = LEUART_INIT_DEFAULT;

#if 0
#if defined(RETARGET_VCOM)
  /* Select HFXO/2 for LEUARTs (and wait for it to stabilize) */
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);
#else
  /* Select LFXO for LEUARTs (and wait for it to stabilize) */
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
#endif
#endif
  //CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_CORELEDIV2);
  CMU_ClockEnable(cmuClock_LEUART0, true);

  /* Do not prescale clock */
  CMU_ClockDivSet(cmuClock_LEUART1, cmuClkDiv_1);

  /* To avoid false start, configure output as high */
  GPIO_PinModeSet(LEU0_TX_PORT, LEU0_TX_PIN, gpioModePushPull, 1);
  GPIO_PinModeSet(LEU0_RX_PORT, LEU0_RX_PIN, gpioModeInput, 0);

  LEUART_Reset(leuart);
  /* Configure LEUART */
  init.enable = leuartDisable;
  init.baudrate = 9600;
#if defined(RETARGET_VCOM)
  init.baudrate = 115200;
#endif
  LEUART_Init(leuart, &init);
  /* Enable pins at default location */
  leuart->ROUTE = LEUART_ROUTE_RXPEN | LEUART_ROUTE_TXPEN | LEUART_ROUTE_LOCATION_LOC0;

  /* Clear previous RX interrupts */
  LEUART_IntClear(LEUART0, LEUART_IF_RXDATAV);
  NVIC_ClearPendingIRQ(LEUART0_IRQn);

  /* Enable RX interrupts */
  LEUART_IntEnable(LEUART0, LEUART_IF_RXDATAV);
  NVIC_EnableIRQ(LEUART0_IRQn);

  /* Finally enable it */
  LEUART_Enable(leuart, leuartEnable);
}

int uart0_send_char (int ch) 
{
	LEUART_Tx(LEUART0, (u_char)ch);
	return (ch);
}

int uart2_send_bytes(u_char *pBuf, u_char ubLength)
{
	int i = 0;
	for (i = 0; i < ubLength; i++)
	{
		uart0_send_char(pBuf[i]);
	}
	return 0;
}

static void uart_copy_rxdata(u_char *pBuf, const u_char *piBuf, u_char ubDataL)
{
	if ((NULL!= pBuf)&&(NULL != piBuf))
	{
		memcpy(pBuf, piBuf, ubDataL);
	}
}





//for 485
void Uart_485SetInput(uart_handler_t handler)
{
	/* store the setting */
	leuart_handler = handler;
	if(leuart_handler == NULL) 
	{
	} 
	else 
	{
	}
}

void serial_485_init(void)
{
	//Uart_485SetInput( );

}


