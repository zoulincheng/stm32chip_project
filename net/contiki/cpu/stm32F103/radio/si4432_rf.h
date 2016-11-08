/**
 * \file
 *         SI4432 RF driver header file
 * \author
 *         Zach Shelby <zach@sensinode.com>
 */

#ifndef __SI4432_RF_H__
#define __SI4432_RF_H__

#include "contiki.h"
#include "dev/radio.h"
#define SI4432_MAX_PACKET_LEN      127

extern const struct radio_driver si4432_radio_driver;



/// si4432 state
enum SI4432_STATE {
//  SI4432_OFF = 0,
  SI4432_RX  = 0x01,
  SI4432_TX  = 0x02,
  
  SI4432_IDLE = 0x10,		// searching for preamble + sync word
  SI4432_RX_RECEIVING = 0x20,		// receiving bytes
  SI4432_RX_PRE = 0x40,		// 
  SI4432_OP_STATE = 0x73,
  
//  SI4432_TURN_OFF = 0x80,
};

#define SI4432_SET_OPSTATE(opstate)		si4432_state = ((si4432_state & ~SI4432_OP_STATE) | (opstate))

/* Constants */
typedef enum rf_address_mode_t {
  RF_DECODER_NONE = 0,
  RF_DECODER_COORDINATOR,
  RF_SOFTACK_MONITOR,
  RF_MONITOR,
  RF_SOFTACK_CLIENT,
  RF_DECODER_ON
} rf_address_mode_t;


#define SI4432_SUCCESS	(0x00)
/**
 * @brief The expected ACK was received after the last transmission.
 */
#define SI4432_PHY_ACK_RECEIVED (0x8F)

/**
 * @brief We expected to receive an ACK following the transmission, but
 * the MAC level ACK was never received.
 */
#define SI4432_MAC_NO_ACK_RECEIVED	(0x40)


PROCESS_NAME(si4432_radio_process);
PROCESS_NAME(si4432_txled_process);
PROCESS_NAME(si4432_rxled_process);
PROCESS_NAME(si4432_rfled_process);
PROCESS_NAME(timer_wait_random);


PROCESS_NAME(si4432_channel_scan);
typedef struct {
  u_char waitForAck;       // Wait for ACK if ACK request set in FCF.
  u_char checkCca;         // backoff and check CCA before transmit.
  u_char ccaAttemptMax;      // The number of CCA attempts before failure;
  u_char backoffExponentMin; // Backoff exponent for the initial CCA attempt.
  u_char backoffExponentMax; // Backoff exponent for the final CCA attempt(s).
  u_char appendCrc;        // Append CRC to transmitted packets.
} RadioTransmitConfig;

int si4432_radio_send(const void *payload, unsigned short payload_len);

#endif

