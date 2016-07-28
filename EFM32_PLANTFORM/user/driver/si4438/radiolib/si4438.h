#ifndef _SI4438_H
#define _SI4438_H



#define   IS_TRUE		1
#define   IS_FALSE		0

#define   RF_SENT_SUCCESS	0
#define   RF_SENT_FAILED	-1

//INT_CTL_PH_ENABLE
#define SI446X_INT_CTL_PH_FLTOK_EN				0x80 //#define RF_INT_CTL_PH_FILTER_MATCH_EN				0x80
#define SI446X_INT_CTL_PH_FLMISS_EN				0x40//#define RF_INT_CTL_PH_FILTER_MISS_EN				0x40
#define SI446X_INT_CTL_PH_PSENT_EN				0x20//#define RF_INT_CTL_PH_PACKET_SENT_EN				0x20
#define SI446X_INT_CTL_PH_PRX_EN				0x10//#define RF_INT_CTL_PH_PACKET_RX_EN					0x10
#define SI446X_INT_CTL_PH_CRCE_EN				0x08//#define RF_INT_CTL_PH_CRC_ERROR_EN					0x08
#define SI446X_INT_CTL_PH_ACRC_EN				0x04//#define RF_INT_CTL_PH_ALT_CRC_ERROR_EN				0x04
#define SI446X_INT_CTL_PH_TFAE_EN				0x02//#define RF_INT_CTL_PH_TX_FIFO_ALMOST_EMPTY_EN		0x02
#define SI446X_INT_CTL_PH_RFAF_EN				0x01//#define RF_INT_CTL_PH_RX_FIFO_ALMOST_FULL_EN		0x01

//INT_CTL_MODEM_ENABLE
//#define SI446X_INT_CTL_MODEM_RSSI_LATCH_EN				0x80//#define RF_INT_CTL_MODEM_RSSI_LATCH_EN				0x80
#define SI446X_INT_CTL_MODEM_PODE_EN			0x40//#define RF_INT_CTL_MODEM_POSTAMBLE_DETECT_EN		0x40
#define SI446X_INT_CTL_MODEM_INSY_EN			0x20//#define RF_INT_CTL_MODEM_INVALID_SYNC_EN			0x20
#define SI446X_INT_CTL_MODEM_RSSIJ_EN			0x10//#define RF_INT_CTL_MODEM_RSSI_JUMP_EN				0x10
#define SI446X_INT_CTL_MODEM_RSSI_EN			0x08//#define RF_INT_CTL_MODEM_RSSI_EN					0x08
#define SI446X_INT_CTL_MODEM_INPR_EN			0x04//#define RF_INT_CTL_MODEM_INVALID_PREAMBLE_EN		0x04
#define SI446X_INT_CTL_MODEM_PRDE_EN			0x02//#define RF_INT_CTL_MODEM_PREAMBLE_DETECT_EN			0x02
#define SI446X_INT_CTL_MODEM_SYDE_EN			0x01//#define RF_INT_CTL_MODEM_SYNC_DETECT_EN				0x01

//INT_CTL_CHIP_ENABLE
#define SI446X_INT_CTL_CHIP_CAL_EN				0x40
#define SI446X_INT_CTL_CHIP_FUOE_EN				0x20//#define INT_CTL_CHIP_FIFO_UNDERFLOW_OVERFLOW_ERROR_EN	0x20
#define SI446X_INT_CTL_CHIP_STCH_EN				0x10//#define INT_CTL_CHIP_STATE_CHANGE_EN					0x10
#define SI446X_INT_CTL_CHIP_CMERR_EN			0x08//#define INT_CTL_CHIP_CMD_ERROR_EN						0x08
#define SI446X_INT_CTL_CHIP_CHRE_EN				0x04//#define INT_CTL_CHIP_CHIP_READY_EN						0x04
#define SI446X_INT_CTL_CHIP_LBY_EN				0x02//#define INT_CTL_CHIP_LOW_BATT_EN						0x02
#define SI446X_INT_CTL_CHIP_WUT_EN				0x01//#define INT_CTL_CHIP_WUT_EN								0x01

#define SI446X_FIFO_SIZE			64
#define SI446X_FIFO_TFAE_TH			10
#define SI446X_FIFO_RFAF_TH			54
#define SI446X_FIFO_TFAF_TH			48


/// si4438 state
typedef enum _SI446X_STATE {
  SI446X_OFF = 0,
  SI446X_RX  = 0x01,
  SI446X_TX  = 0x02,
  
  SI446X_IDLE = 0x10,		// searching for preamble + sync word
  SI446X_RX_RECEIVING = 0x20,		// receiving bytes
  SI446X_RX_PRE = 0x40,		// 
  SI446X_OP_STATE = 0x73,
  
  SI446X_TURN_OFF = 0x80,
}SI446X_STATE;

typedef enum _SI446X_DEV_STATE
{
	SI446X_SLEEP = 0x01,//not applicable
	SI446X_SPI_ACTIVE = 0x02,
	SI446X_READY = 0x03,
	SI446X_READY2 = 0x04,
	SI446X_TX_TUNE = 0x05,
	SI446X_RX_TUNE = 0x06,
	SI446X_TX_STATE	= 0x07,
	SI446X_RX_STATE = 0x08
}SI446X_DEV_STATE;


//#define WIRE_LESS_40KBPS
#define WIRE_LESS_9_6KBPS



#ifdef WIRE_LESS_40KBPS
#define  SI4432_WAIT_INT_TIME		6000
#define  SI4432_WAIT_PKSENT_TIME	2000
#define  SI4432_WAIT_TXFFAEM_TIME	2000
#define  SEND_PACKET_TIMEOUT		20//20ms(240000)
#define  SEND_128PACKET_TIME		30//30ms
#define  SEND_WAIT_TXFIFO_EMPTY		20//20ms(120000)
#endif

#ifdef WIRE_LESS_9_6KBPS
#define  SI4432_WAIT_INT_TIME		6000
#define  SI4432_WAIT_PKSENT_TIME	2000
#define  SI4432_WAIT_TXFFAEM_TIME	2000
#define  SEND_PACKET_TIMEOUT		700//70ms(720000)
#define  SEND_128PACKET_TIME		140//140ms
#define  SEND_WAIT_TXFIFO_EMPTY		700//70ms(480000)
#endif


void si446x_set_pksent(void);
void si446xRadioInit(void);
void si446xStartRX(void);
void si446xReadMutiData(u_char *pBuf);
int si446xRadioSendFixLenData(const u_char *pubdata);
#if 0
/* This section contains command map declarations */
typedef struct _si446x_reply_GENERIC_map {
        U8  REPLY[16];
}SI446X_REPLY_GENERIC_MAP;

typedef struct _si446x_reply_PART_INFO_map {
        U8  CHIPREV;
        U16  PART;
        U8  PBUILD;
        U16  ID;
        U8  CUSTOMER;
        U8  ROMID;
}SI446X_REPLY_PART_INFO_MAP;

typedef struct _si446x_reply_FUNC_INFO_map {
        U8  REVEXT;
        U8  REVBRANCH;
        U8  REVINT;
        U8  FUNC;
}SI446X_REPLY_FUNC_INFO_MAP;

typedef struct _si446x_reply_GET_PROPERTY_map {
        U8  DATA[16];
}SI446X_REPLY_GET_PROPERTY_MAP;

typedef struct _si446x_reply_GPIO_PIN_CFG_map {
        U8  GPIO[4];
        U8  NIRQ;
        U8  SDO;
        U8  GEN_CONFIG;
}SI446X_REPLY_GPIO_PIN_CFG_MAP;

typedef struct _si446x_reply_FIFO_INFO_map {
        U8  RX_FIFO_COUNT;
        U8  TX_FIFO_SPACE;
}SI446X_REPLY_FIFO_INFO_MAP;

typedef struct _si446x_reply_GET_INT_STATUS_map {
        U8  INT_PEND;
        U8  INT_STATUS;
        U8  PH_PEND;
        U8  PH_STATUS;
        U8  MODEM_PEND;
        U8  MODEM_STATUS;
        U8  CHIP_PEND;
        U8  CHIP_STATUS;
}SI446X_REPLY_GET_INT_STATUS_MAP;

typedef struct _si446x_reply_REQUEST_DEVICE_STATE_map {
        U8  CURR_STATE;
        U8  CURRENT_CHANNEL;
}SI446X_REPLY_REQUEST_DEVICE_STATE_MAP;

typedef struct _si446x_reply_READ_CMD_BUFF_map {
        U8  BYTE[16];
}SI446X_REPLY_READ_CMD_BUFF_MAP;

typedef struct _si446x_reply_FRR_A_READ_map {
        U8  FRR_A_VALUE;
        U8  FRR_B_VALUE;
        U8  FRR_C_VALUE;
        U8  FRR_D_VALUE;
}SI446X_REPLY_FRR_A_READ_MAP;

typedef struct _si446x_reply_FRR_B_READ_map {
        U8  FRR_B_VALUE;
        U8  FRR_C_VALUE;
        U8  FRR_D_VALUE;
        U8  FRR_A_VALUE;
}SI446X_REPLY_FRR_B_READ_MAP;

typedef struct _si446x_reply_FRR_C_READ_map {
        U8  FRR_C_VALUE;
        U8  FRR_D_VALUE;
        U8  FRR_A_VALUE;
        U8  FRR_B_VALUE;
}SI446X_REPLY_FRR_C_READ_MAP;

typedef struct _si446x_reply_FRR_D_READ_map {
        U8  FRR_D_VALUE;
        U8  FRR_A_VALUE;
        U8  FRR_B_VALUE;
        U8  FRR_C_VALUE;
}SI446X_REPLY_FRR_D_READ_MAP;

typedef struct _si446x_reply_IRCAL_MANUAL_map {
        U8  IRCAL_AMP_REPLY;
        U8  IRCAL_PH_REPLY;
}SI446X_REPLY_IRCAL_MANUAL_MAP;

typedef struct _si446x_reply_PACKET_INFO_map {
        U16  LENGTH;
}SI446X_REPLY_PACKET_INFO_MAP;


typedef struct _si446x_reply_GET_MODEM_STATUS_map {
        U8  MODEM_PEND;
        U8  MODEM_STATUS;
        U8  CURR_RSSI;
        U8  LATCH_RSSI;
        U8  ANT1_RSSI;
        U8  ANT2_RSSI;
        U16  AFC_FREQ_OFFSET;
}SI446X_REPLY_GET_MODEM_STATUS_MAP;

typedef struct _si446x_reply_READ_RX_FIFO_map {
        U8  DATA[2];
}SI446X_REPLY_READ_RX_FIFO_MAP;

typedef struct _si446x_reply_GET_ADC_READING_map {
        U16  GPIO_ADC;
        U16  BATTERY_ADC;
        U16  TEMP_ADC;
}SI446X_REPLY_GET_ADC_READING_MAP;

typedef struct _si446x_reply_GET_PH_STATUS_map {
        U8  PH_PEND;
        U8  PH_STATUS;
}SI446X_REPLY_GET_PH_STATUS_MAP;

typedef struct _si446x_reply_GET_CHIP_STATUS_map {
        U8  CHIP_PEND;
        U8  CHIP_STATUS;
        U8  CMD_ERR_STATUS;
        U8  CMD_ERR_CMD_ID;
}SI446X_REPLY_GET_CHIP_STATUS_MAP;


/* The union that stores the reply written back to the host registers */
typedef union _si446x_cmd_reply_union {
        U8												RAW[16];
        SI446X_REPLY_GENERIC_MAP						GENERIC;
        SI446X_REPLY_PART_INFO_MAP 						PART_INFO;				//struct si446x_reply_PART_INFO_map                                PART_INFO;
        SI446X_REPLY_FUNC_INFO_MAP 						FUNC_INFO;				//struct si446x_reply_FUNC_INFO_map                                FUNC_INFO;
        SI446X_REPLY_GET_PROPERTY_MAP 					GET_PROPERTY;			//struct si446x_reply_GET_PROPERTY_map                             GET_PROPERTY;
        SI446X_REPLY_GPIO_PIN_CFG_MAP					GPIO_PIN_CFG;			//struct si446x_reply_GPIO_PIN_CFG_map                             GPIO_PIN_CFG;
        SI446X_REPLY_FIFO_INFO_MAP 						FIFO_INFO;				//struct si446x_reply_FIFO_INFO_map                                FIFO_INFO;
        SI446X_REPLY_GET_INT_STATUS_MAP 				GET_INT_STATUS;			//struct si446x_reply_GET_INT_STATUS_map                           GET_INT_STATUS;
        SI446X_REPLY_REQUEST_DEVICE_STATE_MAP 			REQUEST_DEVICE_STATE;	//struct si446x_reply_REQUEST_DEVICE_STATE_map                     REQUEST_DEVICE_STATE;
        SI446X_REPLY_READ_CMD_BUFF_MAP 					READ_CMD_BUFF;			//struct si446x_reply_READ_CMD_BUFF_map                            READ_CMD_BUFF;
        SI446X_REPLY_FRR_A_READ_MAP						FRR_A_READ;				//struct si446x_reply_FRR_A_READ_map                               FRR_A_READ;
        SI446X_REPLY_FRR_B_READ_MAP 					FRR_B_READ;				//struct si446x_reply_FRR_B_READ_map                               FRR_B_READ;
        SI446X_REPLY_FRR_C_READ_MAP						FRR_C_READ;				//struct si446x_reply_FRR_C_READ_map                               FRR_C_READ;
        SI446X_REPLY_FRR_D_READ_MAP						FRR_D_READ;				//struct si446x_reply_FRR_D_READ_map                               FRR_D_READ;
        SI446X_REPLY_IRCAL_MANUAL_MAP					IRCAL_MANUAL;			//struct si446x_reply_IRCAL_MANUAL_map                             IRCAL_MANUAL;
        SI446X_REPLY_PACKET_INFO_MAP					PACKET_INFO;			//struct si446x_reply_PACKET_INFO_map                              PACKET_INFO;
        SI446X_REPLY_GET_MODEM_STATUS_MAP				GET_MODEM_STATUS;		//struct si446x_reply_GET_MODEM_STATUS_map                         GET_MODEM_STATUS;
        SI446X_REPLY_READ_RX_FIFO_MAP					READ_RX_FIFO;			//struct si446x_reply_READ_RX_FIFO_map                             READ_RX_FIFO;
        SI446X_REPLY_GET_ADC_READING_MAP				GET_ADC_READING;		//struct si446x_reply_GET_ADC_READING_map                          GET_ADC_READING;
        SI446X_REPLY_GET_PH_STATUS_MAP					GET_PH_STATUS;			//struct si446x_reply_GET_PH_STATUS_map                            GET_PH_STATUS;
        SI446X_REPLY_GET_CHIP_STATUS_MAP				GET_CHIP_STATUS;		//struct si446x_reply_GET_CHIP_STATUS_map                          GET_CHIP_STATUS;
}SI446X_CMD_REPLY_UNION;
#endif



#endif

