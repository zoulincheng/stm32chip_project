#include "contiki.h"
#include "net/linkaddr.h"

#include "stm32f0xx.h"
#include "basictype.h"
#include <stdint.h>

#include <debug-uart.h>

#include "xprintf.h"
#include "sysprintf.h"
#include "rtimer-arch.h"


#include "net/rime/rime.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"

#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"

#include "dev/watchdog.h"


extern uip_ds6_netif_t uip_ds6_if;
static uip_ds6_route_t *rt;
static uip_ds6_defrt_t *defrt;
static uip_ipaddr_t *addr;

/*---------------------------------------------------------------------------*/
uint8_t process_request_u(void) 
{
	uint8_t len;
	uint8_t count; /* How many did we pack? */
	uint8_t i;
	uint8_t left;
	uint8_t entry_size;
	uip_ds6_nbr_t *nbr;
	u_long udwNeighbors = 0;

	PRINTF(...)("\n");
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
		if(rt != NULL) 
		{
			entry_size = sizeof(i) + sizeof(rt->ipaddr)
			+ sizeof(rt->length)
			+ sizeof(rt->state.lifetime)
			+ sizeof(rt->state.learned_from);

			PRINT6ADDR_U(&rt->ipaddr);
			PRINTF(" 1- %02x", rt->length);
			PRINTF(" 2- ");
			PRINT6ADDR_U(uip_ds6_route_nexthop(rt));

			PRINTF(" 3- %08lx", rt->state.lifetime);

			PRINTF(" 4- %02x [%u]\n", rt->state.learned_from, entry_size);

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
}



/*
This define is used to debug
*/
//#define ROOT_NODE1		
//#define NODE2
//#define NODE3
//#define NODE4
//#define NODE5
//#define NODE6
//#define NODE7
//#define NODE8
//#define NODE9
//#define NODE10
//#define NODE11


#ifdef ROOT_NODE1
linkaddr_t rime_node_macaddr={0x01,0x00,0x01,0x67,0x89,0x22,0x00,0x00}; //node 1
#endif

#ifdef NODE2
linkaddr_t rime_node_macaddr={0x20,0x23,0x45,0x67,0x89,0x22,0x22,0x22}; //node 2
#endif

#ifdef NODE3
linkaddr_t rime_node_macaddr={0x30,0x23,0x45,0x67,0x89,0x22,0x22,0x22}; //node 3
#endif



static uip_ipaddr_t uipaddr;


//This funtion set node global addr, this node must not be root node
/*---------------------------------------------------------------------------*/
static void set_node_global_address(void)
{
	uip_ipaddr_t ipaddr;

	uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
	uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
	uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
	PRINTF("node global addr: ");
	PRINT6ADDR_U(&ipaddr);
}

//const u_char  node_link_addr[8] = {0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99, 0x99};

void set_link_addr(void)
{

	linkaddr_set_node_addr((linkaddr_t*)&node_link_addr[0]);
	memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
}




u_short get_random_seed(void)
{
	u_short uwRandomSeed = 0;

	return uwRandomSeed;
}

void start_node_main_process(void)
{

#if 0
//			set_link_addr( );	
//			netstack_init( );
//			queuebuf_init( );
//			process_start(&tcpip_process, NULL);

			//1 root
			process_start(&unicast_receiver_root, NULL);
			process_start(&center_frame_process, NULL);
 
			//2
			XPRINTF((0, "2 leaf node\r\n"));
			process_start(&user_ip_start, NULL);
			process_start(&receiver_node_process,NULL);	
			process_start(&app_frame_process, NULL);
#endif
}

PROCESS(user_ip_start, "user_ip");
PROCESS_THREAD(user_ip_start, ev, data)
{
	static struct etimer et;
	static u_long udwWaitTime = 0;
	PROCESS_BEGIN( );	
	XPRINTF((10, "user_ip\r\n"));

	set_link_addr( );	
	netstack_init( );
	queuebuf_init( );
	process_start(&tcpip_process, NULL);
	
	start_node_main_process( );
	
	process_exit(&user_ip_start);
	PROCESS_END( );
}

int main(void)
{
	u_short uwRandrom = 0;;

	uwRandrom = get_random_seed( );
	XPRINTF((0, "randdomseed = %d \r\n", uwRandrom));
	random_init( 100 );

#if 0
	set_link_addr( );	
	netstack_init( );
	queuebuf_init( );
	process_start(&tcpip_process, NULL);
	start_node_main_process( );
#else
	process_start(&user_ip_start, NULL);
#endif
	
	XPRINTF((0,"Processes running\r\n"));
	while(1) 
	{
		#if 0	
		#else
	    process_run();
    	etimer_request_poll();
   	   #endif
	}
	return 0;
}



