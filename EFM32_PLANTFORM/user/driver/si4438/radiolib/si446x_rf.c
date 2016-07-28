#include "contiki.h"
#include "dev/radio.h"
#include "net/mac/frame802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "sys/rtimer.h"

#include "basictype.h"

#include "si4438.h"
#include "bsp.h"
#include "si4438_int_ctr_def.h"
#include "si446x_defs_u.h"
#include "si446x.h"

#include "sysprintf.h"

#include <string.h>


#include "em32lg_config.h"


/*---------------------------------------------------------------------------*/
PROCESS(si446x_radio_process, "si446x radio driver");
/*---------------------------------------------------------------------------*/
PROCESS(si446x_rxled_process, "rx_led");
/*---------------------------------------------------------------------------*/
PROCESS(si446x_txled_process, "tx_led");
/*---------------------------------------------------------------------------*/
//PROCESS(si446x_rfled_process, "rf_led");
/*---------------------------------------------------------------------------*/
PROCESS(si446x_sendack_process, "send ack");
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
static int si446x_radio_init(void);
static int si446x_radio_prepare(const void *payload,unsigned short payload_len);
static int si446x_radio_transmit(unsigned short payload_len);
static int si446x_radio_send(const void *data, unsigned short len);
static int si446x_radio_read(void *buf, unsigned short bufsize);
static int si446x_radio_channel_clear(void);
static int si446x_radio_receiving_packet(void);
static int si446x_radio_pending_packet(void);
static int si446x_radio_on(void);
static int si446x_radio_off(void);
static int add_to_rxbuf(uint8_t * src);
static int read_from_rxbuf(void *dest, unsigned short len);
static void send_ack(frame802154_t info154);

#define SI446X_ACK_154_LEN		11
#define SI446X_MAX_PACKET_LEN	126
#define SI446X_CUR_RSSI			127

/*
 * The buffers which hold incoming data.
 */
#ifndef SI446X_RADIO_RXBUFS
#define SI446X_RADIO_RXBUFS 4
#endif 


/* +1 because of the first byte, which will contain the length of the packet. */
// +1 last data is rssi
//len1 [data] rssi1
static uint8_t si446x_rxbufs[SI446X_RADIO_RXBUFS][SI446X_MAX_PACKET_LEN + 1 + 1];//len1 [data] rssi1  last_rssi


#if SI446X_RADIO_RXBUFS > 1
static volatile int8_t first = -1, last = 0;
#else   
static const int8_t first = 0, last = 0;
#endif  


#if SI446X_RADIO_RXBUFS > 1
#define CLEAN_RXBUFS() do{first = -1; last = 0;}while(0)
#define RXBUFS_EMPTY() (first == -1)
static int RXBUFS_FULL( )
{
	int8_t first_tmp = first;
	return first_tmp == last;
}
#else 
#define CLEAN_RXBUFS( ) (si446x_rxbufs[0][0] = 0)
#define RXBUFS_EMPTY( ) (si446x_rxbufs[0][0] == 0)
#define RXBUFS_FULL( ) (si446x_rxbufs[0][0] != 0)
#endif 

static uint8_t si446x_txbuf[SI446X_MAX_PACKET_LEN + 1 + 1];//128Bytes

#define CLEAN_TXBUF() (si446x_txbuf[0] = 0)
#define TXBUF_EMPTY() (si446x_txbuf[0] == 0)

#define BUSYWAIT_UNTIL(cond, max_time)                                    \
    do {                                                                  \
      rtimer_clock_t t0;                                                  \
      t0 = RTIMER_NOW();                                                  \
      while(!(cond) && RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + (max_time)));   \
    } while(0)

static volatile uint8_t onoroff = 0;//OFF;
static volatile int8_t last_rssi;
static u_char ubcurrssi;
static uint8_t locked;
static uint8_t rfchannel;
static volatile uint8_t is_transmitting;
static volatile uint8_t seqnum;
static volatile linkaddr_t destaddr;


static volatile uint8_t ubAckData[3];
static volatile uint8_t  ubACKFlag;
//volatile uint8_t ubRxFlag = 0;

static process_event_t rx_led_event;
static process_event_t tx_led_event;


static volatile uint8_t rf_tx_failed_check_cnt = 0;
static volatile uint8_t rf_state_led_lock = 0;


volatile  SI446X_STATE si446x_state = SI446X_IDLE;
volatile uint8_t ubRxFlag;

#define RF_RSSI_THD	   100

extern void sysPrintExp(unsigned int dwPos);
extern u_long sysGetLR(void);

/* If set to 1, a send() returns only after the packet has been transmitted.
  This is necessary if you use the x-mac module, for example. */
#ifndef RADIO_WAIT_FOR_PACKET_SENT
#define RADIO_WAIT_FOR_PACKET_SENT 1
#endif  /* RADIO_WAIT_FOR_PACKET_SENT */


static u_char ubarxbuf[128] = {0x00};

#define GET_LOCK() locked++
/*--------------------------------------------------------------------------*/
static void RELEASE_LOCK(void)
{
	if(locked > 0)
		locked--;
}


/*---------------------------------------------------------------------------*/
static int si446x_radio_init(void)
{
	si446xRadioInit( );
	onoroff = 0;//OFF;
	locked = 0;
	si446x_state = SI446X_IDLE;

	CLEAN_RXBUFS();
	CLEAN_TXBUF();

	process_start(&si446x_radio_process, NULL);
	process_start(&si446x_sendack_process, NULL);
	return 0;
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_transmit(unsigned short payload_len)
{
	#define DEBUGTEST 1

	si446x_txbuf[0] = payload_len;  //data packet length
	u_char ubLen = 0;
	GET_LOCK();

	#if 1
	while(((si446x_state & SI446X_TX) == SI446X_TX) ||((si446x_state & SI446X_RX_RECEIVING)== SI446X_RX_RECEIVING))
	{	
        /* we are not transmitting. This means that
           we just started receiving a packet or sending a paket,
           so we drop the transmission. */
        #if 0
        #else
		XPRINTF((10, "SEND DROP-----------\r\n"));       
		RELEASE_LOCK();
        return RADIO_TX_COLLISION;
        #endif
     }
     #endif

	#if 0
	if (SI446X_GET_CUR_RSSI( ) > RF_RSSI_THD)
	{	
		/* we are not transmitting. This means that
		we just started receiving a packet,
		so we drop the transmission. */
		#if 0
		#else
		rf_tx_failed_check_cnt ++;
		XPRINTF((10, "rf channel busy******....\r\n"));       
		RELEASE_LOCK();
		return RADIO_TX_COLLISION;
		#endif
	}
	#endif
	ubLen = SI446X_GET_FIFO_INFO_RX_BYTES( );
	XPRINTF((10, "Tx clock_start is %d, %d\r\n", clock_time( ), ubLen));
	ubLen = SI446X_GET_FIFO_INFO_RX_BYTES();
	si446x_state = SI446X_TX;
	is_transmitting = 1;
	if(si446xRadioSendFixLenData(si446x_txbuf) == 0x00) 
	{
		frame802154_t info154;
		frame802154_parse((u_char *)&si446x_txbuf[1], si446x_txbuf[0], &info154);
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
			MEM_DUMP(8, "TX->",(u_char*)si446x_txbuf, si446x_txbuf[0]+1);
			
		}
		#endif
		rf_tx_failed_check_cnt = 0;
		is_transmitting = 0;
		RELEASE_LOCK( );
		si446x_state = SI446X_IDLE; // when send packet, return IDLE state
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
		si446x_state = SI446X_IDLE; // when send packet, return IDLE state
		return RADIO_TX_ERR;
	}
}



/*---------------------------------------------------------------------------*/
static int si446x_radio_prepare(const void *payload, unsigned short payload_len)
{

	GET_LOCK( );
	if(payload_len > SI446X_MAX_PACKET_LEN) 
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
	memcpy(si446x_txbuf+1, payload, payload_len);

	RELEASE_LOCK( );
	return RADIO_TX_OK;
}


/*---------------------------------------------------------------------------*/
int si446x_radio_send(const void *payload, unsigned short payload_len)
{
	si446x_radio_prepare(payload, payload_len);
	return si446x_radio_transmit(payload_len);
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_off(void)
{
	return 1;
}
/*---------------------------------------------------------------------------*/
static int si446x_radio_on(void)
{
	return 1;
}



/*---------------------------------------------------------------------------*/
static int wait_for_tx(void)
{
	struct timer t;
	timer_set(&t, CLOCK_SECOND / 10);
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
static int si446x_radio_channel_clear(void)
{
  //return ST_RadioChannelIsClear();
  	if ((si446x_state&SI446X_RX_RECEIVING == SI446X_RX_RECEIVING)||(si446x_state&SI446X_TX == SI446X_TX))
  	{
  		return 0;//rf busy
  	}

  	//need be improve
  	if (SI446X_GET_CUR_RSSI() > RF_RSSI_THD)
  	{
  		return 0;//busy
  	}
	return 1;
}


/*---------------------------------------------------------------------------*/
static int si446x_radio_receiving_packet(void)
{
  //return receiving_packet;
    return si446x_state & SI446X_RX_RECEIVING;
}

/*---------------------------------------------------------------------------*/
static int si446x_radio_pending_packet(void)
{
  return !RXBUFS_EMPTY();
}


/*---------------------------------------------------------------------------*/
int si446x_radio_is_on(void)
{
  return onoroff == 1;//ON;
}


/*---------------------------------------------------------------------------*/
static int si446x_radio_read(void *buf, unsigned short bufsize)
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

	memcpy(si446x_rxbufs[last], src, src[0] + 1);//src[0] is data length in buf not include src[0], so all data length need to add 1 
	si446x_rxbufs[last][SI446X_CUR_RSSI] = src[SI446X_CUR_RSSI];
	//memcpy(si4432_rxbufs[last], src, src[0]);
	#if SI446X_RADIO_RXBUFS > 1
	last = (last + 1) % SI446X_RADIO_RXBUFS;
	if(first == -1) 
	{
		first = 0;
	}
	#endif
	
	memset(src, 0, src[0] + 1);//clear buf
	return 1;
}


/*---------------------------------------------------------------------------*/
static int read_from_rxbuf(void *dest, unsigned short len)
{
	//PRINTF("first 0   %d\r\n", first);
	u_char  packet_rssi;
	int8_t rssi;
	if(RXBUFS_EMPTY()) 
	{          
		return 0;
	}

	if(si446x_rxbufs[first][0] > len) 
	{   /* Too large packet for dest. */
		len = 0;
	} 
	else 
	{
		len = si446x_rxbufs[first][0];
		packet_rssi = si446x_rxbufs[first][SI446X_CUR_RSSI];
		rssi = -((int8_t)((0xff-packet_rssi)>>1));
		XPRINTF((0, "prssi %d last_rssi %d  rssi %d\r\n", packet_rssi, last_rssi, rssi));
		memcpy(dest, (uint8_t*)&si446x_rxbufs[first][0] + 1, len);
		//packetbuf_set_attr(PACKETBUF_ATTR_RSSI, last_rssi);
		packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
	}

	#if SI446X_RADIO_RXBUFS > 1
	{
		int first_tmp;
		first = (first + 1) % SI446X_RADIO_RXBUFS;
		first_tmp = first;
		if(first_tmp == last) 
		{
			CLEAN_RXBUFS();
		}
	}
	#else
	CLEAN_RXBUFS();
	#endif

	return len;
}

static void send_ack(frame802154_t info154)
{
	uint8_t ackdata[16] = {0};

	if(is_transmitting || ((si446x_state & SI446X_RX_RECEIVING)== SI446X_RX_RECEIVING)) 
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
	si446xRadioSendFixLenData(ackdata);
	is_transmitting = 0;

	MEM_DUMP(10, "ac->", ackdata, 12);
}

/******************************************************************/
PROCESS_THREAD(si446x_radio_process, ev, data)
{
	int len;

	PROCESS_BEGIN();
	XPRINTF((10,"rf_radio_process: started\r\n"));
	rx_led_event = process_alloc_event( );
	tx_led_event = process_alloc_event( );	

	//process_start(&si446x_rfled_process,NULL);
	while(1) 
	{
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

		packetbuf_clear();
		len = si446x_radio_read(packetbuf_dataptr(), PACKETBUF_SIZE);
		if(len > 0) 
		{
			packetbuf_set_datalen(len);

			NETSTACK_RDC.input();
			//process_post(&si446x_rxled_process, rx_led_event, NULL);
		}
		if(!RXBUFS_EMPTY()) 
		{
			/*
			* Some data packet still in rx buffer (this happens because process_poll
			* doesn't queue requests), so stm32w_radio_process needs to be called
			* again.
			*/
			process_poll(&si446x_radio_process);
		}
	}
	PROCESS_END();
}



static int is_broadcast_addr(uint8_t mode, uint8_t *addr)
{
  int i = mode == FRAME802154_SHORTADDRMODE ? 2 : 8;
  while(i-- > 0) {
    if(addr[i] != 0xff) {
      return 0;
    }
  }
  return 1;
}



/******************************************************************/
PROCESS_THREAD(si446x_sendack_process, ev, data)
{
	frame802154_t info154;
	int c = 0;
	PROCESS_BEGIN();
	XPRINTF((0, "start si446x_sendack\r\n"));
	while(1) 
	{
		PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_POLL);

		if (linkaddr_cmp((linkaddr_t *)&ubarxbuf[4],(linkaddr_t *)&destaddr)&&(ubarxbuf[3] == seqnum))
		{
			MEM_DUMP(8, "RX<-",ubarxbuf, ubarxbuf[0]+1);
		}
		
		XPRINTF((0, "rx\r\n"));
	
		if((ubarxbuf[0] > (SI446X_ACK_154_LEN+1))&& (ubarxbuf[SI446X_CUR_RSSI]) > 50) //rssi_reg value
		//if((ubarxbuf[0] > (SI446X_ACK_154_LEN+1))) //rssi_reg value
		{
			//XPRINTF((10,"RSSI is = %d\n", (char)pRevPak->ubRxPaketData[128]));
      		/* Send a link-layer ACK before reading the full packet. */
      		/* Try to parse the incoming frame as a 802.15.4 header. */
			/* For dataframes that has the ACK request bit set and thatis destined for us, we send an ack. */
			c = frame802154_parse((u_char *)&ubarxbuf[1], ubarxbuf[0], &info154);
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
				MEM_DUMP(8, "RX<-",ubarxbuf,ubarxbuf[0]+1);	

				add_to_rxbuf(ubarxbuf);
				process_poll(&si446x_radio_process);
			}
      	}
    }
	PROCESS_END();
}

/******************************************************************/
//RX receive led state blue led
PROCESS_THREAD(si446x_rxled_process, ev, data)
{
	static struct etimer et;	

	PROCESS_BEGIN();

	while (1)
	{
		#if 1
		PROCESS_YIELD_UNTIL(ev == rx_led_event);
		while(rf_state_led_lock)
		{
			PROCESS_PAUSE( );
		}
		rf_state_led_lock = 1;
		etimer_set(&et, 40);
		//LED4(1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		//LED4(0);
		etimer_set(&et, 40);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

		etimer_set(&et, 40);
		//LED4(1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		//LED4(0);	
		etimer_set(&et, 40);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		rf_state_led_lock = 0;
		#endif
	}
	PROCESS_END();
}

/******************************************************************/
//TX receive led state  red led
PROCESS_THREAD(si446x_txled_process, ev, data)
{
	static struct etimer et;	
	PROCESS_BEGIN();

	while (1)
	{
		#if 1
		PROCESS_YIELD_UNTIL(ev == tx_led_event);
		while(rf_state_led_lock)
		{
			PROCESS_PAUSE( );
		}
		rf_state_led_lock = 1;
		etimer_set(&et, 40);
		//LED3(1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		//LED3(0);
		etimer_set(&et, 40);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

		etimer_set(&et, 40);
		//LED3(1);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		//LED3(0);
		etimer_set(&et, 40);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		rf_state_led_lock = 0;
		#endif
	}
	PROCESS_END();
}


void static ph_prx_handler(void)
{
	si446xReadMutiData(ubarxbuf);

	if (ubarxbuf[0] == SI446X_ACK_154_LEN)
	{
		if (linkaddr_cmp((linkaddr_t *)&ubarxbuf[4],(linkaddr_t *)&destaddr)&&(ubarxbuf[3] == seqnum) && last_rssi > -100) 
		{
			ubACKFlag = 3;
			ubAckData[0] =  ubarxbuf[1];
			ubAckData[1] =  ubarxbuf[2];
			ubAckData[2] =  ubarxbuf[3];
		}
	}
	if (ubarxbuf[0] != 0)
	{
		//MEM_DUMP(0, "<-rx", ubarxbuf, ubarxbuf[0]+1);
		ubarxbuf[SI446X_CUR_RSSI] = ubcurrssi; //add rssi to last data position
		process_poll(&si446x_sendack_process);
	}
	si446xStartRX();
	si446x_state = SI446X_IDLE;
	ubRxFlag = 0;	
}

void static modem_syde_handler(void)
{
	//u_char ubrssi = 0;
	SI446X_GET_MODEM_STATUS(0x00);
	ubcurrssi = Si446xCmd.GET_MODEM_STATUS.CURR_RSSI;
	last_rssi = -((int8_t)((0xff-ubcurrssi)>>1));
	//XPRINTF((0, "last_rssi = %d\r\n", last_rssi));
	ubarxbuf[0] = 0;
	si446x_state = SI446X_RX;
	ubRxFlag = 1;
}


uint8_t get_ack_flag(void)
{
	return ubACKFlag;
}

void set_ack_data(uint8_t *pbuf, uint8_t ubLen)
{
	memcpy(pbuf, (void const*)ubAckData, ubLen);
}

void clear_ack_data_flag(void)
{
	ubACKFlag = 0;
	memset((void*)ubAckData, 0, sizeof(ubAckData));
}


#if 0
void EXTI9_5_IRQHandler(void)
{	
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		U8 si446x_ph_pend;
		U8 si446x_modem_pend;
		U8 si446x_chip_pend;
		U8 ubph_en;
		U8 ubmodem_en;
		U8 ubph_flag;
		U8 ubmodem_flag;

		EXTI_ClearITPendingBit(EXTI_Line6);	
				
		SI446X_GET_INT_STATUS( );
		si446x_ph_pend = Si446xCmd.GET_INT_STATUS.PH_PEND;
		si446x_modem_pend = Si446xCmd.GET_INT_STATUS.MODEM_PEND;
		si446x_chip_pend = Si446xCmd.GET_INT_STATUS.CHIP_PEND;
		ubph_en = SI446X_GET_PROPERTY_1(INT_CTL_PH_ENABLE);
		ubmodem_en = SI446X_GET_PROPERTY_1(INT_CTL_MODEM_ENABLE);

		ubph_flag = ubph_en&si446x_ph_pend;
		ubmodem_flag = ubmodem_en&si446x_modem_pend;

		//packet sent finish
		if ((ubph_flag& SI446X_INT_CTL_PH_PSENT_EN) == SI446X_INT_CTL_PH_PSENT_EN)//packet sent
		{
			si446x_set_pksent( );
		}
		//packet rx finish
		else if ((ubph_flag & SI446X_INT_CTL_PH_PRX_EN) == SI446X_INT_CTL_PH_PRX_EN)
		{
			ph_prx_handler( );
		}
		//packet crc error
		else if ((ubph_flag & SI446X_INT_CTL_PH_CRCE_EN) == SI446X_INT_CTL_PH_CRCE_EN)
		{
			si446xStartRX();
		}
		//syde
		if ((ubmodem_flag & SI446X_INT_CTL_MODEM_SYDE_EN) == SI446X_INT_CTL_MODEM_SYDE_EN)
		{
			modem_syde_handler();
		}
		memset(Si446xCmd.RAW, 0, 16);
	}
}
#endif



/*
void GPIO_ODD_IRQHandler(void)
{
}
*/
void GPIO_EVEN_IRQHandler(void)
{

	u_long intFlag = GPIO_IntGet( );
	INT_Disable( );
	if ((RF_nIRQ_FLAG&intFlag) == RF_nIRQ_FLAG)
	{
		U8 si446x_ph_pend;
		U8 si446x_modem_pend;
		U8 si446x_chip_pend;
		U8 ubph_en;
		U8 ubmodem_en;
		U8 ubph_flag;
		U8 ubmodem_flag;

		GPIO_IntClear(RF_nIRQ_FLAG);

			
		SI446X_GET_INT_STATUS( );
		si446x_ph_pend = Si446xCmd.GET_INT_STATUS.PH_PEND;
		si446x_modem_pend = Si446xCmd.GET_INT_STATUS.MODEM_PEND;
		si446x_chip_pend = Si446xCmd.GET_INT_STATUS.CHIP_PEND;
		ubph_en = SI446X_GET_PROPERTY_1(INT_CTL_PH_ENABLE);
		ubmodem_en = SI446X_GET_PROPERTY_1(INT_CTL_MODEM_ENABLE);

		ubph_flag = ubph_en&si446x_ph_pend;
		ubmodem_flag = ubmodem_en&si446x_modem_pend;

		//packet sent finish
		if ((ubph_flag& SI446X_INT_CTL_PH_PSENT_EN) == SI446X_INT_CTL_PH_PSENT_EN)//packet sent
		{
			//XPRINTF((0, "tx\r\n"));
			si446x_set_pksent( );
		}
		//packet rx finish
		else if ((ubph_flag & SI446X_INT_CTL_PH_PRX_EN) == SI446X_INT_CTL_PH_PRX_EN)
		{
			//XPRINTF((0, "rx\r\n"));
			ph_prx_handler( );
		}
		//packet crc error
		else if ((ubph_flag & SI446X_INT_CTL_PH_CRCE_EN) == SI446X_INT_CTL_PH_CRCE_EN)
		{
			//XPRINTF((0, "crce\r\n"));
			//si446x_read_fifo();
			si446xStartRX();
		}
		//syde
		if ((ubmodem_flag & SI446X_INT_CTL_MODEM_SYDE_EN) == SI446X_INT_CTL_MODEM_SYDE_EN)
		{
			//XPRINTF((0, "syde\r\n"));
			modem_syde_handler();
		}
		memset(Si446xCmd.RAW, 0, 16);
	}
	INT_Enable( );
}


/*--------------------------------------------------------------------------*/
const struct radio_driver si446x_radio_driver = {
  si446x_radio_init,
  si446x_radio_prepare,
  si446x_radio_transmit,
  si446x_radio_send,
  si446x_radio_read,
  si446x_radio_channel_clear,
  si446x_radio_receiving_packet,
  si446x_radio_pending_packet,
  si446x_radio_on,
  si446x_radio_off,
};



