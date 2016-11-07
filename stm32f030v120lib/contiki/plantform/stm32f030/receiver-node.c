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


#define FRAME_3762_BUF_NUM		128
static struct simple_udp_connection unicast_connection;

static uip_ipaddr_t ip_to_goal;

static u_char ubaBuf3762[FRAME_3762_BUF_NUM] = {0};


//PROCESS_NAME(app_frame_process);
/*---------------------------------------------------------------------------*/
PROCESS(receiver_node_process, "Receiver node");
//AUTOSTART_PROCESSES(&receiver_node_process);


#define	DEBUG_DATA_PRT	0

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
  static struct etimer et;
  static struct uip_ds6_notification n;
  uip_ipaddr_t *ipaddr;
  static uip_ipaddr_t *ipladdr;
  static struct rpl_dag* prpl_dag;
//  static u_char ubmsgid = 0;
 // u_char ubNodeId = 20;
 // u_char buf[30];
  

  PROCESS_BEGIN();

  ipaddr = set_global_address();
  ipladdr = getLocalIpAddr( );
  prpl_dag = rpl_get_any_dag( );

  uip_ds6_notification_add(&n, route_callback);

  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);

  etimer_set(&et, CLOCK_SECOND*120);
  while(1) 
  {
	PROCESS_YIELD( );
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/