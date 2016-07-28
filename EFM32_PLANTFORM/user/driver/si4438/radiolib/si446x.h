#ifndef __SI446X_H_

#define __SI446X_H_


#define  PACKET_LENGTH      64 //0-64, if = 0: variable mode, else: fixed mode

void SI446X_SET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM, INT8U proerty );
void SI446X_RX_FIFO_RESET( void );
void SI446X_TX_FIFO_RESET( void );
u_char SI446X_GET_CUR_RSSI(void);
void SI446X_INT_STATUS( INT8U *buffer );
void SI446X_GET_MODEM_STATUS( U8 MODEM_CLR_PEND );
INT8U SI446X_GET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM );
void SI446X_GET_INT_STATUS(void);

#endif

