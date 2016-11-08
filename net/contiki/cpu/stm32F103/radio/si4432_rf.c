#include "contiki.h"

#include "net/mac/frame802154.h"

#include "net/netstack.h"

#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "sys/rtimer.h"

//#include "basictype.h"
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "iodef.h"
#include "basictype.h"
#include "atom.h"

#include "radio/si4432_v2.h"
#include "radio/si4432.h"
#include "radio/si4432_rf.h"


#include <string.h>

//#include <example\rime\rime_exsample.h>

#include "sysprintf.h"

/*---------------------------------------------------------------------------*/
static int si4432_radio_init(void);
static int si4432_radio_prepare(const void *payload,unsigned short payload_len);
static int si4432_radio_transmit(unsigned short payload_len);
int si4432_radio_send(const void *data, unsigned short len);
static int si4432_radio_read(void *buf, unsigned short bufsize);
static int si4432_radio_channel_clear(void);
static int si4432_radio_receiving_packet(void);
static int si4432_radio_pending_packet(void);
static int si4432_radio_on(void);
static int si4432_radio_off(void);
static int add_to_rxbuf(uint8_t * src);
static int read_from_rxbuf(void *dest, unsigned short len);
//static void si4432_interrupt_process(void);
static void send_ack(frame802154_t info154);


#define ACK_154_LEN		11



#ifndef MAC_RETRIES
#define MAC_RETRIES 4
#endif  /* MAC_RETRIES */

#if MAC_RETRIES
int8_t mac_retries_left;
#define INIT_RETRY_CNT() (mac_retries_left = packetbuf_attr(PACKETBUF_ATTR_MAX_MAC_TRANSMISSIONS))
#define DEC_RETRY_CNT() (mac_retries_left--)
#define RETRY_CNT_GTZ() (mac_retries_left > 0)
#else
#define INIT_RETRY_CNT()
#define DEC_RETRY_CNT()
#define RETRY_CNT_GTZ() 0
#endif /* MAC_RETRIES */


/* If set to 1, a send() returns only after the packet has been transmitted.
  This is necessary if you use the x-mac module, for example. */
#ifndef RADIO_WAIT_FOR_PACKET_SENT
#define RADIO_WAIT_FOR_PACKET_SENT 1
#endif  /* RADIO_WAIT_FOR_PACKET_SENT */

#define TO_PREV_STATE()       do {                                    \
                                if(onoroff == OFF){                   \
                                  ENERGEST_OFF(ENERGEST_TYPE_LISTEN); \
                                }                                     \
                              } while(0)

//#define MAC_RETRIES 0

/*
 * The buffers which hold incoming data.
 */
#ifndef RADIO_RXBUFS
#define RADIO_RXBUFS 10
#endif  /* RADIO_RXBUFS */

/* +1 because of the first byte, which will contain the length of the packet. */
// +1 last data is rssi
//len1 [data] rssi1
static uint8_t si4432_rxbufs[RADIO_RXBUFS][SI4432_MAX_PACKET_LEN + 1 + 2];//len1 [data] rssi1  last_rssi


#if RADIO_RXBUFS > 1
static volatile int8_t first = -1, last = 0;
#else   /* RADIO_RXBUFS > 1 */
static const int8_t first = 0, last = 0;
#endif  /* RADIO_RXBUFS > 1 */

#if RADIO_RXBUFS > 1
#define CLEAN_RXBUFS() do{first = -1; last = 0;}while(0)
#define RXBUFS_EMPTY() (first == -1)

static int RXBUFS_FULL( )
{
	int8_t first_tmp = first;
	return first_tmp == last;
}

#else /* RADIO_RXBUFS > 1 */
#define CLEAN_RXBUFS( ) (si4432_rxbufs[0][0] = 0)
#define RXBUFS_EMPTY( ) (si4432_rxbufs[0][0] == 0)
#define RXBUFS_FULL( ) (si4432_rxbufs[0][0] != 0)
#endif /* RADIO_RXBUFS > 1 */

static uint8_t si4432_txbuf[SI4432_MAX_PACKET_LEN + 1];

#define CLEAN_TXBUF() (si4432_txbuf[0] = 0)
#define TXBUF_EMPTY() (si4432_txbuf[0] == 0)

/*
 * The transceiver state.
 */
#define ON     0
#define OFF    1

#define SEND_FRAME_LEN	1
#define SEND_TIMEOUT	1200

#define BUSYWAIT_UNTIL(cond, max_time)                                    \
    do {                                                                  \
      rtimer_clock_t t0;                                                  \
      t0 = RTIMER_NOW();                                                  \
      while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
    } while(0)


static volatile uint8_t onoroff = OFF;
static volatile int8_t last_rssi;
static volatile uint8_t reg_rssi;
static uint8_t locked;
static int rfchannel;
static volatile uint8_t  ubRxCnt = 0;
static volatile uint8_t is_transmitting;
static volatile uint8_t seqnum;
static volatile linkaddr_t destaddr;


volatile uint8_t ubAckData[3];
volatile uint8_t  ubACKFlag;
//volatile uint8_t ubRxFlag = 0;
static process_event_t rx_led_event;
static process_event_t tx_led_event;
//static process_event_t rssi_event;

volatile enum SI4432_STATE si4432_state = SI4432_IDLE;
volatile uint8_t ubRxFlag = 0;

static volatile uint8_t rf_tx_failed_check_cnt = 0;

static volatile uint8_t rf_state_led_lock = 0;


extern void sysPrintExp(unsigned int dwPos);
extern u_long sysGetLR(void);


#define GET_LOCK() locked++
/*--------------------------------------------------------------------------*/
static void RELEASE_LOCK(void)
{
	if(locked > 0)
		locked--;
}


#define RF_RSSI_THD	   100

PROCESS_NAME(si4432_sendack_process);


/*---------------------------------------------------------------------------*/
static u_char getreg(u_char ubregname)
{
	u_char ubreg;
	ubreg = SI4432ReadReg( ubregname );
	return ubreg;
}
/*---------------------------------------------------------------------------*/
static void setreg(u_char ubregname, u_char ubValue)
{
	SI4432WriteReg(ubregname, ubValue);
}

/*---------------------------------------------------------------------------*/
static void set_txpower(uint8_t ubpower)
{
	u_char ubreg;

	ubreg = getreg(SI4432_TX_POWER);
	ubreg = (ubreg & (~0x07)) | (ubpower & 0x07);
	setreg(SI4432_TX_POWER, ubreg);
}

//Get current si4432 statte
/*
si4432 has two state:SI4432_IDLE, SI4432_TX, SI4432_RX_RECEIVING
*/
static uint8_t si4432_radio_status(void)
{
	return si4432_state;
}

/*---------------------------------------------------------------------------*/
static int si4432_radio_init(void)
{

	SI4432Init( );
	//SI4432RXPrepare( );//Receive mode
	onoroff = OFF;
	locked = 0;
	si4432_state = SI4432_IDLE;

	CLEAN_RXBUFS();
	CLEAN_TXBUF();

	process_start(&si4432_radio_process, NULL);
	process_start(&si4432_sendack_process, NULL);
	//process_start(&si4432_channel_scan, NULL);
	return 0;
}

/*---------------------------------------------------------------------------*/
static int si4432_radio_transmit(unsigned short payload_len)
{
	#define DEBUGTEST 1

	si4432_txbuf[0] = payload_len;  //data packet length

	GET_LOCK();

	#if 1
	while(((si4432_state & SI4432_TX) == SI4432_TX) ||((si4432_state & SI4432_RX_RECEIVING)== SI4432_RX_RECEIVING))
	{	
        /* we are not transmitting. This means that
           we just started receiving a packet or sending a paket,
           so we drop the transmission. */
        #if 0
        udwTimeOut++;
        if ( udwTimeOut > 720000)
        {
			XPRINTF((0, "SEND DROP****************************************\r\n"));       
			RELEASE_LOCK();
			//SI4432RXPrepare( );
	        return RADIO_TX_COLLISION;
        }
        #else
    
		XPRINTF((10, "SEND DROP-----------\r\n"));       
		RELEASE_LOCK();
        return RADIO_TX_COLLISION;
        #endif
     }
     #endif
     #if 1
    if (rf_tx_failed_check_cnt > 20)
    {
    	SI4432RXPrepare( );
    }
	else if (rf_tx_failed_check_cnt > 40)
	{
		SI4432Init( );
	}
	XPRINTF((10, "rssi is %d \r\n",SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR)));	
	XPRINTF((10, "rf_tx_failed_check_cnt %d\r\n",rf_tx_failed_check_cnt));	
	if (SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR) > RF_RSSI_THD)
	{	
        /* we are not transmitting. This means that
           we just started receiving a packet or sending a paket,
           so we drop the transmission. */
        #if 0
        udwTimeOut++;
        if ( udwTimeOut > 720000)
        {
			XPRINTF((0, "SEND DROP****************************************\r\n"));       
			RELEASE_LOCK();
			//SI4432RXPrepare( );
	        return RADIO_TX_COLLISION;
        }
        #else
    	rf_tx_failed_check_cnt ++;
		XPRINTF((10, "rf channel busy******....\r\n"));       
		RELEASE_LOCK();
        return RADIO_TX_COLLISION;
        #endif
     }
    #endif
	XPRINTF((10, "Tx clock_start is %d\r\n", clock_time( )));
	si4432_state = SI4432_TX;
	is_transmitting = 1;
	if(SI4432RadioTransmit(si4432_txbuf) == SI4432_SUCCESS) 
	{
		frame802154_t info154;
		frame802154_parse((u_char *)&si4432_txbuf[1], si4432_txbuf[0], &info154);
		if(info154.fcf.frame_type == FRAME802154_DATAFRAME &&info154.fcf.ack_required != 0)
		{
			seqnum = info154.seq;
			memcpy((void*)&destaddr, info154.dest_addr, 8);
			
			XPRINTF((10, "seqnum %02x\r\n", seqnum));
			MEM_DUMP(10, "dest", (void *)&destaddr, 8);
		}

		#if  DEBUGTEST > 0
		{
			XPRINTF((10, "Tx clock_end is %d\r\n", clock_time( )));
			MEM_DUMP(8, "TX->",(u_char*)si4432_txbuf, si4432_txbuf[0]+1);
			
		}
		#endif
		rf_tx_failed_check_cnt = 0;
		is_transmitting = 0;
		RELEASE_LOCK( );
		si4432_state = SI4432_IDLE; // when send packet, return IDLE state
		//memset((u_char*)si4432_txbuf, 0, si4432_txbuf[0] + 1);//clear txbuf
		//process_post(&si4432_txled_process, tx_led_event, NULL);
		return RADIO_TX_OK;
	}
	else
	{

		RELEASE_LOCK( );
		/* TODO: Do we have to retransmit? */
		//CLEAN_TXBUF( );
		is_transmitting = 0;
		si4432_state = SI4432_IDLE; // when send packet, return IDLE state
		return RADIO_TX_ERR;
	}
}
/*---------------------------------------------------------------------------*/
static int si4432_radio_prepare(const void *payload, unsigned short payload_len)
{

	GET_LOCK( );
	if(payload_len > SI4432_MAX_PACKET_LEN) 
	{
		XPRINTF((10,"SI4432: payload length=%d is too long.\r\n", payload_len));
		RELEASE_LOCK( );
		return RADIO_TX_ERR;
	}
	#if !RADIO_WAIT_FOR_PACKET_SENT
	/* 
	* Check if the txbuf is empty. Wait for a finite time.
	* This should not occur if we wait for the end of transmission in 
	* si4432_radio_transmit().
	*/
	if(wait_for_tx()) 
	{
		XPRINTF((10,"si4432: radio prepare tx buffer full.\r\n"));
		RELEASE_LOCK( );
		return RADIO_TX_ERR;
	}
	#endif /* RADIO_WAIT_FOR_PACKET_SENT */
	
	/*
	* Copy to the txbuf. 
	* The first byte must be the packet length.
	*/
	CLEAN_TXBUF();
	memcpy(si4432_txbuf+1, payload, payload_len);

	RELEASE_LOCK( );
	return RADIO_TX_OK;
}
#if 1
/*---------------------------------------------------------------------------*/
int si4432_radio_send(const void *payload, unsigned short payload_len)
{
	si4432_radio_prepare(payload, payload_len);
	return si4432_radio_transmit(payload_len);
}
#else
/*---------------------------------------------------------------------------*/
int si4432_radio_send(const void *payload, unsigned short payload_len)
{
	u_char ubDevStatus = 0;
  	rtimer_clock_t timeout_time;
	PRINTF("IN SEND\r\n");
	
	if (si4432_radio_prepare(payload, payload_len) == RADIO_TX_ERR) 
	{
		return RADIO_TX_ERR;
	}
	//ubDevStatus = SI4432ReadReg(SI4432_CRYSTAL_OSCILLATOR_CONTROL_TEST);
	XPRINTF((0,"ubDevStatus  %02x\r\n", ubDevStatus));
	timeout_time = RTIMER_NOW() + RTIMER_SECOND / 1000 * SEND_TIMEOUT;
	//ubDevStatus = 0;
	XPRINTF((0,"timeout_time  %d\r\n", timeout_time));
	//when the chip is receiving data or tx data, need to wait
	while(((si4432_state & SI4432_RX_RECEIVING) == SI4432_RX_RECEIVING)||((si4432_state & SI4432_TX) == SI4432_TX))
	{
		//ubDevStatus = SI4432ReadReg(SI4432_DEVICE_STATUS)&SI4432_CPS_MASK;
		if ( RTIMER_CLOCK_LT(timeout_time, RTIMER_NOW()))//time out
		{
			XPRINTF((0,"si4432: transmission blocked by reception in progress\n"));
			return RADIO_TX_ERR;
		}
	}
	XPRINTF((0,"RTIMER_NOW  %d\r\n", RTIMER_NOW()));
	XPRINTF((15,"radio start sent\r\n"));

	//SI4432SendDataPrepare( );  //when send data, reback to receive mode
	
	return si4432_radio_transmit(payload_len);
}
#endif

/*---------------------------------------------------------------------------*/
static int si4432_radio_off(void)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
static int si4432_radio_on(void)
{
	return 1;
}

/*---------------------------------------------------------------------------*/
int
si4432_get_channel(void)
{
  return rfchannel;
}
/*---------------------------------------------------------------------------*/
int si4432_radio_set_channel(uint8_t ubchannel)
{
	rfchannel = ubchannel;

  	BUSYWAIT_UNTIL(((si4432_state&(SI4432_TX) == SI4432_TX)||(si4432_state&(SI4432_TX) == SI4432_RX_RECEIVING)), RTIMER_SECOND / 10);

	SI4432_SetChannel(ubchannel);
	return 1;
}
/*---------------------------------------------------------------------------*/
static int wait_for_tx(void)
{
	struct timer t;
	timer_set(&t, CLOCK_SECOND / 6);
	while(!TXBUF_EMPTY()) 
	{
		if(timer_expired(&t)) 
		{
			XPRINTF((10,"si446x: tx buffer full.\r\n"));
			return 1;
		}
		//PRINTF("In wait for tx\r\n");
		/* Put CPU in sleep mode. */
		//halSleepWithOptions(SLEEPMODE_IDLE, 0);
	}
  return 0;
}
/*---------------------------------------------------------------------------*/
static int si4432_radio_channel_clear(void)
{
  //return ST_RadioChannelIsClear();
  	if ((si4432_state&SI4432_RX_RECEIVING == SI4432_RX_RECEIVING)||(si4432_state&SI4432_TX == SI4432_TX) || SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR) > RF_RSSI_THD)
  	{
  		return 0;//rf busy
  	}
	return 1;
}
/*---------------------------------------------------------------------------*/
static int si4432_radio_receiving_packet(void)
{
  //return receiving_packet;
    return si4432_state & SI4432_RX_RECEIVING;
}
/*---------------------------------------------------------------------------*/
static int si4432_radio_pending_packet(void)
{
  return !RXBUFS_EMPTY();
}

/*---------------------------------------------------------------------------*/
int si4432_radio_is_on(void)
{
  return onoroff == ON;
}

/*---------------------------------------------------------------------------*/
static int si4432_radio_read(void *buf, unsigned short bufsize)
{
  return read_from_rxbuf(buf, bufsize);
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

	//PRINTF("first 0   %d\r\n", first);
	int8_t packet_rssi;
	if(RXBUFS_EMPTY()) 
	{          /* Buffers are all empty */
		return 0;
	}

	if(si4432_rxbufs[first][0] > len) 
	{   /* Too large packet for dest. */
		len = 0;
	} 
	else 
	{
		len = si4432_rxbufs[first][0];
		packet_rssi = (int8_t)si4432_rxbufs[first][129];
		memcpy(dest, (uint8_t*)&si4432_rxbufs[first][0] + 1, len);
		//packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
		packetbuf_set_attr(PACKETBUF_ATTR_RSSI, packet_rssi);
	}

	#if RADIO_RXBUFS > 1
	{
		//OSCRITICAL cr;
		int first_tmp;
		//OSInitCritical(&cr);
		//OSEnterCritical(&cr);		
		first = (first + 1) % RADIO_RXBUFS;
		first_tmp = first;
		if(first_tmp == last) 
		{
			CLEAN_RXBUFS();
		}
	//	OSExitCritical(p);
	}
	#else
	CLEAN_RXBUFS();
	#endif

	return len;
}
/*---------------------------------------------------------------------------*/
short last_packet_rssi()
{
  return last_rssi;
}





/*---------------------------------------------------------------------------*/
PROCESS(si4432_radio_process, "si4432 radio driver");
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
PROCESS(si4432_rxled_process, "rx_led");
/*---------------------------------------------------------------------------*/
PROCESS(si4432_txled_process, "tx_led");
/*---------------------------------------------------------------------------*/
PROCESS(si4432_rfled_process, "rf_led");

/*---------------------------------------------------------------------------*/
PROCESS(timer_wait_random, "timer_wait_random");


/*---------------------------------------------------------------------------*/
PROCESS(si4432_rssi, "si4432 rssi");
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
PROCESS(si4432_checkrx, "si4432 checkrx");
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(si4432_sendack_process, "send ack");
/*---------------------------------------------------------------------------*/




PROCESS_THREAD(si4432_radio_process, ev, data)
{
	int len;

	PROCESS_BEGIN();
	XPRINTF((10,"rf_radio_process: started\r\n"));
	//rimeaddr_node_addr.u8[0] = 1;
	// rimeaddr_node_addr.u8[1] = 0;
	rx_led_event = process_alloc_event( );
	tx_led_event = process_alloc_event( );	
	//data_event = process_alloc_event( );
	//process_start(&si4432_rxled_process,NULL);
	//process_start(&si4432_txled_process,NULL);

	//process_start(&timer_wait_random,NULL);
	//process_start(&si4432_checkrx, NULL);
	//process_start(&si4432_rssi,NULL);
	//process_start(&si4432_rx,NULL);
	//process_start(&si4432_rfled_process,NULL);
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
			//XPRINTF((10, "rf rx time is %d\r\n", clock_time( )));
			//MEM_DUMP(8, "RX<-",(u_char*)packetbuf_dataptr(), len);				
			packetbuf_set_datalen(len);

			NETSTACK_RDC.input();
			//process_post(&si4432_rxled_process, rx_led_event, NULL);

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

static int
is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = mode == FRAME802154_SHORTADDRMODE ? 2 : 8;
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}

PROCESS_THREAD(si4432_sendack_process, ev, data)
{
	static SI4432RECVPAKETDATA *pRevPak = NULL;
	frame802154_t info154;
	int c = 0;
	PROCESS_BEGIN();
	XPRINTF((0, "start si4432_sendack\r\n"));
	while(1) 
	{
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		pRevPak = (SI4432RECVPAKETDATA *)SI4432GetRXRecvPak( );

		if (linkaddr_cmp((linkaddr_t *)&pRevPak->ubRxPaketData[4],(linkaddr_t *)&destaddr)&&(pRevPak->ubRxPaketData[3] == seqnum))
		{
			MEM_DUMP(8, "RX<-",(u_char*)pRevPak->ubRxPaketData, pRevPak->ubDataLen);
		}
	
		if((pRevPak->ubDataLen > (ACK_154_LEN+1))&& (pRevPak->ubRxPaketData[128]) > 60) //rssi_reg value
		{
			//XPRINTF((10,"RSSI is = %d\n", (char)pRevPak->ubRxPaketData[128]));
      		/* Send a link-layer ACK before reading the full packet. */
      		/* Try to parse the incoming frame as a 802.15.4 header. */
			/* For dataframes that has the ACK request bit set and thatis destined for us, we send an ack. */
			c = frame802154_parse((u_char *)&pRevPak->ubRxPaketData[1], pRevPak->ubRxPaketData[0], &info154);
			if(is_broadcast_addr(FRAME802154_SHORTADDRMODE,(uint8_t *)&info154.dest_addr) ||
  			   is_broadcast_addr(FRAME802154_LONGADDRMODE,(uint8_t *)&info154.dest_addr) ||
               linkaddr_cmp((linkaddr_t *)&info154.dest_addr,&linkaddr_node_addr)) 
            {
				if( info154.fcf.frame_type == FRAME802154_DATAFRAME &&
					info154.fcf.ack_required != 0 &&
					linkaddr_cmp((linkaddr_t *)&info154.dest_addr,&linkaddr_node_addr)) 
	            {
					send_ack(info154);
				}
				
				XPRINTF((10, "rf rx time is %d\r\n", clock_time( )));
				MEM_DUMP(8, "RX<-",(u_char*)pRevPak->ubRxPaketData, pRevPak->ubDataLen);	

				add_to_rxbuf(pRevPak->ubRxPaketData);
				process_poll(&si4432_radio_process);
			}
      	}
    }
	PROCESS_END();
}





//RX receive led state blue led
PROCESS_THREAD(si4432_rxled_process, ev, data)
{
	static struct etimer et;	
//	static u_char ubLedCnt = 0;

	PROCESS_BEGIN();

	while (1)
	{
		PROCESS_YIELD_UNTIL(ev == rx_led_event);
		//PROCESS_YIELD();
		//if (ev == rx_led_event)
		//for (ubLedCnt = 0; ubLedCnt < 3; ubLedCnt++)
		{
			/*
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

			*/
			while(rf_state_led_lock)
			{
				PROCESS_PAUSE( );
			}
			rf_state_led_lock = 1;
			etimer_set(&et, 40);
			LED4(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED4(0);
			etimer_set(&et, 40);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

			etimer_set(&et, 40);
			LED4(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED4(0);	
			etimer_set(&et, 40);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			rf_state_led_lock = 0;
		}
		
		//XPRINTF((0,"RX LED\r\n"));
	}

	PROCESS_END();
}

//RX receive led state blue led
PROCESS_THREAD(si4432_rfled_process, ev, data)
{
	static struct etimer et;	
	PROCESS_BEGIN();

	while (1)
	{
		PROCESS_YIELD( );
		//if (ev == rx_led_event)//rx state led
		{
			etimer_set(&et, 40);
			LED2(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED2(0);
			etimer_set(&et, 40);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

			etimer_set(&et, 40);
			LED2(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED2(0);
		}
		//XPRINTF((0,"RX LED\r\n"));
	}

	PROCESS_END();
}







//TX receive led state  red led
PROCESS_THREAD(si4432_txled_process, ev, data)
{
	static struct etimer et;	

	PROCESS_BEGIN();

	while (1)
	{
		
		PROCESS_YIELD_UNTIL(ev == tx_led_event);
		//PROCESS_YIELD( );
		//for (ubLedCnt = 0; ubLedCnt < 2; ubLedCnt++)
		{

			while(rf_state_led_lock)
			{
				PROCESS_PAUSE( );
			}
			rf_state_led_lock = 1;
			etimer_set(&et, 40);
			LED3(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED3(0);
			etimer_set(&et, 40);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

			etimer_set(&et, 40);
			LED3(1);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			LED3(0);
			etimer_set(&et, 40);
			PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
			rf_state_led_lock = 0;
		}
		//XPRINTF((0, "Tx LED\r\n"));
	}

	PROCESS_END();
}


PROCESS_THREAD(timer_wait_random, ev, data)
{
	static struct etimer et;	

	PROCESS_BEGIN();
	XPRINTF((10, "TIMER wait\r\n"));
	etimer_set(&et, (10*(random_rand( )%NBR_TABLE_CONF_MAX_NEIGHBORS)));
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	
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
		XPRINTF((10, "RSSI IS %d\r\n",SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR)));
		PROCESS_PAUSE( );
	}

	PROCESS_END();
}


PROCESS_THREAD(si4432_checkrx, ev, data)
{
	static struct etimer et;	

	static u_long i = 0;

	PROCESS_BEGIN();

	while (1)
	{		
		i = 0;
		XPRINTF((10, "RX CHECK START\r\n"));
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);
		for(i = 0; i < 13 ; i++)
		{
			if (ubRxFlag)
			{
				if (i == 12)
				{	
					ubRxFlag = 0;
					break;
				}
				etimer_set(&et, 10);
				XPRINTF((10, "rx1 i = %d\r\n", i));
				PROCESS_YIELD_UNTIL(etimer_expired(&et));
			}
			else
			{
				XPRINTF((10, "rx2 i = %d\r\n", i));
				si4432_state &= ~(SI4432_RX_RECEIVING); 
				
				//PROCESS_YIELD();
				break;
			}
		}
		XPRINTF((10, "rx3 i = %d\r\n", i));
		PROCESS_YIELD();
	}

	PROCESS_END();
}


extern volatile uint8_t si4432_ubpksent ;
extern volatile uint8_t si4432_ubtxffaem;

//io interrupt service funtion for si4432
/*lz在什么时候读取rssi值的，我现在在接收到数据后读取rssi的值，可靠吗？lz能否讲一下 

-------------------------------------------------------------------------------------------------------------------------------------
1、把06H设置为0X80使能同步子侦测。 
2、写个中断查询函数，如果检测到中断，则查询04H是否为0X80（侦测到同步字），如果为真，读26H即可。
*/
#if 0
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
	
	SI4432REGSTAT *pRegSta = (SI4432REGSTAT *)SI4432GetRFRegSta( );
	SI4432RECVPAKETDATA *pRevPak = (SI4432RECVPAKETDATA *)SI4432GetRXRecvPak( );

		
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line6);	
		//SI4432_Disable_Interrupt( );
		
		ubEnIFlag1 = SI4432ReadReg(SI4432_INTERRUPT_ENABLE_1);
		ubEnIFlag2 = SI4432ReadReg(SI4432_INTERRUPT_ENABLE_2);
		//clear interrupt flag
		ubIntSta1 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
		ubIntSta2 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
		pRegSta->ubINTFlag = 1; //this flag only when si4432 reset

		ubInterruptFlag1 = ubIntSta1&ubEnIFlag1;
		ubInterruptFlag2 = ubIntSta2&ubEnIFlag2;

		
		//receive crc eeror, clear the si4432 rx fifo buf
		if ((ubInterruptFlag1 & SI4432_ICRCERROR) == SI4432_ICRCERROR)
		{
			SI4432RXPrepare( );
			si4432_state = SI4432_IDLE;
			//clear relate  flag when receive packet is right or not;
			pRevPak->ubPaketLenFlag = 0;
			pRevPak->ubRxPaketData[0] = 0;
			pRevPak->ubDataLen = 0;
			pRevPak->ubRevDataCnt = 0;
			memset(pRevPak->ubRxPaketData, 0, 128);
		}

		//sent tx
		if ((ubInterruptFlag1 & SI4432_ITXFFAEM) == SI4432_ITXFFAEM)
		{
			pRegSta->ubtxffaem = SI4432_ITXFFAEM;
			si4432_ubtxffaem = 1;
			//PRINTF(".");
		}
		//sent paket finish
		if ((ubInterruptFlag1 & SI4432_IPKSENT) == SI4432_IPKSENT)
		{
			pRegSta->ubpksent = SI4432_IPKSENT;
			si4432_ubpksent = 1;
			si4432_state = SI4432_IDLE;
		}		

		//rx when receive data almost full, read data to buf.
		if ((ubInterruptFlag1 & SI4432_IRXFFAFULL) == SI4432_IRXFFAFULL)
		{
			SI4432ReadMutiData((u8 *)(&pRevPak->ubRxPaketData[0]+pRevPak->ubPaketLenFlag*SI4432RXFAFREAD), SI4432RXFAFREAD);
			pRevPak->ubPaketLenFlag++;
			pRevPak->ubRevDataCnt += SI4432RXFAFREAD;
			if (pRevPak->ubRxPaketData[0] > 128)
			{
				SI4432RXPrepare( );	
			}
		}
		//rx Receive paket data finish.
		if ((ubInterruptFlag1 & SI4432_IPKVALID) == SI4432_IPKVALID)
		{
			ubRxFlag = 0;   //rx flinish		
			SpiWriteRegister(0x07, 0x01);//Disable receive
			
			pRevPak->ubDataLen = SI4432ReadReg( SI4432_RECEIVED_PACKET_LENGTH);
			pRevPak->ubIPKVaild = 1;

			//if (pRevPak->ubDataLen > (pRevPak->ubPaketLenFlag*SI4432RXFAFREAD))
			if ((pRevPak->ubDataLen > (pRevPak->ubRevDataCnt))&&(pRevPak->ubDataLen <= 128))
			{
				#if 0
				SI4432ReadMutiData((u8 *)(&pRevPak->ubRxPaketData[0]+pRevPak->ubPaketLenFlag*SI4432RXFAFREAD), 
										pRevPak->ubDataLen - pRevPak->ubPaketLenFlag*SI4432RXFAFREAD);
				#else
				SI4432ReadMutiData((u8 *)(&pRevPak->ubRxPaketData[0]+pRevPak->ubRevDataCnt), pRevPak->ubDataLen - pRevPak->ubRevDataCnt);
				#endif
			}

			if ((pRevPak->ubDataLen == (pRevPak->ubRxPaketData[0] + 1))&&(pRevPak->ubDataLen <= 128 ))
			{

				if (pRevPak->ubRxPaketData[0] == ACK_154_LEN)
				{
					if (linkaddr_cmp((linkaddr_t *)&pRevPak->ubRxPaketData[4],(linkaddr_t *)&destaddr)&&(pRevPak->ubRxPaketData[3] == seqnum) && last_rssi > -85) 
					{
						ubACKFlag = 3;
						ubAckData[0] =  pRevPak->ubRxPaketData[1];
						ubAckData[1] =  pRevPak->ubRxPaketData[2];
						ubAckData[2] =  pRevPak->ubRxPaketData[3];
					}
				}
				//else
				{
					//add_to_rxbuf(pRevPak->ubRxPaketData);
					//process_poll(&si4432_radio_process);
					pRevPak->ubRxPaketData[128] = reg_rssi; //add rssi to last data position
					pRevPak->ubRxPaketData[129] = last_rssi; //add rssi to last data position
					process_poll(&si4432_sendack_process);
					//process_poll(&si4432_rfled_process);
				}
			}

			//MEM_DUMP(0, "rx", pRevPak->ubRxPaketData, pRevPak->ubRxPaketData[0]);
			SI4432RXPrepare( );
			si4432_state = SI4432_IDLE;
			//clear relate  flag when receive packet is right or not;
			//pRevPak->ubPaketLenFlag = 0;
			//pRevPak->ubRxPaketData[0] = 0;
			//pRevPak->ubDataLen = 0;
			//pRevPak->ubRevDataCnt = 0;
		}


		if ((ubIntSta2 & SI4432_ISWDET) == SI4432_ISWDET)//receive Preamble
		{
			ubRxFlag = 1;
			//SI4432_SET_OPSTATE(SI4432_RX_RECEIVING);
			si4432_state = SI4432_RX_RECEIVING;
			reg_rssi = SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR);
			last_rssi = -((int8_t)((230-reg_rssi)>>1));
			pRevPak->ubPaketLenFlag = 0;
			pRevPak->ubRxPaketData[0] = 0;
			pRevPak->ubDataLen = 0;
			pRevPak->ubRevDataCnt = 0;
			//XPRINTF((0, "rssi = %d\r\n", last_rssi));
		}
		#if 0
		if ((ubIntSta2 & SI4432_IPREAVAL) == SI4432_IPREAVAL)//receive Preamble
		{			
			si4432_state = SI4432_RX_PRE;
			rf_pre_check_cnt = 10;
			XPRINTF((10, "_pre\r\n"));
		}
		#endif
		
	//SI4432_Enable_Interrupt( );	
	}
	//OSExitCritical(&cr);
}
#endif


#if 1


static void send_ack(frame802154_t info154)
{
	uint8_t ackdata[16] = {0};

	if(is_transmitting || ((si4432_state & SI4432_RX_RECEIVING)== SI4432_RX_RECEIVING)) 
	{
		/* Trying to send an ACK while transmitting  or is receiving - should not be
		possible, so this check is here only to make sure. */
		return;
	}
	ackdata[0] = 11; //ack length
	ackdata[1] = FRAME802154_ACKFRAME;
	ackdata[2] = 0;
	ackdata[3] = info154.seq;
	memcpy(&ackdata[4], &linkaddr_node_addr, 8);

	/* Send packet length */
	is_transmitting = 1;
	SI4432RadioTransmit(ackdata);
	is_transmitting = 0;

	MEM_DUMP(10, "ac->", ackdata, 12);
}


#endif



/*--------------------------------------------------------------------------*/
const struct radio_driver si4432_radio_driver = {
  si4432_radio_init,
  si4432_radio_prepare,
  si4432_radio_transmit,
  si4432_radio_send,
  si4432_radio_read,
  si4432_radio_channel_clear,
  si4432_radio_receiving_packet,
  si4432_radio_pending_packet,
  si4432_radio_on,
  si4432_radio_off,
};


/** @} */
