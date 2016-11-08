/*
 * Copyright (c) 2012, Thingsquare, www.thingsquare.com.
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
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
//#include "net/uip.h"
#include "net/ip/uip.h"
//#include "net/uip-ds6.h"
#include "net/ipv6/uip-ds6.h"
//#include "net/uip-debug.h"

//#include "net/simple-udp.h"
#include "net/ip/simple-udp.h"

#include "net/rpl/rpl.h"
#include "dev/leds.h"

#include <stdio.h>
#include <string.h>


#include "sysprintf.h" 
#include "basictype.h"
#define UDP_PORT 1234

static struct simple_udp_connection unicast_connection;
static uip_ipaddr_t ip_to_goal;



/*---------------------------------------------------------------------------*/
PROCESS(receiver_node_process, "Receiver node");

struct simple_udp_connection* get_leafnode_unicast_conn(void)
{
	return &unicast_connection;
}

//get sender ip
void get_senderip(uip_ipaddr_t *piouipAddr, const uip_ipaddr_t *pcSenderIp)
{
	if ((NULL != piouipAddr)&&(NULL != pcSenderIp))
	{
		memcpy(piouipAddr, pcSenderIp, sizeof(uip_ipaddr_t));
	}
}

uip_ipaddr_t *get_goal_uipaddr(void)
{
	return (uip_ipaddr_t *)&ip_to_goal;
}

//get any local ip addr 
static uip_ipaddr_t* getLocalIpAddr(void)
{
	int i = 0;
	for(i = 0; i < UIP_DS6_ADDR_NB; i++) 
	{
		if(uip_ds6_if.addr_list[i].isused) 
		{
			//PRINT6ADDR_U(&uip_ds6_if.addr_list[i].ipaddr);
			return &uip_ds6_if.addr_list[i].ipaddr;
		}
	}
	return NULL;
}

/*---------------------------------------------------------------------------*/
static void
receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
	uip_ipaddr_t *puipLocalAddr =  getLocalIpAddr( );

	XPRINTF((10,"Data received from "));
	//PRINT6ADDR_U(sender_addr);
	get_senderip(&ip_to_goal, sender_addr);	
	XPRINTF((10," on port %d from port %d with length %d: \n",receiver_port, sender_port, datalen));
	
	MEM_DUMP(6 , "<-w", data, datalen);
	//app_3762_get_udp_data(data, datalen);
}
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  XPRINTF((0,"IPv6 addresses: "));
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      PRINT6ADDR_U(&uip_ds6_if.addr_list[i].ipaddr);
      PRINTF("\n");
    }
  }

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
uint8_t should_blink = 1;
static void
route_callback(int event, uip_ipaddr_t *route, uip_ipaddr_t *ipaddr, int num_routes)
{
  if(event == UIP_DS6_NOTIFICATION_DEFRT_ADD) {
    should_blink = 0;
  } else if(event == UIP_DS6_NOTIFICATION_DEFRT_RM) {
    should_blink = 1;
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(receiver_node_process, ev, data)
{
//  static struct etimer et;
  static struct uip_ds6_notification n;
  uip_ipaddr_t *ipaddr;
  static uip_ipaddr_t *ipladdr;
  static struct rpl_dag* prpl_dag;

  PROCESS_BEGIN();

  ipaddr = set_global_address();
  ipladdr = getLocalIpAddr( );
  prpl_dag = rpl_get_any_dag( );

  uip_ds6_notification_add(&n, route_callback);

  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  join_mcast_group_node( );
 // etimer_set(&et, CLOCK_SECOND*120);
  while(1) {
  	#if 0
   	ipladdr = getLocalIpAddr( );
  	prpl_dag = rpl_get_any_dag( );
 	
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    etimer_set(&et, 60*CLOCK_SECOND+(random_rand()%10)*CLOCK_SECOND + (random_rand()%20)*CLOCK_CONF_SECOND + (random_rand()%30)*CLOCK_SECOND);
    if ( NULL != ipladdr && NULL != prpl_dag)
	{
		// XPRINTF((0, "senddata*dfdsafds**\r\n"));

		ubNodeId = (ipladdr->u8[8] >> 4);
		xsprintf(buf, "msg %d from node %d", ubmsgid,ubNodeId);
		simple_udp_sendto(&unicast_connection, buf, strlen(buf) + 1, (const uip_ipaddr_t *)&(prpl_dag->dag_id));
	}
    ubmsgid++;
    #else
    PROCESS_YIELD( );
    if (ev == tcpip_event)
    {
    	if(uip_newdata())
    	{
			MEM_DUMP(6 , "<-w", uip_appdata, uip_datalen());
			//app_3762_frame_process(ubaBuf3762, data);
			//app_3762_get_udp_data(uip_appdata, uip_datalen());    	
		}
    }
    #endif

#if 0
   if(should_blink) {
      //leds_on(LEDS_ALL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
      etimer_reset(&et);
      //leds_off(LEDS_ALL);
    }
 #endif
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
