#ifndef _SI4432_H
#define _SI4432_H



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



//save SI4432 Reg status
typedef struct
{
	u_char ubINTFlag;
	u_char ubpksent;
	u_char ubtxffaem;
	u_char ubInt1Flag;
	u_char ubInt2Flag;
	u_char ubSta1Flag;
	u_char ubSta2Falg;
	u_char ubCurrentInt1Flag;
	u_char ubCurrentInt2Flag;
}SI4432REGSTAT;

//save SI4432 interrupt status
typedef struct
{
	u_char ubINT1Status;
	u_char ubINT2Status;
}SI4432INTSTATUS;




//save si4432 receive paket data in si4432 isr
typedef struct
{
	//u_char ubCRCError;
	//u_char ubRxFFAFull;
	u_char ubIPKVaild;
	u_char ubDataLen;
	u_char ubRevDataCnt;
	//u_char ubPaketFinish;   //0 no finish	1 finish
	u_char ubPaketLenFlag;	//0 < 64, >1 data length > 64 
	u_char ubRxPaketData[128];
}SI4432RECVPAKETDATA;

/**************************************************************************/
/*!
        Driver Control Block
        This structure contains flags and information that is used to control
        the behavior of the driver and allow it to respond to rx and tx events.
*/
/**************************************************************************/
typedef struct
{
    U8 handle;                  ///< Frame handle used to identify the frame and assoc it with a status.
    U8 status;                  ///< Transmit status after a transmission attempt.
    bool status_avail;  ///< Status available flag. Used to signal the driver that a new status is avail.
    bool data_rx;       ///< Data rx flag. Used to signal the driver that data has been received.
} SI4432_DCB;


//================================================================================================
//
// MAC return parameter enumeration
//
//================================================================================================
typedef enum
{
	MAC_OK = 0,
	NAME_ERROR,
	VALUE_ERROR,
	STATE_ERROR,
	WAKEUP_ERROR,
	LBT_ERROR,
	PACKET_SENT,
	ACK_RX_ERROR,
	INCONSISTENT_SETTING,
	CHIPTYPE_ERROR
}MacParams;



typedef enum
{
	TX_PWR__1DBM = 0x00,
	TX_PWR_2DBM	= 0x01,
	TX_PWR_5DBM	= 0x02,
	TX_PWR_8DBM	= 0x03,
	TX_PWR_11DBM = 0x04,
	TX_PWR_14DBM = 0x05,
	TX_PWR_17DBM = 0x06,
	TX_PWR_20DBM = 0x07
}SI4432POWERDBM;


/*********************************************************/
//PROCESS_NAME(drvr_process);             ///< Main SI4432 driver process
/*********************************************************/

#define  SI4432TXFAFTHD			54  //TX FIFO Almost Full Threshold
#define  SI4432TXFAFWRTHD		48  //TX FIFO Almost Full Threshold

#define  SI4432TXFAETHD			10  //TX FIFO Almost Empty Threshold
#define	 SI4432RXFAFTHD			54	//RX FIFO Almost Full Threshold
#define	 SI4432RXFAFREAD		48	//when RX FIFO Almost Full Threshold interrupt,read numbers bytes of the rx fifo



//when paket send FIFO Limit size
#define		SI4432FIFOSIZE			(64)
#define		SI4432FIFOEMPTYTHD		(16)
#define		SI4432FIFOTXFULLTHD		(54)
#define		SI4432FIFORXFULLTHD		(48)
#define		SI4432FREQFD				(0x48)

//This define is used for write reg
#define		REG_BIT7			(1<<7)
#define		REG_BIT6			(1<<6)
#define		REG_BIT5			(1<<5)
#define		REG_BIT4			(1<<4)
#define		REG_BIT3			(1<<3)
#define		REG_BIT2			(1<<2)
#define		REG_BIT1			(1<<1)
#define		REG_BIT0			(1<<0)


//chip id version 0x00 0x01 reg
#define		SI4432_CHIP_TYPE_CODE				(0x08)
#define		SI4432_CHIP_VERSION_CODE			(0x06)

//chip device status 0x02
#define		SI4432_STA_RXTX_FFOVFL		(1<<7)
#define		SI4432_STA_RXTX_FFUNFL		(1<<6)
#define		SI4432_STA_RX_FFEMPTY		(1<<5)
#define		SI4432_STA_HEADERR			(1<<4)
#define		SI4432_STA_FREQERR			(1<<3)
//si4432 work state 0x02
#define		SI4432_STA_IDLE			(0x00)
#define		SI4432_STA_RX				(0x01)
#define		SI4432_STA_TX				(0x02)

//
PROCESS_NAME(rf_tx_test);

void SI4432SNDTurnON(void);
void SI4432SNDTurnOff(void);
void SI4432ReadID(void);


void testSi4432RecvInit(void);
void testsi4432revc(void);
void testSI4432SendInit(void);
void testsi4432sendpaketdata(void);



void SI4432_NIRQ_Config(void);
void SI4432Init(void);
u_char SI4432_SetChannel(u_char ubChannel);

#endif






