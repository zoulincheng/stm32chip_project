#ifndef _EM32_UART_H
#define _EM32_UART_H


typedef int(*uart_handler_t)(unsigned char);


void usart1_init(void);
int usart1_sendbyte(char c);



#endif



