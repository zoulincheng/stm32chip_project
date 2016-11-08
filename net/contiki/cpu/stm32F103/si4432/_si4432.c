/*
 * Copyright (c) 2012, Thingsquare, http://www.thingsquare.com/.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * \file
 *        si4432 driver.
 */

#include "contiki.h"

/* Radio type sanity check */

#include "_si4432.h"
#include "_si4432_v2.h"

#include "net/packetbuf.h"
//#include "net/rime/rimestats.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#include <string.h>
#include <stdio.h>


#define BUSYWAIT_UNTIL(cond, max_time)                                  \
    do {                                                                  \
      rtimer_clock_t t0;                                                  \
      t0 = RTIMER_NOW();                                                  \
      while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time))) {/*printf(".");*/} \
    } while(0)


#define ACK_LEN 	3

#define SI4432_MAX_PAYLOAD 	127	

/* Flag indicating whether non-interrupt routines are using SPI */
static volatile uint8_t spi_locked = 0;
#define LOCK_SPI() do { spi_locked++; } while(0)
#define SPI_IS_LOCKED() (spi_locked != 0)
#define RELEASE_SPI() do { spi_locked--; } while(0)

/* CRC errors counter */
uint16_t si4432_crc_errors = 0;



/*
 * The buffers which hold incoming data.
 */
#ifndef RADIO_RXBUFS
#define RADIO_RXBUFS 4
#endif  /* RADIO_RXBUFS */

/* +1 because of the first byte, which will contain the length of the packet. */
/* Packet buffer for reception */
static uint8_t si4432_rxbufs[RADIO_RXBUFS][SI4432_MAX_PAYLOAD + 1];

#if RADIO_RXBUFS > 1
static volatile int8_t first = -1, last = 0;
#else   
static const int8_t first = 0, last = 0;
#endif 

#if RADIO_RXBUFS > 1
#define CLEAN_RXBUFS() do{first = -1; last = 0;}while(0)
#define RXBUFS_EMPTY() (first == -1)
int RXBUFS_FULL( )
{
	int8_t first_tmp = first;
	return first_tmp == last;
}
#else 
#define CLEAN_RXBUFS( ) (si4432_rxbufs[0][0] = 0)
#define RXBUFS_EMPTY( ) (si4432_rxbufs[0][0] == 0)
#define RXBUFS_FULL( ) (si4432_rxbufs[0][0] != 0)
#endif 
static uint8_t si4432_txbuf[SI4432_MAX_PAYLOAD + 1];
#define CLEAN_TXBUF() (si4432_txbuf[0] = 0)
#define TXBUF_EMPTY() (si4432_txbuf[0] == 0)


static volatile uint16_t packet_rx_len = 0;
static volatile uint8_t is_transmitting;
static int request_set_channel = -1;


/* Prototypes: */
static int si4432_rf_on(void);
static int si4432_rf_off(void);
static int si4432_rf_init(void);

static int si4432_rf_read(void *buf, unsigned short bufsize);
static int si4432_rf_send(const void *data, unsigned short len);
static int si4432_rf_prepare( const void *data, unsigned short len );
static int si4432_rf_transmit(unsigned short len);

static int si4432_rf_receiving_packet(void);
static int si4432_rf_pending_packet(void);
static int si4432_rf_channel_clear(void);
static signed char si4432_rssi_dbm(unsigned char temp) ;
static signed char si4432_read_rssi(void);
static void si4432_channel_set(unsigned char channel_number);

static int current_channel = 0;

#define MIN(a,b) ((a)<(b)?(a):(b))

//Flag for record si4432 interrupt
static volatile uint8_t si4432_ready = 0;
static volatile uint8_t tx_fifo_empty = 0;
static volatile uint8_t rx_fifo_full = 0;
static volatile uint8_t tx_packet_sent = 0;
volatile enum SI4432_STATE si4432_state = SI4432_IDLE;
volatile uint8_t ubAckData[3];
volatile uint8_t  ubACKFlag;


/*---------------------------------------------------------------------------*/
#define BUFSIZE 256
static struct 
{
	volatile u_char ubreceiving;
	u_char ubpacketlen;
	u_char ubptr;
	u_char ubabuf[BUFSIZE];
} rxstate;



#define		SI4432_FIFO_SIZE			(64)
#define		SI4432_FIFO_EMPTYTHD		(16)
#define		SI4432_FIFO_TXFULLTHD		(54)
#define		SI4432_FIFO_RXFULLTHD		(48)

#define  	SI4432_FIFO_TX_ALMOST_FULL_THD			54  //TX FIFO Almost Full Threshold
#define		SI4432_FIFO_TX_ALMOST_EMPTY_THD			(10)
#define		SI4432_FIFO_RX_ALMOST_FULL_THD			(48)


/*---------------------------------------------------------------------------*/
PROCESS(si4432_rf_process, "_si4432 driver");
/*---------------------------------------------------------------------------*/
PROCESS(si4432_rxled_process, "rx_led");
/*---------------------------------------------------------------------------*/
PROCESS(si4432_txled_process, "tx_led");
/*---------------------------------------------------------------------------*/

static radio_result_t si4432_get_value(radio_param_t param, radio_value_t *value)
{
	if(!value) 
	{
		return RADIO_RESULT_INVALID_VALUE;
	}
	switch(param) 
	{
		case RADIO_PARAM_CHANNEL:
			*value = current_channel;
			return RADIO_RESULT_OK;
		case RADIO_CONST_CHANNEL_MIN:
			*value = 0;
			return RADIO_RESULT_OK;
		case RADIO_CONST_CHANNEL_MAX:
			*value = 32;
			return RADIO_RESULT_OK;
		case RADIO_PARAM_RSSI:
			*value = si4432_read_rssi( );
			return RADIO_RESULT_OK;
		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}
}

static radio_result_t si4432_set_value(radio_param_t param, radio_value_t value)
{
	switch(param) 
	{
		case RADIO_PARAM_CHANNEL:
			if(value < 0 || value > 49) 
			{
				return RADIO_RESULT_INVALID_VALUE;
			}
			si4432_channel_set(value);
			return RADIO_RESULT_OK;
		default:
			return RADIO_RESULT_NOT_SUPPORTED;
	}
}

static radio_result_t si4432_get_object(radio_param_t param, void *dest, size_t size)
{
	return RADIO_RESULT_NOT_SUPPORTED;
}

static radio_result_t si4432_set_object(radio_param_t param, const void *src, size_t size)
{
	return RADIO_RESULT_NOT_SUPPORTED;
}

const struct radio_driver _si4432_driver = {
  si4432_rf_init,
  si4432_rf_prepare,
  si4432_rf_transmit,
  si4432_rf_send,
  si4432_rf_read,
  si4432_rf_channel_clear,
  si4432_rf_receiving_packet,
  si4432_rf_pending_packet,
  si4432_rf_on,
  si4432_rf_off,
  si4432_get_value,
  si4432_set_value,
  si4432_get_object,
  si4432_set_object,
};


#define SI4432_WRITE	0x80	

static void si4432_clear_rxstate(void)
{
	memset(&rxstate, 0, sizeof(rxstate));
}


//write a Byte data to the si4432 reg
static void si4432_write_reg(u8 ubReg, u8 ubValue)
{
	SI4432NSEL(LOW);
	SPI_SendByteData(ubReg|SI4432_WRITE);
	SPI_SendByteData(ubValue);
	SI4432NSEL(HIGH);
}

//read specify the reg 
static u_char si4432_read_reg(u8 ubReg)
{
	u_char ubRead = 0;	
	SI4432NSEL(LOW);
	SPI_SendByteData(ubReg);
	//SPI_SendByteData(WRITE_DUMMYDATA);
	ubRead = SPI_SendByteData(0xFF);
	SI4432NSEL(HIGH);

	return ubRead;
}


#define  SI4432_WAIT_CHIP_READY_TIME		240000
#define  SI4432_WAIT_PKSENT_TIME			240000
#define  SI4432_WAIT_TXFFAEM_TIME			240000

static int si4432_wait_ready(void)
{
  	clock_time_t timeout_time = 0;
  	
	while(!si4432_ready)
	{
		timeout_time++;
		if ( timeout_time > SI4432_WAIT_CHIP_READY_TIME)//time out
		{
			XPRINTF((0,"send error\n"));
			return 1;
		}
	}
	si4432_ready = 0;	
	return 0;
}

static int si4432_wait_pksent(void)
{
	clock_time_t timeout_time  = 0;

	while((!tx_packet_sent))
	{
		timeout_time++;
		if ( timeout_time > SI4432_WAIT_PKSENT_TIME)//time out
		{
			XPRINTF((0,"send error 1\n"));
			return 1;
		}
	}
	tx_packet_sent = 0;
	return 0;
}

//wait SI4432_entxffaem interrupt,finish a paket sent.
//for send data
static int si4432_wait_tx_fifo_empty(void)
{
	clock_time_t timeout_time  = 0;

	while((!tx_fifo_empty))
	{
		timeout_time++;
		if ( timeout_time > SI4432_WAIT_TXFFAEM_TIME )//time out
		{
			return 1;
		}
	}
	tx_fifo_empty = 0;
	return 0;
}


static u_char si4432_dev_stat(void)
{
	u_char ubDevSta;
	ubDevSta = si4432_read_reg(SI4432_DEVICE_STATUS);
	return ubDevSta;
}


//write muti datas to si4432,use to write data to FIFO
static void si4432_write_muti_data(const u8 *pcBuf, u_char ubDataLen)
{
	u_char i = 0;
	if (NULL == pcBuf)
	{
		return;
	}

	SI4432NSEL(LOW);
	SPI_SendByteData(SI4432_FIFO_ACCESS|SI4432_WRITE);
	for (i = 0; i < ubDataLen; i++)
	{
		SPI_SendByteData(pcBuf[i]);
		//SI4432WriteReg(SI4432_FIFO_ACCESS, pcBuf[i]);
	}
	SI4432NSEL(HIGH);
}

//read muti datas from si4432,use to read data from rx FIFO
static void si4432_read_muti_data(u8 *pcBuf, u_char ubDataLen)
{
	u_char i = 0;
	if (NULL == pcBuf)
	{
		return;
	}

	SI4432NSEL(LOW);
	for (i = 0; i < ubDataLen; i++)
	{
		pcBuf[i] = si4432_read_reg(SI4432_FIFO_ACCESS);
	}
	SI4432NSEL(HIGH);
}


//read si4432 device type(reg 0x00),device version(0x01)
//device status(0x02)
//This funtion is used for test SPI interface
static void si4432_read_id(void)
{
	u_char ubStatus = 0;

	ubStatus = si4432_read_reg(SI4432_DEVICE_TYPE);
	XPRINTF((0,"si4432 ID----------------------------------\r\n"));
	#if 1
	if(ubStatus == 0x08)
	{
		XPRINTF((0,"si4432 device type 0x00 is %02x\r\n", ubStatus));
	}
	else
	{
		XPRINTF((0,"Read device type 0x00 error\r\n"));
	}
	#endif
	#if 1
	ubStatus = si4432_read_reg(SI4432_DEVICE_VERSION);
	if((ubStatus & 0x06)== 0x06)
	{
		XPRINTF((0,"si4432 version code  0x01 is %02x\r\n", ubStatus));
	}
	else
	{
		XPRINTF((0,"Read version code 0x01 error\r\n"));
	}
	ubStatus = si4432_read_reg(SI4432_DEVICE_STATUS);
	XPRINTF((0,"si4432 device status 0x02 is %02x\r\n", ubStatus));
	#endif

	ubStatus = si4432_read_reg(SI4432_INTERRUPT_ENABLE_1);
	XPRINTF((0,"INTERRUPT_ENABLE_1 REG 0x05 is %x\r\n" , ubStatus));

	ubStatus = si4432_read_reg(SI4432_INTERRUPT_ENABLE_2);
	XPRINTF((0,"INTERRUPT_ENABLE_2 REG 0x06 is %x\r\n" , ubStatus));	
}

//soft reset si4432
//note reg 0x03 0x04 status is readed in the interrupt.
//This funtion is only send cmd to si4432
//软件复位
static void si4432_soft_reset(void)
{
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x80); 	//向0X07地址  写入0X80  软件复位
	//while(SI4432NIRQSTA == 1);		//wait for interrupt;
	si4432_wait_ready( );
}


//Turn on the radio by set PWRDN pin to zero,wait for some made the chip stable.
//note   
void si4432_turn_on(void)
{
	#ifdef UNUSED_NIRQ_EXIT
	u_char ubStatus1 = 0;
	u_char ubStatus2 = 0;
	#endif

	unsigned int delay = 0;
	SI4432SDN(HIGH);

	#if 0	
	{
		int i = 0;
		for(delay = 0; delay < 10000; delay++)
		{
			for(i = 0; i < 1000; i++);
		}
	}
	#endif
	clock_wait(40);
	SI4432SDN(LOW);
	clock_wait(40);
	
	#if 0
	{
		int i = 0;
		for(delay = 0; delay < 10000; delay++)
		{
			for(i = 0; i < 1000; i++);
		}
	}
	#endif
	
	#ifdef UNUSED_NIRQ_EXIT
	ubStatus1 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
	ubStatus2 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
	#endif
}

//Close  the radio by set PWRDN pin high.
//note   
static void si4432_turn_off(void)
{
	SI4432SDN(HIGH);
	clock_wait(30);//delay 30ms
}

static u_char si4432_get_EZMAC_state(void)
{
	u_char ubEZMACSta = 0;
	ubEZMACSta = si4432_read_reg(SI4432_EZMAC_STATUS);
	return ubEZMACSta;
}


//when receive data crc error, reset SI4432 RX FIFO
static void si4432_reset_rxfifo(void)
{

	u_char ubSta;
	//reset the RX FIFO
	//write 0x02 to the Operating Function Control 2 register
	ubSta = si4432_read_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
	ubSta = ubSta | 0x02;
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);
	ubSta = ubSta &(~0x02);
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x02);
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x00);//write 0x00 to the Operating Function Control 2 register	
	
}

// reset SI4432 TX FIFO
static void si4432_reset_txfifo(void)
{
	//reset the TX FIFO
	//write 0x02 to the Operating Function Control 2 register

	u_char ubSta;
	ubSta = si4432_read_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
	ubSta = ubSta | 0x01;
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);
	ubSta = ubSta &(~0x01);
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);

	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x01);
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x00);//write 0x00 to the Operating Function Control 2 register	
}

//Turn receiver OFF
static void si4432_stop_receive(void)
{
	// Turn Receiver OFF
	//SpiWriteRegister(0x07, 0x01);
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x01);
}


static void si4432_start_receive(void)
{
	// Turn Receiver ON
	//SpiWriteRegister(0x07, 0x04);
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x04);
	
}


static void si4432_disable_receive_chain(void)
{
	//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, SI4432_XTON | SI4432_PLLON);													//write 0x01 to the Operating Function Control 1 register			
	return;
}


static void si4432_enable_receive_chain(void)
{
	//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
	si4432_write_reg(0x07, 0x05);													//write 0x01 to the Operating Function Control 1 register			
	return;
}


//param ubFreq this need to commulate with data rate ,carrer freq and so on
//note FD need to be set with the other si4432 rf param 
static void si4432_set_fd(void)
{
	//The Tx deviation register has to set according to the deviation before every transmission (+-30kHz)
	si4432_write_reg(SI4432_FREQUENCY_DEVIATION, 0x48);//write  to the Frequency Deviation register
}


//set the TX paket data length
static void si4432_set_tx_packet_len(u_char ubDataLen)
{
	//set the length of the payload to 8bytes
	//SpiWriteRegister(0x3E, 128);//write  to the Transmit Packet Length register
	si4432_write_reg(SI4432_TRANSMIT_PACKET_LENGTH, ubDataLen);//write  to the Transmit Packet Length register
}


/*
Set Tx power
*/
static void si4432_set_tx_power(u_char dbm)
{
	u_char ubRegValue = 0;

	ubRegValue = si4432_read_reg(SI4432_TX_POWER);
	ubRegValue = ((~(0x07))&ubRegValue) | (dbm & 0x07);
	si4432_write_reg(SI4432_TX_POWER, ubRegValue);
}


/*
Get Rx signal strenth
Input receive power			RSSI value
-120dbm  					20
-100dbm						45
0dbm						230
120/(230-20) = 0.57/dbm   rssi value to dbm is  (value - 230 ) <<1
when receive power is larger 0dbm, RSSI value is 230,not change

specify the value is 0.5/dbm for  commulate
note this value more biger and the RSSI value more smller.
*/
/*
接收到信号强度指示（RSSI）提供一个在调谐信道上接收到的信号强度的测量。
读出RSSI的结果是0.5dB的增量,类似于一个高分辨率使能精确信道评估需要CCA
（清理信道评估）、CS（载波感应）和LBT（载波侦听）功能。
RSSI (接收信号强度指示)信号接收器打开了在信道的一个信号强度评估,
当使能同步字侦测在同步字已经侦测到后将会冻结RSSI的值，
当禁止同步字侦测或同步字没有侦测到，RSSI的值将连续不断的更新。
从一个8位的寄存器能读出每位精度是0.5dB的RSSI的值，总的127.5dB的RSSI的范围。
信道空闲评估门槛在寄存器可编程27h的rssith[7:0]，
在引导码RSSI评估后，如果在这个信道上无信号强度产生一个判断是在门槛以上或者以下。
如果信号强度在可编程的门槛以上那么一个“1”将出现在器件状态寄存器02h 的RSSI状态位上。
中断状态寄存器04h ，或配置GPIO 
*/
static u_char si4432_get_rssi(void)
{
	u_long udwRSSI = 0;

	udwRSSI = si4432_read_reg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR);

	return (u_char)((udwRSSI<<6)>>7);
}



//set  rssi threshold value
//note when set 80dbm,fact is -80dbm
static void si4432_set_rssi_threshold(u_char ubDbm)
{
	u_long ubRSSITH = 0;

	ubRSSITH = 230 - (ubDbm <<7 )>>6;

	si4432_write_reg(SI4432_RSSI_THRESHOLD, (u_char)ubRSSITH);
}

static u_char si4432_channel_set(u_char ubChannel)
{
	//SpiWriteRegister(0x76|0x80, ubChannel * 25);  // 需要跳频  2014 12 09
	si4432_write_reg(0x7a, ubChannel);  // 需要跳频  2014 12 09
    return 0;
}


// tx fifo always full 
static void si4432_set_tx_almost_full_thr(u_char ubTxafthr)
{
	if (ubTxafthr > SI4432_FIFO_SIZE)
	{
		return ;
	}

	si4432_write_reg(SI4432_TX_FIFO_CONTROL_1, ubTxafthr);
}


//tx always empty
static void si4432_set_tx_almost_empty_thr(u_char ubTxaethr)
{
	if (ubTxaethr < 1)
	{
		return;
	}
	si4432_write_reg(SI4432_TX_FIFO_CONTROL_2, ubTxaethr);
}


//rx always full
static void si4432_set_rx_almost_full_thr(u_char ubRXafthr)
{
	if (ubRXafthr > SI4432_FIFO_SIZE)
	{
		return;
	}
	si4432_write_reg(SI4432_RX_FIFO_CONTROL, ubRXafthr);
}

//when send a frame data, before send data, prepare
static void si4432_send_data_prepare(void)
{
	//set freq fd
	si4432_set_fd( );

	//SI4432ResetTXFIFO( );
	//SpiWriteRegister(0x07, 0x01);	//READY
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, SI4432_XTON|SI4432_PLLON);	//READY

	//SI4432WriteReg(0x0e|0x80, 0x02); // 发射状态的天线开关定义
	//SI4432OPENTXSWITCH;
}




static void si4432_rx_prepare(void)
{

	si4432_clear_rxstate( );
	
	si4432_reset_rxfifo( );
	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	// c) third shows if the RX fifo almost full: 'irxffafull
	//SpiWriteRegister(0x05, 0x13); 											//write 0x13 to the Interrupt Enable 1 register
	
	si4432_write_reg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENRXFFAFULL|SI4432_ENPKVALID|SI4432_ENCRCERROR); //write 0x13 to the Interrupt Enable 1 register
	//SpiWriteRegister(0x06, 0x00); 																	//write 0x00 to the Interrupt Enable 2 register
	si4432_write_reg(SI4432_INTERRUPT_ENABLE_2, SI4432_ENSWDET); 										//write 0x80 to the Interrupt Enable 2 register  rssi

	si4432_read_reg(SI4432_INTERRUPT_STATUS_1);
	si4432_read_reg(SI4432_INTERRUPT_STATUS_2);

		
#ifndef SI4432_B1
	//SpiWriteRegister(0x72, 0x1a);		//2014 12 09 disable			//write 0x1F to the Frequency Deviation register	9.6k	

#endif
	//SI4432CLOSESWITCH;//close tx rx switch
	/*enable receiver chain again*/
	//SpiWriteRegister(0x07, 0x05);
	si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, SI4432_XTON | SI4432_PLLON | SI4432_RXON);
	
}



/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
static int si4432_radio_send_packet(const u_char *pubdata)
{
	u_char ubDataNLen = 0;
//	u_char ubDataLen = 0;
//	u_char ItStatus1 = 0;
//	u_char ItStatus2 = 0;
	const u_char *pbuf = pubdata;
	int sdwSendTimeFlag = 0;
	int nresult = 0;
	u_char ubDevT = 0;

	ubDataNLen = pubdata[0] + 1;
	
	si4432_disable_receive_chain( );
	si4432_reset_txfifo( );
	//set paket len
	si4432_set_tx_packet_len( ubDataNLen );

	//si4432_send_data_prepare( );
#ifndef SI4432_B1	
	//SI4432WriteReg(SI4432_FREQUENCY_DEVIATION, 0x48);//write  to the Frequency Deviation register
#endif
	//Fill data to FIFO first,frame length < 64
	if(ubDataNLen < SI4432_FIFO_SIZE )
	{
		si4432_write_muti_data(pbuf, ubDataNLen);

		si4432_write_reg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENPKSENT);	// 整包数据发射完后，产生中断
		si4432_write_reg(SI4432_INTERRUPT_ENABLE_2, 0x00);
		si4432_read_reg(SI4432_INTERRUPT_STATUS_1);
		si4432_read_reg(SI4432_INTERRUPT_STATUS_2);
		
		//si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x09);  // 进入发射模式
		si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, SI4432_XTON|SI4432_PLLON|SI4432_TXON);  // 进入发射模式
		sdwSendTimeFlag = si4432_wait_pksent( );

		if (sdwSendTimeFlag)//send time out
		{
			nresult = -1;
			si4432_init( );
			goto rxprepare;
		}
	}
	else
	{
		si4432_write_muti_data( pbuf, SI4432_FIFO_SIZE);
		//Disable all other interrupts and enable the FIFO almost empty interrupt only.
		si4432_write_reg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENTXFFAEM);		//write 0x20 to the Interrupt Enable 1 register	
		si4432_write_reg(SI4432_INTERRUPT_ENABLE_2, 0x00);		//write 0x00 to the Interrupt Enable 2 register	
		si4432_read_reg(SI4432_INTERRUPT_STATUS_1);
		si4432_read_reg(SI4432_INTERRUPT_STATUS_2);

		/*enable transmitter*/
		//SpiWriteRegister(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x09);	//write 0x09 to the Operating Function Control 1 register
		si4432_write_reg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, SI4432_XTON|SI4432_PLLON|SI4432_TXON);  // 进入发射模式
		sdwSendTimeFlag = si4432_wait_tx_fifo_empty( );  //wait for interrupt
		if (sdwSendTimeFlag)//send time out
		{
			nresult = -1;
			si4432_init( );
			goto rxprepare;
		}
		
		ubDataNLen = ubDataNLen - SI4432_FIFO_SIZE;
		pbuf = pbuf + SI4432_FIFO_SIZE;

		while(ubDataNLen > SI4432_FIFO_TX_ALMOST_FULL_THD)
		{
			si4432_write_muti_data(pbuf, SI4432_FIFO_TX_ALMOST_FULL_THD);

			ubDataNLen = ubDataNLen - SI4432_FIFO_TX_ALMOST_FULL_THD;
			pbuf = pbuf + SI4432_FIFO_TX_ALMOST_FULL_THD;
			//clear flag
			si4432_read_reg(SI4432_INTERRUPT_STATUS_1);
			si4432_read_reg(SI4432_INTERRUPT_STATUS_2);
			sdwSendTimeFlag = si4432_wait_tx_fifo_empty( );
			if (sdwSendTimeFlag)//send time out
			{
				nresult = -1;
				si4432_init( );
				goto rxprepare;
			}
		}
		
		si4432_write_muti_data(pbuf, ubDataNLen);
		//Disable all other interrupts and enable the packet sent interrupt only.
		//This will be used for indicating the successfull packet transmission for the MCU
		si4432_write_reg(SI4432_INTERRUPT_ENABLE_1|0x80, SI4432_ENPKSENT);	// 整包数据发射完后，产生中断
		si4432_read_reg(SI4432_INTERRUPT_STATUS_1);
		si4432_read_reg(SI4432_INTERRUPT_STATUS_2);
		sdwSendTimeFlag = si4432_wait_pksent( );
		if (sdwSendTimeFlag)//send time out
		{
			nresult = -1;
			si4432_init( );
			goto rxprepare;
		}		
	}
	//timeout = 0;
	/*
	when send packet timeout or finish, go to rx state
	*/
	rxprepare:
	/*disable tx chain again*/
	//SpiWriteRegister(0x07, 0x01);														//write 0x05 to the Operating Function Control 1 register				

	//after packet transmission set the interrupt enable bits according receiving mode
	//Enable three interrupts: 
	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	// c) third shows if the RX fifo almost full: 'irxffafull
	//SpiWriteRegister(0x05, 0x13); 													//write 0x13 to the Interrupt Enable 1 register
	//SpiWriteRegister(SI4432_INTERRUPT_ENABLE_2, SI4432_ENSWDET); 											//write 0x80 to the Interrupt Enable 2 register  rssi
	//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
	//ItStatus1 = SpiReadRegister(0x03);												//read the Interrupt Status1 register
	//ItStatus2 = SpiReadRegister(0x04);												//read the Interrupt Status2 register

	/*enable receiver chain again*/
	//SpiWriteRegister(0x07, 0x05);														//write 0x05 to the Operating Function Control 1 register				
	si4432_rx_prepare( );
	return nresult;
}



//si4432 ISR service funtion
/* 
*/
/*
1. 使能EXTIx线的时钟和第二功能AFIO时钟
2. 配置EXTIx线的中断优先级
3. 配置EXTI 中断线I/O
4. 选定要配置为EXTI的I/O口线和I/O口的工作模式
5. EXTI 中断线工作模式配置
*/
#define SI4432_NIRQ_PORT	GPIOC
#define	SI4432_NIRQ_PIN		GPIO_Pin_6

static void si4432_nIRQ_config(void)
{
	GPIO_InitTypeDef gpioStr;
	EXTI_InitTypeDef extiStr;
	NVIC_InitTypeDef nvicStr;

	//clock Enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

	//nvic config for  exti interrupt
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );
	nvicStr.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvicStr.NVIC_IRQChannelPreemptionPriority =  0x00;
	nvicStr.NVIC_IRQChannelSubPriority = 0x00;
	//nvicStr.NVIC_IRQChannelPreemptionPriority =  0x03;
	//nvicStr.NVIC_IRQChannelSubPriority = 0x03;
	nvicStr.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStr);

	//EXTI line gpio conifg
	gpioStr.GPIO_Pin = SI4432_NIRQ_PIN;
	gpioStr.GPIO_Mode = GPIO_Mode_IPU;
	//gpioStr.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SI4432_NIRQ_PORT, &gpioStr);

	//EXTI line mode config
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);
	extiStr.EXTI_Line = EXTI_Line6;
	extiStr.EXTI_Mode = EXTI_Mode_Interrupt;
	extiStr.EXTI_Trigger = EXTI_Trigger_Falling;
	extiStr.EXTI_LineCmd = ENABLE;
	EXTI_Init( &extiStr);
}


//SI4432 reg value is must be set with the feq,speed,fd and so on
//these value can get from the si4432 excel
static void si4432_init(void)
{
	u_char ItStatus1 = 0;
	u_char ItStatus2 = 0;
	#if 1
	//spi interface config
	SPI_Config( );
	//si4432 power on
	si4432_turn_on( );

	si4432_read_id( );
	#endif

	/* ======================================================== *
	* Initialize the Si4432 ISM chip *
	* ======================================================== */
	//SW reset
	si4432_write_reg(0x07, 0x80);	//write 0x80 to the Operating & Function Control1 register
	//while ( SI4432NIRQSTA == 1);	//wait for chip ready interrupt from the radio (while the nIRQ pin is high)
	si4432_wait_ready( );
	//clock_wait(500);
	si4432_wait_ready( );
	
	#ifdef UNUSED_NIRQ_EXIT
	ItStatus1 = si4432_read_reg(0x03); //read the Interrupt Status1 register
	ItStatus1 = si4432_read_reg(0x04); //read the Interrupt Status2 register
	#endif
	XPRINTF((0,"INT_1\r\n"));
	si4432_write_reg(0x06|0x80, 0x00);  	//  关闭不需要的中断
	si4432_write_reg(0x07|0x80, SI4432_PWRSTATE_READY); // 进入 Ready 模式
	si4432_write_reg(0x09|0x80, 0x7f);  	//  负载电容= 12P

	si4432_write_reg(0x0a|0x80, 0x05);		// 关闭低频输出
	//si4432_write_reg(0x0b|0x80, 0xea); 		// GPIO 0 当做普通输出口
	//si4432_write_reg(0x0c|0x80, 0xea); 		// GPIO 1 当做普通输出口 control 
	//si4432_write_reg(0x0d|0x80, 0xea);  	// GPIO 2 当做普通输出口

	si4432_write_reg(0x69, 0x60);  //AGC过载	
	
	/*set the physical parameters*/
	//set the center frequency to 470 MHz
	si4432_write_reg(0x75, 0x57); 	//write  to the Frequency Band Select register
	si4432_write_reg(0x76, 0x00); 	//write  to the Nominal Carrier Frequency1 register
	si4432_write_reg(0x77, 0x00); 	//write  to the Nominal Carrier Frequency0 register

	//si4432_write_reg(0x75, 0x60); 	//write  to the Frequency Band Select register
	//si4432_write_reg(0x76, 0x19); 	//write  to the Nominal Carrier Frequency1 register


	//set the desired TX data rate (9.6kbps)
	si4432_write_reg(0x6E, 0x4E); //write  to the TXDataRate 1 register
	si4432_write_reg(0x6F, 0xA5); //write  to the TXDataRate 0 register
	si4432_write_reg(0x70, 0x2C);//10 1100 //write  to the Modulation Mode Control 1 register

	/*set the modem parameters according to the excel calculator
	(parameters: 9.6 kbps, deviation: 45 kHz, channel filterBW:112.1 kHz*/
	si4432_write_reg(0x1C, 0x05); //write  to the IF Filter Bandwidth register
	si4432_write_reg(0x20, 0xA1); //write  to the Clock Recovery Oversampling Ratio register
	si4432_write_reg(0x21, 0x20); //write  to the Clock Recovery Offset 2 register
	si4432_write_reg(0x22, 0x4E); //write  to the Clock Recovery Offset 1 register
	si4432_write_reg(0x23, 0xA5); //write  to the Clock Recovery Offset 0 register
	si4432_write_reg(0x24, 0x00); //write  to the Clock Recovery Timing Loop Gain 1 register
	si4432_write_reg(0x25, 0x13); //write  to the Clock Recovery Timing Loop Gain 0 register
	si4432_write_reg(0x1D, 0x40); //enable AFC
	//SpiWriteRegister(0x1D, 0x00); //enable AFC  20141209

	si4432_write_reg(0x72, 0x1F); //+-45kHz

	//SpiWriteRegister(0x72, 0x20); //+-45kHz

	si4432_write_reg(0x2a|0x80, 0x14);//AFC limiter
	//SpiWriteRegister(0x2a|0x80, 0x00);//AFC limiter  20141209

	/*set the packet structure and the modulation type*/
	si4432_write_reg(0x30|0x80, 0x8c);		// 使能PH+ FIFO模式，高位在前面，使能CRC校验
	si4432_write_reg(0x32|0x80, 0xff);		// byte0, 1,2,3 作为头码
	si4432_write_reg(0x33|0x80, 0x42);		// byte 0,1,2,3 是头码，同步字3,2 是同步字
	si4432_write_reg(0x34|0x80, 20);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte
	//si4432_write_reg(0x34|0x80, 24);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte //add 20141209 for more channels
	si4432_write_reg(0x35|0x80, 0x20);  	// 需要检测4个nibble的Preamble
	si4432_write_reg(0x36|0x80, 0x2d);  	// 同步字为 0x2dd4
	si4432_write_reg(0x37|0x80, 0xd4);
	si4432_write_reg(0x38|0x80, 0x00);
	si4432_write_reg(0x39|0x80, 0x00);
	si4432_write_reg(0x3a|0x80, 'j');   	// 发射的头码为： “"
	si4432_write_reg(0x3b|0x80, 'y');
	si4432_write_reg(0x3c|0x80, 'u');
	si4432_write_reg(0x3d|0x80, 'e');
	si4432_write_reg(0x3e|0x80, 10);    	// 总共发射10个字节的数据
	//si4432_write_reg(0x3e|0x80, 16);    	// 总共发射16个字节的数据 //12 byte + 2 byte + 4 Byte add 20141209 for more channels
	si4432_write_reg(0x3f|0x80, 'j');   	// 需要校验的头码为：”"
	si4432_write_reg(0x40|0x80, 'y');
	si4432_write_reg(0x41|0x80, 'u');
	si4432_write_reg(0x42|0x80, 'e');
	si4432_write_reg(0x43|0x80, 0xff);  // 头码1,2,3,4 的所有位都需要校验
	si4432_write_reg(0x44|0x80, 0xff);  // 
	si4432_write_reg(0x45|0x80, 0xff);  // 
	si4432_write_reg(0x46|0x80, 0xff);  // 


	//需要跳频时，设置0x79为物理信道编号，0x7a为信道间隔
	si4432_write_reg(0x79|0x80, 0x0);  // 不需要跳频
	si4432_write_reg(0x7a|0x80, 0x0);  // 不需要跳频
	//si4432_write_reg(0x79|0x80, 10);  // 需要跳频  2014 12 09
	//si4432_write_reg(0x7a|0x80, 20);  // 需要跳频  2014 12 09	
	
	si4432_write_reg(0x71|0x80, 0x23); // 发射不需要 CLK，FiFo ， GFSK模式

	//set the TX FIFO almost full threshold to 54 bytes
	si4432_write_reg(0x7C,SI4432_FIFO_TX_ALMOST_FULL_THD);	
	//set the TX FIFO almost empty threshold to 10 bytes
	si4432_write_reg(0x7D,SI4432_FIFO_TX_ALMOST_EMPTY_THD);	//write 0x0B to the TX FIFO Control 2 register
	//set the RX FIFO almost full threshold to 54 bytes
	si4432_write_reg(0x7E,SI4432_FIFO_RX_ALMOST_FULL_THD);	


	#if 1
	si4432_write_reg(0x0C, 0x15); //write 0x12 to the GPIO1 Configuration(set the TX state)
	si4432_write_reg(0x0D, 0x12); //write 0x15 to the GPIO2 Configuration(set the RX state)
	#endif

	/*set the non-default Si4432 registers*/
	//set the VCO and PLL
	si4432_write_reg(0x5A, 0x7F); //write 0x7F to the VCO Current Trimming register
	si4432_write_reg(0x58, 0x80); //write 0x80 to the ChargepumpCurrentTrimmingOverride register
	si4432_write_reg(0x59, 0x40); //write 0x40 to the Divider Current Trimming register
	//set the AGC
	si4432_write_reg(0x6A, 0x0B); //write 0x0B to the AGC Override 2 register
	//set ADC reference voltage to 0.9V
	si4432_write_reg(0x68, 0x04); //write 0x04 to the Deltasigma ADC Tuning 2 register
	si4432_write_reg(0x1F, 0x03); //write 0x03 to the Clock Recovery Gearshift Override register
	//set Crystal Oscillator Load Capacitance register
	si4432_write_reg(0x09, 0xD7); //write 0xD7 to the Crystal Oscillator Load Capacitance register

	//SpiWriteRegister(0x6d|0x80, 0x07);  // 设置为最大功率发射
	//SpiWriteRegister(0x6d|0x80, 0x06);  // 
	si4432_write_reg(0x6d|0x80, 0x18|0x07);  // 

	//set si4432 receive signal level 
	si4432_write_reg(SI4432_RSSI_THRESHOLD, 60);  //when rssi interrupt open, will generate  interrupt

	si4432_reset_txfifo( );

	//process_start(&si4432_send_check,NULL);
	XPRINTF((0,"INT Finish\r\n"));
//	SI4432SetChannel(0);//20141209
	si4432_rx_prepare( );//Receive mode, go to rx state
}

/*---------------------------------------------------------------------------*/
static void si4432_send_ack(uint8_t seqno)
{
	uint8_t len;
	uint8_t ackdata[ACK_LEN] = { 0, 0, 0};

	if((si4432_state & SI4432_RX_RECEIVING == SI4432_RX_RECEIVING)||(si4432_state & SI4432_TX == SI4432_TX)) 
	{
		/* Trying to send an ACK while transmitting - should not be
		possible, so this check is here only to make sure. */
		return;
	}
	ackdata[0] = FRAME802154_ACKFRAME;
	ackdata[1] = 0;
	ackdata[2] = seqno;
	len = ACK_LEN;
	/* Send packet */
	si4432_rf_send(ackdata, len);	
}


static void si4432_ack(void)
{
	//frame is not ack frame.
    if(rxstate.ubpacketlen > (ACK_LEN + 1)) 
	{
		/* Send a link-layer ACK before reading the full packet. */
		/* Try to parse the incoming frame as a 802.15.4 header. */
		frame802154_t info154;
		if(frame802154_parse((uint8_t *)(&rxstate.ubabuf[0] + 1), rxstate.ubpacketlen -1, &info154) != 0) 
		{
			/* XXX Potential optimization here: we could check if the
			frame is destined for us, or for the broadcast address and
			discard the packet if it isn't for us. */
			if(info154.fcf.frame_type == FRAME802154_ACKFRAME ||
			is_broadcast_addr(FRAME802154_SHORTADDRMODE,
			(uint8_t *)&info154.dest_addr) ||
			is_broadcast_addr(FRAME802154_LONGADDRMODE,
			(uint8_t *)&info154.dest_addr) ||
			linkaddr_cmp((linkaddr_t *)&info154.dest_addr,&linkaddr_node_addr)) 
			{
				/* For dataframes that has the ACK request bit set and that
				is destined for us, we send an ack. */
				if(info154.fcf.frame_type == FRAME802154_DATAFRAME && 
				info154.fcf.ack_required != 0 && 
				linkaddr_cmp((linkaddr_t *)&info154.dest_addr, &linkaddr_node_addr)) 
				{
					send_ack(info154.seq);
				}
			}
		}
	}
}


/*
radio inerface
*/

/*---------------------------------------------------------------------------*/
static int si4432_rf_on(void)
{
	return 1;
}




/*---------------------------------------------------------------------------*/
static int si4432_rf_off(void)
{
	return 1;
}



/*---------------------------------------------------------------------------*/
static int si4432_rf_init(void)
{
	si4432_init( );
	locked = 0;
	si4432_state = SI4432_IDLE;

	CLEAN_RXBUFS( );
	CLEAN_TXBUF( );

	//clean rxstate buf
	si4432_clear_rxstate( );
	//start radio process.

	return 0;
}


/*---------------------------------------------------------------------------*/
static int si4432_rf_prepare( const void *data, unsigned short len )
{
	if(len > SI4432_MAX_PACKET_LEN) 
	{
		XPRINTF((0,"SI4432: payload length=%d is too long.\r\n", len));
		return RADIO_TX_ERR;
	}
	/*
	* Copy to the txbuf. 
	* The first byte must be the packet length.
	*/
	CLEAN_TXBUF( );
	memcpy(si4432_txbuf+1, data, len);

	return RADIO_TX_OK;
}


/*---------------------------------------------------------------------------*/
static int si4432_rf_transmit(unsigned short len)
{
	u_long udwTimeOut =0; 
	int nResult = 0;
	//u_char ubCH = 0;
	si4432_txbuf[0] = len;  //data packet length

	while(((si4432_state & SI4432_RX_RECEIVING) == SI4432_RX_RECEIVING)||((si4432_state & SI4432_TX) == SI4432_TX))
	{	
		/* we are not transmitting. This means that
		we just started receiving a packet or sending a paket,
		so we drop the transmission. */        
		XPRINTF((0, "SEND DROP------------------------------\r\n"));       
		return RADIO_TX_COLLISION;
	}

	//when send packet failed or success, the si4432 go to rx state.
	si4432_state = SI4432_TX;
	if(si4432_radio_send_packet(si4432_txbuf) == SI4432_SUCCESS) 
	{
		
		si4432_state = SI4432_IDLE; 
		memset((u_char*)si4432_txbuf, 0, si4432_txbuf[0] + 1);//clear txbuf
		//MEM_DUMP(0, "TX->",(u_char*)si4432_txbuf, 128);
		process_post(&si4432_txled_process, tx_led_event, NULL);
		return RADIO_TX_OK;
	}
	else
	{
		/* TODO: Do we have to retransmit? */
		//CLEAN_TXBUF( );
		si4432_state = SI4432_IDLE; // when send packet, return IDLE state
		return RADIO_TX_ERR;
	}
}


/*---------------------------------------------------------------------------*/
static int si4432_rf_send(const void *data, unsigned short len)
{
	if (si4432_rf_prepare(data, len) == RADIO_TX_ERR) 
	{
    	return RADIO_TX_ERR;
    }
	return si4432_rf_transmit(len);
}


/*---------------------------------------------------------------------------*/
static int si4432_rf_read(void *buf, unsigned short bufsize)
{
	return read_from_rxbuf(buf, bufsize);
//	return 0;
}

/*---------------------------------------------------------------------------*/
static int si4432_rf_receiving_packet(void)
{
	if ((si4432_state & SI4432_RX_RECEIVING) == SI4432_RX_RECEIVING)
	{
		return 1;
	}
	else
		return 0;
}

/*---------------------------------------------------------------------------*/
static int si4432_rf_pending_packet(void)
{
	return !RXBUFS_EMPTY( );
}

/*---------------------------------------------------------------------------*/
static int si4432_rf_channel_clear(void)
{
  	if ((si4432_state&SI4432_RX_RECEIVING == SI4432_RX_RECEIVING)||(si4432_state&SI4432_TX == SI4432_TX))
  	{
  		return 0;//rf busy
  	}
	return 1;
}

/*---------------------------------------------------------------------------*/
static int add_to_rxbuf(uint8_t *src)
{
	if(RXBUFS_FULL()) 
	{
		return 0;
	}

	memcpy(si4432_rxbufs[last], src, src[0] + 1);//src[0] is data length in buf not include src[0], so all data length need to add 1 
	//memcpy(si4432_rxbufs[last], src, src[0]);
	//MEM_DUMP(0, "cp_rx", si4432_rxbufs[last], src[0]+1);	

	#if RADIO_RXBUFS > 1
	last = (last + 1) % RADIO_RXBUFS;
	if(first == -1) 
	{
		first = 0;
	}
	#endif
	
	#if 0
	{
		memcpy(&ubData[1],src, 3);
	}
	#endif
	
	memset(src, 0, src[0] + 1);//clear buf
	return 1;
}
/*---------------------------------------------------------------------------*/
static int read_from_rxbuf(void *dest, unsigned short len)
{
	if(RXBUFS_EMPTY()) 
	{          
		/* Buffers are all empty */
		return 0;
	}

	if(si4432_rxbufs[first][0] > len) 
	{   /* Too large packet for dest. */
		len = 0;
	} 
	else 
	{
		len = si4432_rxbufs[first][0];
		memcpy(dest, (uint8_t*)&si4432_rxbufs[first][0] + 1, len);
		packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
	}
	#if RADIO_RXBUFS > 1
	{
		OSCRITICAL cr;
		int first_tmp;
		OSInitCritical(&cr);
		OSEnterCritical(&cr);		
		first = (first + 1) % RADIO_RXBUFS;
		first_tmp = first;
		if(first_tmp == last) 
		{
			CLEAN_RXBUFS();
		}
		OSExitCritical(p);
	}
	#else
	CLEAN_RXBUFS();
	#endif

	return len;
}
/*---------------------------------------------------------------------------*/
static short last_packet_rssi()
{
  return last_rssi;
}


//io interrupt service funtion for si4432
/*lz在什么时候读取rssi值的，我现在在接收到数据后读取rssi的值，可靠吗？lz能否讲一下 
-------------------------------------------------------------------------------------------------------------------------------------
1、把06H设置为0X80使能同步子侦测。 
2、写个中断查询函数，如果检测到中断，则查询04H是否为0X80（侦测到同步字），如果为真，读26H即可。
*/
void EXTI9_5_IRQHandler(void)
{	
	//SI4432 interrupt state reg value.
	u_char ubIntSta1 = 0;
	u_char ubIntSta2 = 0;
	u_char ubRssi = 0;
	u_char ubEnIFlag1 = 0;
	u_char ubEnIFlag2 = 0;
	u_char ubInterruptFlag1 = 0;
	u_char ubInterruptFlag2 = 0;
		
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line6);	

		//get enable interrupt flag
		ubEnIFlag1 = SI4432ReadReg(SI4432_INTERRUPT_ENABLE_1);
		ubEnIFlag2 = SI4432ReadReg(SI4432_INTERRUPT_ENABLE_2);

		//clear interrupt flag and get interrupt flag
		ubIntSta1 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
		ubIntSta2 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);

		if ((ubIntSta2 &SI4432_ICHIPRDY == SI4432_ICHIPRDY) ||((ubIntSta2 &SI4432_IPOR == SI4432_IPOR)))
		{
			//this flag only for si4432 hardware reset and soft reset.
			si4432_ready = 1;
		}
		//get current interrupt flag
		ubInterruptFlag1 = ubIntSta1&ubEnIFlag1;
		ubInterruptFlag2 = ubIntSta2&ubEnIFlag2;
		//receive crc eeror, clear the si4432 rx fifo buf
		if ((ubInterruptFlag1 & SI4432_ICRCERROR) == SI4432_ICRCERROR)
		{
			si4432_rx_prepare( );
			si4432_state = SI4432_IDLE;
		}

		//sent tx
		//tx fifo almost empty flag
		if ((ubInterruptFlag1 & SI4432_ITXFFAEM) == SI4432_ITXFFAEM)
		{
			//set flag value
			tx_fifo_empty = 1;
		}
		//sent packet finish
		if ((ubInterruptFlag1 & SI4432_IPKSENT) == SI4432_IPKSENT)
		{
			tx_packet_sent = 1;
			si4432_state = SI4432_IDLE;
		}		

		//rx when receive data almost full, read data to buf.
		if ((ubInterruptFlag1 & SI4432_IRXFFAFULL) == SI4432_IRXFFAFULL)
		{			
			//SI4432ReadMutiData((u8 *)(&pRevPak->ubRxPaketData[0]+pRevPak->ubPaketLenFlag*SI4432RXFAFREAD), SI4432RXFAFREAD);
			si4432_read_muti_data((u8*)(&rxstate.ubabuf[0] + rxstate.ubptr), SI4432_FIFO_RXFULLTHD);
			rxstate.ubptr +=  SI4432_FIFO_RXFULLTHD;			
			//error data length, go to rx state again
			if (rxstate->ubabuf[0] > 128)
			{
				si4432_rx_prepare( );
				si4432_state = SI4432_IDLE;
			}
		}
		//rx Receive paket data finish.
		if ((ubInterruptFlag1 & SI4432_IPKVALID) == SI4432_IPKVALID)
		{
			si4432_write_reg(0x07, 0x01);//Disable receive			
			rxstate.ubpacketlen = si4432_read_reg(SI4432_RECEIVED_PACKET_LENGTH);
			if ((rxstate.ubpacketlen > rxstate.ubptr)&&(rxstate.ubpacketlen <= 128) &&(rxstate.ubpacketlen == (rxstate.ubabuf[0]+1)))
			{
				//SI4432ReadMutiData((u8 *)(&pRevPak->ubRxPaketData[0]+pRevPak->ubRevDataCnt), pRevPak->ubDataLen - pRevPak->ubRevDataCnt);
				si4432_read_muti_data((u8*)(&rxstate.ubabuf[0] + rxstate.ubptr), rxstate.ubpacketlen - rxstate.ubptr);
				if (rxstate.ubabuf[0] == 3)
				{
					ubACKFlag = 3;
					ubAckData[0] =  rxstate.ubabuf[1];
					ubAckData[1] =  rxstate.ubabuf[2];
					ubAckData[2] =  rxstate.ubabuf[3];
				}
				add_to_rxbuf(rxstate.ubabuf);
				process_poll(&si4432_radio_process);
			}			
			//when receive data failed and success, go to rx state
			si4432_rx_prepare( );
			si4432_state = SI4432_IDLE;
			//clear relate  flag when receive packet is right or not;
			rxstate.ubpacketlen = 0;
			rxstate.ubptr = 0;
			rxstate.ubabuf[0] = 0;
		}

		if ((ubIntSta2 & SI4432_ISWDET) == SI4432_ISWDET)//receive swdet
		{
			ubRxFlag = 1;
			SI4432_SET_OPSTATE(SI4432_RX_RECEIVING);
			ubRssi = SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR);
			last_rssi = -((int8_t)((230-ubRssi)>>1));
			//clear rxstate for new packet
			rxstate.ubpacketlen = 0;
			rxstate.ubptr = 0;
			rxstate.ubabuf[0] = 0;			
		}
	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(si4432_rf_process, ev, data)
{
	int len;

	PROCESS_BEGIN();
	XPRINTF((0,"stm32w_radio_process: started\r\n"));
	//rimeaddr_node_addr.u8[0] = 1;
	// rimeaddr_node_addr.u8[1] = 0;
	rx_led_event = process_alloc_event( );
	tx_led_event = process_alloc_event( );	
	//data_event = process_alloc_event( );
	process_start(&si4432_rxled_process,NULL);
	process_start(&si4432_txled_process,NULL);
	//process_start(&si4432_checkrx, NULL);
	//process_start(&si4432_rssi,NULL);
	//process_start(&si4432_rx,NULL);
	while(1) 
	{
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		//PROCESS_YIELD_UNTIL(ev == data_event);
		#if DEBUG > 1
		#endif

		packetbuf_clear();
		len = si4432_radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
		if(len > 0) 
		{
			MEM_DUMP(0, "RX<-",(u_char*)packetbuf_dataptr(), len);				
			packetbuf_set_datalen(len);

			NETSTACK_RDC.input();
			//process_poll(&si4432_rxled_process); 
			//ctimer_set(&rxCtimer, LED_STATE_TIME, rxLedHandler, (void *)LED_STATE_NUM);
			//process_post_synch(&si4432_rxled_process, rx_led_event, NULL);
			process_post(&si4432_rxled_process, rx_led_event, NULL);

		}
		if(!RXBUFS_EMPTY()) 
		{
			/*
			* Some data packet still in rx buffer (this happens because process_poll
			* doesn't queue requests), so stm32w_radio_process needs to be called
			* again.
			*/
			process_poll(&si4432_radio_process);
		}
	}
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/
//RX receive led state blue led
PROCESS_THREAD(si4432_rxled_process, ev, data)
{
	static struct etimer et;	
	static u_char ubLedCnt = 0;

	PROCESS_BEGIN();
	while (1)
	{
		PROCESS_YIELD_UNTIL(ev == rx_led_event);
		{
			etimer_set(&et, 10);
			LED2(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED2(0);
			etimer_set(&et, 10);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

			etimer_set(&et, 10);
			LED2(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED2(0);
		}
		
	}

	PROCESS_END();
}



//TX receive led state  red led
PROCESS_THREAD(si4432_txled_process, ev, data)
{
	static struct etimer et;	
	static u_char ubLedCnt = 0;

	PROCESS_BEGIN();

	while (1)
	{		
		PROCESS_YIELD_UNTIL(ev == tx_led_event);
		{
			etimer_set(&et, 10);
			LED3(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED3(0);
			etimer_set(&et, 10);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

			etimer_set(&et, 10);
			LED3(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED3(0);
		}
	}

	PROCESS_END();
}





//TX receive led state  red led
#define RX_TIMEOUT_CHECK	160
PROCESS_THREAD(si4432_rssi, ev, data)
{
	static struct etimer et;	
	//static float ftime;

	PROCESS_BEGIN();

	while (1)
	{		
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		//PROCESS_YIELD_UNTIL(ev == rssi_event);		
		//PROCESS_YIELD( );
		//XPRINTF((0, "last_rssi is %d dbm\r\n" , last_rssi));
		//ftime = ((float)ubRxCnt)*1.2;
		etimer_set(&et, RX_TIMEOUT_CHECK);	
		PROCESS_YIELD_UNTIL(etimer_expired(&et));
		if ((si4432_state&SI4432_RX_RECEIVING) == SI4432_RX_RECEIVING)
		{
			si4432_state &= ~(SI4432_RX_RECEIVING); 

			if (si4432_state&SI4432_TX != SI4432_TX)
			{
				SI4432RXPrepare( );
			}
			//XPRINTF((0, " receive time too long\r\n"));
		}
		//ftime = 0;
		//ubRxCnt = 0;
	}

	PROCESS_END();
}



/*---------------------------------------------------------------------------*/
