#ifndef CONTIKI_CONF_H_CDBB4VIH3I__

#define CONTIKI_CONF_H_CDBB4VIH3I__

#include <stdint.h>
//#define CC_CONF_NO_VA_ARGS   0
#define CCIF
#define CLIF

//#define NETSTACK_CONF_WITH_IPV4 1
#define WITH_ASCII 1

#define CLOCK_CONF_SECOND 1000

/* These names are deprecated, use C99 names. */
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

/* Platform typedefs */
typedef unsigned int clock_time_t;
typedef unsigned int uip_stats_t;

/*
 * rtimer.h typedefs rtimer_clock_t as unsigned short. We need to define
 * RTIMER_CLOCK_LT to override this
 */
typedef uint32_t rtimer_clock_t;
#define RTIMER_CLOCK_LT(a,b)     ((int32_t)((a)-(b)) < 0)

#ifndef BV
#define BV(x) (1<<(x))
#endif


/* NETSTACK_CONF_WITH_IPV6 specifies  IPv6  should be used. */
//#ifndef NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_WITH_IPV6 1
//#endif /* NETSTACK_CONF_WITH_IPV6 */
#define UIP_CONF_IPV6_RPL		1
/*---------------------------------------------------------------------------*/
/**
 * \name Network Stack Configuration
 *
 * @{
 */
#ifndef NETSTACK_CONF_NETWORK
#if NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_NETWORK sicslowpan_driver
#else
#define NETSTACK_CONF_NETWORK rime_driver
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* NETSTACK_CONF_NETWORK */

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
//#define NETSTACK_CONF_MAC     nullmac_driver

#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_driver
#endif

/* Configure NullRDC for when it's selected */
//#define NULLRDC_802154_AUTOACK                  1
//#define NULLRDC_802154_AUTOACK_HW               1



#define NULLRDC_CONF_802154_AUTOACK		1
#define NULLRDC_CONF_802154_AUTOACK_HW	1
#define NULLRDC_CONF_SEND_802154_ACK	1
//#define NULLRDC_CONF_ACK_WAIT_TIME

/* Configure ContikiMAC for when it's selected */
#define CONTIKIMAC_CONF_WITH_PHASE_OPTIMIZATION 0
#define WITH_FAST_SLEEP 

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE    4
#endif

#ifndef NETSTACK_CONF_FRAMER
#if NETSTACK_CONF_WITH_IPV6
#define NETSTACK_CONF_FRAMER  framer_802154
#else /* NETSTACK_CONF_WITH_IPV6 */
#define NETSTACK_CONF_FRAMER  contikimac_framer
#endif /* NETSTACK_CONF_WITH_IPV6 */
#endif /* NETSTACK_CONF_FRAMER */

//#define NETSTACK_CONF_RADIO   si4432_radio_driver
#define NETSTACK_CONF_RADIO   nullradio_driver

/** @} */

/*---------------------------------------------------------------------------*/
/**
 * \name RF configuration
 *
 * @{
 */
/* RF Config */
#ifndef IEEE802154_CONF_PANID
#define IEEE802154_CONF_PANID           0xABCD
#endif
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name IPv6, RIME and network buffer configuration
 *
 * @{
 */



#if NETSTACK_CONF_WITH_IPV6
/* Addresses, Sizes and Interfaces */
/* 8-byte addresses here, 2 otherwise */
#define LINKADDR_CONF_SIZE                   8
#define UIP_CONF_LL_802154                   1
#define UIP_CONF_LLH_LEN                     0
#define UIP_CONF_NETIF_MAX_ADDRESSES         3

/* TCP, UDP, ICMP */
#ifndef UIP_CONF_TCP
#define UIP_CONF_TCP                         1
#endif
#ifndef UIP_CONF_TCP_MSS
#define UIP_CONF_TCP_MSS                    64
#endif
#define UIP_CONF_UDP                         1
#define UIP_CONF_UDP_CHECKSUMS               1
#define UIP_CONF_ICMP6                       1

/* ND and Routing */
#ifndef UIP_CONF_ROUTER
#define UIP_CONF_ROUTER                      1
#endif


#define UIP_CONF_ND6_SEND_NA				0   
#define UIP_CONF_ND6_SEND_RA                 0
#define UIP_CONF_IP_FORWARD                  0
#define RPL_CONF_STATS                       0
#define RPL_CONF_MAX_DAG_ENTRIES             1
#ifndef RPL_CONF_OF
#define RPL_CONF_OF rpl_mrhof
#endif

#define RPL_CONF_DIO_INTERVAL_DOUBLINGS		10
//#define RPL_CONF_DIO_INTERVAL_MIN        	16  //default is 12 4s, when is 16 is 65s



//#define UIP_CONF_ND6_REACHABLE_TIME     600000
#define UIP_CONF_ND6_REACHABLE_TIME     (600000*3)
#define UIP_CONF_ND6_RETRANS_TIMER       30000

#ifndef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS                2
#endif
#ifndef UIP_CONF_MAX_ROUTES
#define UIP_CONF_MAX_ROUTES                 		2
#endif

/* uIP */
#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE              140
#endif

//#define UIP_CONF_IPV6_QUEUE_PKT              1
#define UIP_CONF_IPV6_QUEUE_PKT              0

#define UIP_CONF_IPV6_CHECKS                 1
#define UIP_CONF_IPV6_REASSEMBLY             0
#define UIP_CONF_MAX_LISTENPORTS             8

/* 6lowpan */
#define SICSLOWPAN_CONF_COMPRESSION          SICSLOWPAN_COMPRESSION_HC06
#ifndef SICSLOWPAN_CONF_COMPRESSION_THRESHOLD
#define SICSLOWPAN_CONF_COMPRESSION_THRESHOLD 63
#endif
#ifndef SICSLOWPAN_CONF_FRAG
#define SICSLOWPAN_CONF_FRAG                 1
#endif
#define SICSLOWPAN_CONF_MAXAGE               8


/* Define our IPv6 prefixes/contexts here */
#define SICSLOWPAN_CONF_MAX_ADDR_CONTEXTS    1
#ifndef SICSLOWPAN_CONF_ADDR_CONTEXT_0
#define SICSLOWPAN_CONF_ADDR_CONTEXT_0 { \
  addr_contexts[0].prefix[0] = 0xaa; \
  addr_contexts[0].prefix[1] = 0xaa; \
}
#endif
#define MAC_CONF_CHANNEL_CHECK_RATE          2

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    6
#endif
/*---------------------------------------------------------------------------*/
#else /* NETSTACK_CONF_WITH_IPV6 */
/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1

#ifndef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE               108
#endif

#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0

#ifndef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM                    8
#endif

#endif /* NETSTACK_CONF_WITH_IPV6 */
/** @} */


//#define RAND_MAX 0x7fff

/*---------------------------------------------------------------------------*/
#endif /* CONTIKI_CONF_H_ */
/** @} */




