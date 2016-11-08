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
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"

#include "net/ip/simple-udp.h"

#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "sysprintf.h"

#define UDP_PORT 1234

#define MCAST_SINK_UDP_PORT 3001 /* Host byte order */

static struct simple_udp_connection unicast_connection;

static struct uip_udp_conn *mcast_conn;

static void prepare_mcast(void)
{
	uip_ipaddr_t ipaddr;

	/*
	* IPHC will use stateless multicast compression for this destination
	* (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
	*/
	uip_ip6addr(&ipaddr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
	mcast_conn = udp_new(&ipaddr, UIP_HTONS(MCAST_SINK_UDP_PORT), NULL);
}

void multicast_send(const unsigned char *pData, int dataLen)
{
	PRINTF("Send to: ");
	PRINT6ADDR_U(&mcast_conn->ripaddr);
	uip_udp_packet_send(mcast_conn, pData, dataLen);
}

/*---------------------------------------------------------------------------*/
uip_ds6_maddr_t *join_mcast_group_node(void)
{
	uip_ipaddr_t addr;
	uip_ds6_maddr_t *rv;

	/*
	* IPHC will use stateless multicast compression for this destination
	* (M=1, DAC=0), with 32 inline bits (1E 89 AB CD)
	*/
	uip_ip6addr(&addr, 0xFF1E,0,0,0,0,0,0x89,0xABCD);
	rv = uip_ds6_maddr_add(&addr);

	if(rv) 
	{
		PRINTF("Joined multicast group ");
		PRINT6ADDR_U(&uip_ds6_maddr_lookup(&addr)->ipaddr);
		PRINTF("\n");

		/**/
		mcast_conn = udp_new(NULL, UIP_HTONS(0), NULL);
		udp_bind(mcast_conn, UIP_HTONS(MCAST_SINK_UDP_PORT));

		PRINTF("Listening: ");
		PRINT6ADDR_U(&mcast_conn->ripaddr);
		PRINTF(" local/remote port %u/%u\n",
		UIP_HTONS(mcast_conn->lport), UIP_HTONS(mcast_conn->rport));
	}
	return rv;
}



/*---------------------------------------------------------------------------*/
PROCESS(unicast_receiver_root, "Unicast receiver root process");

struct simple_udp_connection* get_unicast_conn(void)
{
	return &unicast_connection;
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
  XPRINTF((10,"Data from"));
  //PRINT6ADDR_U(sender_addr);
  XPRINTF((10,"on port %d from port %d with length %d:  time %d\n",receiver_port, sender_port, datalen, clock_time( )));
  MEM_DUMP(7, "<-w", data, datalen);
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

  PRINTF("IPv6 addresses: ");
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
static void
create_rpl_dag(uip_ipaddr_t *ipaddr)
{
  struct uip_ds6_addr *root_if;

  root_if = uip_ds6_addr_lookup(ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    uip_ipaddr_t prefix;
    
    rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
    dag = rpl_get_any_dag();
    uip_ip6addr(&prefix, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &prefix, 64);
    XPRINTF((12, "created a new RPL dag\r\n"));
  } else {
    XPRINTF((12, "failed to create a new RPL DAG\n"));
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_receiver_root, ev, data)
{
  uip_ipaddr_t *ipaddr;

  PROCESS_BEGIN();

  ipaddr = set_global_address();

  create_rpl_dag(ipaddr);

  prepare_mcast( );

#if 1
  simple_udp_register(&unicast_connection, UDP_PORT,
                      NULL, UDP_PORT, receiver);
#endif
  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
