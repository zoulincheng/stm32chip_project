#include "contiki.h"
#include "net/linkaddr.h"

#include "stm32f10x.h"
#include "basictype.h"
#include <stdint.h>

#include <debug-uart.h>

#include "xprintf.h"
#include "sysprintf.h"
#include "rtimer-arch.h"
#include "sysinit.h"
#include "iodef.h"

#include "arch_spi.h"

#include "dev/serial-line.h"


#include "net/rime/rime.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"

#include "userapp.h"
#include "dev_info.h"


#include "dev/watchdog.h"
#include "uart_dma.h"
#include "si446x_cmd.h"
#include "si446x.h"

#include "apphwgg.h"
#include "sim900a.h"


uint8_t process_request_u(void);

PROCESS(blink_process, "Blink");
PROCINIT(NULL);
PROCESS_THREAD(blink_process, ev, data)
{
	//static u_char ubach[4] = {0x12, 0x23, 0x55, 0x66};
	static struct etimer et;
	static u_char ubacrctest[] = {0x1f,0x3a,0xab,0xcc};
	u_short crc = 0;
	NET_MODE *pnetMode;
	//const static u_char ubTdata[6] = {0x05,0x11, 0x12, 0x13, 0x14,0x15};
	PROCESS_BEGIN();
	XPRINTF((10, "LED\r\n"));
	
	while(1) 
	{
		etimer_set(&et, 500);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		LED_T(1);
		pnetMode = (NET_MODE *)netModeGet();
		//if (getNetState( ))
		if (pnetMode->netMode != NET_CONNECT_NONE)
		{
			NET_LED(0);
			//FAULT_LED(1);
			ALARM_LED(1);
		}
		else
		{
			NET_LED(1);
			//FAULT_LED(0);
			ALARM_LED(0);
		}		//uart4_send_char('a');
		//sim900a_send_cmd("AT");
		etimer_set(&et, 500);
		//crc = cyg_crc16((const unsigned char*)ubacrctest, sizeof(ubacrctest));
		//XPRINTF((8, "CRC = %04x\n", crc));
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		LED_T(0);
		//uart2_send_char("a");
	}
	PROCESS_END();
}


//
PROCESS(feeddog_process, "feeddog");
PROCESS_THREAD(feeddog_process, ev, data)
{
	static struct etimer et;

	PROCESS_BEGIN();
	XPRINTF((10, "feeddog\r\n"));
	XPRINTF((10, "clock s is %d\r\n", clock_seconds( )));
	watchdog_init( );
	etimer_set(&et, 10*CLOCK_SECOND);
	
	while(1) 
	{		
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		watchdog_periodic( );
		etimer_reset(&et);
	}
	PROCESS_END();
}



/*---------------------------------------------------------------------------*/
uint8_t process_request_u(void) 
{
	#if 1
	extern uip_ds6_netif_t uip_ds6_if;
	uip_ds6_route_t *rt = NULL;
	uip_ds6_defrt_t *defrt = NULL;
	uip_ipaddr_t *addr = NULL;
	uint8_t i;
	uint8_t entry_size;
	uip_ds6_nbr_t *nbr = NULL;
	u_long udwNeighbors = 0;

	const DEV_PARAM_STORAGE_INFO* pcDevParamInfo = (const DEV_PARAM_STORAGE_INFO*)extgdbdevGetDeviceSettingInfo( );

	MEM_DUMP(0, "Nadd", &pcDevParamInfo->st2nodeAddrInfo.ubaNodeAddr, 8);
	PRINTF("\n");
	/* Neighbors */
	PRINTF("Neighbors\n");
	for(nbr = nbr_table_head(ds6_neighbors);nbr != NULL;nbr = nbr_table_next(ds6_neighbors, nbr)) 
	{
		entry_size = sizeof(i) + sizeof(uip_ipaddr_t) + sizeof(uip_lladdr_t) + sizeof(nbr->state);
		// PRINTF("%02u: ", i);
		PRINT6ADDR_U(&nbr->ipaddr);
		PRINTF(" - ");
		//      PRINTLLADDR_U(&nbr->lladdr);
		PRINTF(" - %u\n", nbr->state);
		udwNeighbors++;
	}
	PRINTF("Neighbors is %d\n", udwNeighbors);

	PRINTF("\n");

	PRINTF("Routing table\n");
	rt = uip_ds6_route_head();
	PRINTF("Routing num is %d\r\n",uip_ds6_route_num_routes());
	for(i = 0; i < uip_ds6_route_num_routes(); i++) 
	{
		u_char uaddr[16] = {0};
		if(rt != NULL) 
		{
			entry_size = sizeof(i) + sizeof(rt->ipaddr)
			+ sizeof(rt->length)
			+ sizeof(rt->state.lifetime)
			+ sizeof(rt->state.learned_from);

			PRINT6ADDR_U(&rt->ipaddr);
			memcpy(uaddr, &rt->ipaddr.u8[8], 8);
			uaddr[0] ^= 0x02; 
			PRINTF(" 1-%02x", rt->length);
			PRINTF(" 2-");
			PRINT6ADDR_U(uip_ds6_route_nexthop(rt));

			PRINTF(" 3-%08lx", rt->state.lifetime);

			PRINTF(" 4-%02x [%u]", rt->state.learned_from, entry_size);
			PRINTF(" m %02x%02x%02x%02x%02x%02x%02x%02x\n", uaddr[7], uaddr[6], uaddr[5], uaddr[4], uaddr[3],uaddr[2],uaddr[1],uaddr[0]);
			rt = uip_ds6_route_next(rt);

		}
	}
	PRINTF("\n");

	i = 0;
	PRINTF("Default Route\n");
	addr = uip_ds6_defrt_choose();
	if(addr != NULL) 
	{
		defrt = uip_ds6_defrt_lookup(addr);
	}

	//    i = buf[1];

	if(defrt != NULL && i < 1) 
	{
		entry_size = sizeof(i) + sizeof(defrt->ipaddr)
		+ sizeof(defrt->isinfinite);

		PRINT6ADDR_U(&defrt->ipaddr);
		PRINTF(" - %u\n", defrt->isinfinite);
	}
	PRINTF("\n");

	PRINTF("Unicast Addresses\n");
	for(i = 0; i < UIP_DS6_ADDR_NB; i++) 
	{
		if(uip_ds6_if.addr_list[i].isused) 
		{
			entry_size = sizeof(i) + sizeof(uip_ds6_if.addr_list[i].ipaddr);
			PRINT6ADDR_U(&uip_ds6_if.addr_list[i].ipaddr);
			PRINTF("\n");
		}
	}
	PRINTF("\n");

	return 0;
	#endif
}





//Init sys
/*
1  ->  hardware  init
2  ->  clock init
3  ->  debug uart init, shell init
*/
void sysInit(void)
{
	OSInitSys( );//Init system hardware io, uart, interrupt
	//OS clock ,when use some funtion of the clock, must after clocd_init
	clock_init(); 
	
	//System debug uart init
	//dbg_setup_uart();  							//
	Uart_Init(1);
	#if 0
	dbguart_DMA_Configuration( );
	dbguart_DMA_NVIC_Config( );
	xdev_out(uart_dma_putchar);
	#else
	//Uart_Std_Init( );							//open std uart ->uart1
	xdev_out(dbg_send_char);//set std output
	#endif
	Uart_StdSetInput(serial_line_input_byte);	//For shell read data  
#if 1
	//usart2 interface
	Uart_Init(2);
	Uart_Init(4);
	Uart_Init(3);
	Uart_Init(5);
	//Uart_Init(3);
	#if 1
	//dbguart_DMA_Configuration( );
	//dbguart_DMA_NVIC_Config( );
	#endif
#endif
	
	XPRINTF((0,"Initialising\r\n"));

	rtimer_init( );
	
	process_init( );
	process_start(&etimer_process, NULL);
	ctimer_init( );
	
	serial_line_init( );//For shell
	serial_shell_init( );
	shell_init( );

}

const u_char  node_link_addr[8] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x00, 0x00};
//global save node rf param,when modify; when node before reboot, node rf param use this config
static  RF_NODE_PARAM_CONFIG stRfParamCfg ={
	NODE_TYPE_LEAF,
	RF_TX_POWER_20DBM,
	60,
	0,
	0xABCD
};


RF_NODE_PARAM_CONFIG * get_node_rf_param(void)
{
	return &stRfParamCfg;
}



void set_link_addr(void)
{
	#if 1
	const DEV_PARAM_STORAGE_INFO* pcDevParamInfo = (const DEV_PARAM_STORAGE_INFO*)extgdbdevGetDeviceSettingInfo( );
	if (NULL != pcDevParamInfo)
	{
		if (pcDevParamInfo->udwProtectWord == 0x5aa5)
		{
			linkaddr_set_node_addr((linkaddr_t*)&pcDevParamInfo->st2nodeAddrInfo.ubaNodeAddr[0]);
			//copy node rf config param
			memcpy(&stRfParamCfg, &pcDevParamInfo->st1NodeConfig, sizeof(RF_NODE_PARAM_CONFIG));
			
			XPRINTF((2, "set addr success\r\n"));
		}
		else
		{
			linkaddr_set_node_addr((linkaddr_t*)&node_link_addr[0]);
		}
		MEM_DUMP(2, "Nadd", &linkaddr_node_addr, 8);
		memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
	}
	else
	{
		linkaddr_set_node_addr((linkaddr_t*)&node_link_addr[0]);
		memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
	}
	#endif
}

/*---------------------------------------------------------------------------*/
void uip_log(char *m)
{
  PRINTF("%s\n", m);
}


u_short get_random_seed(void)
{
	const NODE_ADDR_INFO* pcNodeAddrInfo = (const NODE_ADDR_INFO*)extgdbdevGetDeviceSettingInfoSt(LABLE_ADDR_INFO);
	u_short uwRandomSeed = 0;
	int i = 0;
	if (NULL != pcNodeAddrInfo)
	{
		for (i = 0; i < 8; i++)
		{
			uwRandomSeed = crc16_data((const unsigned char * )&pcNodeAddrInfo->ubaNodeAddr[0], 8, 0);
		}
	}

	return uwRandomSeed;
}


void init_app(void)
{
	const RF_NODE_PARAM_CONFIG* pcNodeRfParamCfg = (const RF_NODE_PARAM_CONFIG*)extgdbdevGetDeviceSettingInfoSt(LABLE_RF_NODE_PARAM);
	
	if (pcNodeRfParamCfg->ubNodeType == NODE_TYPE_CENTER)
	{
		XPRINTF((0, "1 center node\r\n"));
		process_start(&unicast_receiver_root, NULL);
		
	}
	else 
	{
		XPRINTF((0, "2 leaf node\r\n"));
		process_start(&receiver_node_process,NULL);	
	}
	//serial_uart_init( );
}


PROCESS(user_ip_start, "user_ip");
PROCESS_THREAD(user_ip_start, ev, data)
{
	static struct etimer et;
	u_long udwWaitTime = 0;
 	const DEV_PARAM_STORAGE_INFO* pcDevParamInfo = NULL;
	PROCESS_BEGIN( );	
	XPRINTF((10, "user_ip\r\n"));

	pcDevParamInfo = (const DEV_PARAM_STORAGE_INFO*)extgdbdevGetDeviceSettingInfo( );
	
	#if 0
	if (pcDevParamInfo->st1NodeConfig.ubNodeType == NODE_TYPE_LEAF)
	{
		udwWaitTime = get_random_seed( )%(30*1000);
		etimer_set(&et, udwWaitTime);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}
	#endif
	XPRINTF((10, "start tcp\r\n"));
	set_link_addr( );	
	netstack_init( );
	XPRINTF((10, "net finish\r\n"));
	queuebuf_init( );
	process_start(&tcpip_process, NULL);
	//init_app( );
	process_exit(&user_ip_start);
	PROCESS_END( );
}


int main(void)
{
	u_short uwRandrom = 0;;
	sysInit();

	uwRandrom = get_random_seed( );
	XPRINTF((0, "randdomseed = %d \r\n", uwRandrom));
	random_init( uwRandrom );
	//random_init( 100 );
	
	process_start(&blink_process,NULL);
	process_start(&feeddog_process, NULL);

	process_start(&user_ip_start, NULL);

	//grps
	sim900a_init( );
	//rf
	fireAppInit( );

	initAppKey( );
	//net
	app_enc28j60_init( );
	app485Init( );
	mp3init( );
	XPRINTF((0,"Processes running\r\n"));
	while(1) 
	{
		#if 1
		do 
		{
		}while(process_run() > 0);
		#else
		#endif
	}
	return 0;
}



