
#include "contiki-conf.h"
#include "sys/etimer.h"
#include "sys/clock.h"
#include "net/netstack.h"



#include "stm32f10x.h"
#include "iodef.h"
#include "basictype.h"
#include "atom.h"

#include "sysprintf.h"

#include "arch_spi.h"

#include "radio/si4432.h"
#include "radio/si4432_v2.h"
#include "radio/si4432_rf.h"
#include "radio/si4432_config_const.h"

#include "extgdb_376_2.h"


#include <string.h>

#define		WRITE_SI4432REG		0x80
#define		WRITE_DUMMYDATA		0xff
#define 	SI4432TPYPECODE		0x08
#define		SI4432VERSIONCODE	0x06

//#define		UNUSED_NIRQ_EXIT	//for test ,when use NIRQ interrupt ,this is not used


#define  SI4432_PWRSTATE_READY		          01		// 模块 Ready 状态定义
#define  SI4432_PWRSTATE_TX				      0x09		// 模块 发射状态定义
#define  SI4432_PWRSTATE_RX				      05		// 模块 接收状态定义
#define  SI4432_PACKET_SENT_INTERRUPT	      04		// 模块 发射完成中断
#define  SI4432_Rx_packet_received_interrupt  0x02      // 模块 收到数据包中断

#define  TX1_RX0	SpiWriteRegister(0x0e|0x80, 0x02)		// 发射状态的天线开关定义
#define  TX0_RX1	SpiWriteRegister(0x0e|0x80, 0x04)		// 接收状态的天线开关定义
#define  TX0_RX0	SpiWriteRegister(0x0e|0x80, 0x00)         // 非发射，接收状态的天线开关定义
#define		TX_TIME_OUT				360000

#define  SI4432OPENTXSWITCH		SpiWriteRegister(0x0e|0x80, 0x04)// VCONT1 = 1  VCONT2 =0		
#define  SI4432OPENRXSWITCH		SpiWriteRegister(0x0e|0x80, 0x02)// VCONT1 = 0  VCONT2 =1			
#define  SI4432CLOSESWITCH		SpiWriteRegister(0x0e|0x80, 0x00)         // 非发射，接收状态的天线开关定义


static SI4432REGSTAT stRFRegSta = {0, 0, 0};
//static SI4432_DCB dcb;      ///< Driver control block instantiation
static SI4432RECVPAKETDATA stRecvPaket = {0,0,0,0,0,0,0};


volatile uint8_t si4432_ubpksent = 0;
volatile uint8_t si4432_ubtxffaem = 0;
//static u_long timeout = 0;

u_char gubSI4432Int = 0; //for debug test
static volatile uint8_t  ubTxWaitFlag = 0;

#define SI4432_B1

extern RF_NODE_PARAM_CONFIG * get_node_rf_param(void);

void SI4432RXPrepare(void);

//write a Byte data to the si4432 reg
void SpiWriteRegister(u8 ubReg, u8 ubValue)
{
	SI4432NSEL(LOW);
	SPI_SendByteData(ubReg|WRITE_SI4432REG);
	SPI_SendByteData(ubValue);
	SI4432NSEL(HIGH);
}

//read specify the reg 
u_char SpiReadRegister(u8 ubReg)
{
	u_char ubRead = 0;
	
	SI4432NSEL(LOW);
	SPI_SendByteData(ubReg);
	//SPI_SendByteData(WRITE_DUMMYDATA);
	ubRead=SPI_SendByteData(0xFF);
	SI4432NSEL(HIGH);

	return ubRead;
}

 //write si4432 reg value.
void SI4432WriteReg(u8 ubReg, u8 ubValue)
{
	SpiWriteRegister(ubReg, ubValue);
}

//read si4432 reg
u_char SI4432ReadReg(u8 ubReg)
{
	return (SpiReadRegister(ubReg));
}


int  SI4432WaitInt(void)
{

  	rtimer_clock_t timeout_time = 0;

	//timeout_time = RTIMER_NOW() + (SI4432_WAIT_INT_TIME);
#if 0
	while(!gubSI4432Int)
	{
	}
	gubSI4432Int = 0;
#endif	
	while(!stRFRegSta.ubINTFlag)
	{

		timeout_time++;
		//if ( RTIMER_CLOCK_LT(timeout_time, RTIMER_NOW()))//time out
		if ( timeout_time > 480000)//time out
		{
			XPRINTF((10,"send error\n"));
			return 1;
		}
	}
	stRFRegSta.ubINTFlag = 0;	
	return 0;
}

//wait SI4432_IPKVALID interrrupt, receive a valid paket .
void SI4432WaitIPKVALID(void)
{
//	while((stRFRegSta.ubINT1STATemp&SI4432_IPKVALID) != SI4432_IPKVALID)
	{
	
	}
	//clear SI4432_IPKVALID Flag.
//	stRFRegSta.ubINT1STATemp = stRFRegSta.ubINT1STATemp&(~SI4432_IPKVALID);
}

//wait SI4432_pksent interrupt,finish a paket sent.
//for send data
int  SI4432Wait_pksent(void)
{
	#if 0
	u_long timeout = 0;
	si4432_ubpksent = 0;
	while((!si4432_ubpksent))
	{
		timeout++;
		if ( timeout > SEND_PACKET_TIMEOUT)//time out
		{
			XPRINTF((10,"send error 1\n"));
			return 1;
		}
	}
	si4432_ubpksent = 0;
	return 0;
	#else
	rtimer_clock_t wt;
	/* Check for ack */
	wt = RTIMER_NOW();
	si4432_ubpksent = 0;
	while((!si4432_ubpksent)&&RTIMER_CLOCK_LT(RTIMER_NOW(), wt + SEND_PACKET_TIMEOUT));
	if (si4432_ubpksent)
	{
		//tx successful
		si4432_ubpksent = 0;
		return 0;
	}
	//tx failed 
	return 1;
	#endif
}

//wait SI4432_entxffaem interrupt,finish a paket sent.
//for send data
int  SI4432Wait_txffaem(void)
{
	#if 0
	u_long timeout = 0;
	si4432_ubtxffaem = 0;
	while((!si4432_ubtxffaem))
	{
		timeout++;
		if ( timeout > SEND_WAIT_TXFIFO_EMPTY )//time out
		{
			return 1;
		}
	}
	si4432_ubtxffaem = 0;
	return 0;
	#else
	rtimer_clock_t wt;
	/* Check for ack */
	wt = RTIMER_NOW();
	si4432_ubtxffaem = 0;
	while((!si4432_ubtxffaem)&&RTIMER_CLOCK_LT(RTIMER_NOW(), wt + SEND_WAIT_TXFIFO_EMPTY));
	if (si4432_ubtxffaem)
	{
		//wait empty successful
		si4432_ubtxffaem = 0;
		return 0;
	}
	//wait empty failed 
	return 1;
	
	#endif
}



// wait SI4432_IPOR interrupt,si4432 POR power reset.
void SI4432WaitIPOR(void)
{
//	while((stRFRegSta.ubINT1STATemp&SI4432_IPOR) != SI4432_IPOR)
	{
	
	}
	//clear SI4432_IPKSENT Flag.
//	stRFRegSta.ubINT1STATemp = stRFRegSta.ubINT1STATemp&(~SI4432_IPOR);
}


// wait SI4432_ICHIPRDY interrupt,si4432 POR power reset.
void SI4432WaitICHIPRDY(void)
{
//	while((stRFRegSta.ubINT1STATemp&SI4432_ICHIPRDY) != SI4432_ICHIPRDY)
	{
	
	}
	//clear SI4432_IPKSENT Flag.
//	stRFRegSta.ubINT1STATemp = stRFRegSta.ubINT1STATemp&(~SI4432_ICHIPRDY);
}








u_char SI4432ReadDevSta(void)
{
	u_char ubDevSta;
	ubDevSta = SI4432ReadReg(SI4432_DEVICE_STATUS);
	return ubDevSta;
}


//write muti datas to si4432,use to write data to FIFO
void SI4432WriteMutiData(const u8 *pcBuf, u_char ubDataLen)
{
	u_char i = 0;
	if (NULL == pcBuf)
	{
		return;
	}

	SI4432NSEL(LOW);
	SPI_SendByteData(SI4432_FIFO_ACCESS| WRITE_SI4432REG);

	for (i = 0; i < ubDataLen; i++)
	{
		SPI_SendByteData(pcBuf[i]);
		//SI4432WriteReg(SI4432_FIFO_ACCESS, pcBuf[i]);
	}
	SI4432NSEL(HIGH);
}

#if 0
//read muti datas from si4432,use to read data from rx FIFO
void SI4432ReadMutiData(u8 *pcBuf, u_char ubDataLen)
{
	u_char i = 0;
	if (NULL == pcBuf)
	{
		return;
	}

	SI4432NSEL(LOW);
	for (i = 0; i < ubDataLen; i++)
	{
		pcBuf[i] = SI4432ReadReg(SI4432_FIFO_ACCESS);
	}
	SI4432NSEL(HIGH);
}
#else

//read muti datas from si4432,use to read data from rx FIFO
void SI4432ReadMutiData(u8 *pcBuf, u_char ubDataLen)
{
	u_char i = 0;
	if (NULL == pcBuf)
	{
		return;
	}

	SI4432NSEL(LOW);
	SPI_SendByteData(SI4432_FIFO_ACCESS);
	for (i = 0; i < ubDataLen; i++)
	{
		pcBuf[i] = SPI_SendByteData(SI4432_FIFO_ACCESS);
	}
	SI4432NSEL(HIGH);
}
#endif

//read si4432 device type(reg 0x00),device version(0x01)
//device status(0x02)
//This funtion is used for test SPI interface
void SI4432ReadID(void)
{
	u_char static ubStatus = 0;

	ubStatus = SI4432ReadReg(SI4432_DEVICE_TYPE);
	XPRINTF((0,"si4432 ID----------------------------------\r\n"));
	#if 1
	if(ubStatus == SI4432TPYPECODE)
	{
		XPRINTF((0,"si4432 device type 0x00 is %02x\r\n", ubStatus));
	}
	else
	{
		XPRINTF((0,"Read device type 0x00 error\r\n"));
	}
	#endif
	#if 1
	ubStatus = SI4432ReadReg(SI4432_DEVICE_VERSION);
	if((ubStatus&SI4432VERSIONCODE )== SI4432VERSIONCODE)
	{
		XPRINTF((0,"si4432 version code  0x01 is %02x\r\n", ubStatus));
	}
	else
	{
		XPRINTF((0,"Read version code 0x01 error\r\n"));
	}


	ubStatus = SI4432ReadReg(SI4432_DEVICE_STATUS);
	XPRINTF((0,"si4432 device status 0x02 is %02x\r\n", ubStatus));
	#endif

	ubStatus = SI4432ReadReg(SI4432_INTERRUPT_ENABLE_1);
	XPRINTF((0,"INTERRUPT_ENABLE_1 REG 0x05 is %x\r\n" , ubStatus));

	ubStatus = SI4432ReadReg(SI4432_INTERRUPT_ENABLE_2);
	XPRINTF((0,"INTERRUPT_ENABLE_2 REG 0x06 is %x\r\n" , ubStatus));	
}



//soft reset si4432
//note reg 0x03 0x04 status is readed in the interrupt.
//This funtion is only send cmd to si4432
//软件复位
void SI4432SoftReset(SI4432REGSTAT *pStr)
{
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x80); 	//向0X07地址  写入0X80  软件复位
	//while(SI4432NIRQSTA == 1);		//wait for interrupt;
	SI4432WaitInt( );

	#ifdef UNUSED_NIRQ_EXIT
	pStr->ubINT1STA = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1); //clear interrupt flag,release si4432 nIRQ pin
	pStr->ubINT2STA = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2); //clear interrupt flag,release si4432 nIRQ pin
	#endif
}


//Turn on the radio by set PWRDN pin to zero,wait for some made the chip stable.
//note   
void SI4432SNDTurnON(void)
{
	#ifdef UNUSED_NIRQ_EXIT
	u_char ubStatus1 = 0;
	u_char ubStatus2 = 0;
	#endif

	unsigned int delay = 0;
	SI4432SDN(HIGH);

#if 0
	
	{
		int i = 0;
		for(delay = 0; delay < 10000; delay++)
		{
			for(i = 0; i < 1000; i++);
		}
	}
#endif
	XPRINTF((10, "SND1\r\n"));
	clock_wait(2);
	SI4432SDN(LOW);
	//clock_wait(10);
	XPRINTF((10, "SND2\r\n"));
#if 0
	{
		int i = 0;
		for(delay = 0; delay < 10000; delay++)
		{
			for(i = 0; i < 1000; i++);
		}
	}
#endif
	#ifdef UNUSED_NIRQ_EXIT
	ubStatus1 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
	ubStatus2 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
	#endif
}



//Close  the radio by set PWRDN pin high.
//note   
void SI4432SNDTurnOff(void)
{
	SI4432SDN(HIGH);
	clock_wait(30);//delay 30ms
}



//get si4432 Regstate struct.
SI4432REGSTAT * SI4432GetRFRegSta(void)
{
	return &stRFRegSta;
}

u_char SI4432GetDevSta(void)
{
	u_char ubDevSta = 0;

	ubDevSta = SI4432ReadReg(SI4432_DEVICE_STATUS);

	return ubDevSta;
}

u_char SI4432GetEZMACSta(void)
{
	u_char ubEZMACSta = 0;
	ubEZMACSta = SpiReadRegister(SI4432_EZMAC_STATUS);
	return ubEZMACSta;
}



#if 0
//when receive data crc error, reset SI4432 RX FIFO
void SI4432ResetRXFIFO(SI4432REGSTAT *pstRFRegSta)
{
	u_char ubOldReg = 0;
	
	if (NULL == pstRFRegSta)
	{
		return;
	}

	if ((pstRFRegSta->ubINT1STA & FLAG_ICRCERROR) == FLAG_ICRCERROR)
	{
		#if 0
		//reset the RX FIFO
		SpiWriteRegister(0x08, 0x02);
		//write 0x02 to the Operating Function Control 2 register
		SpiWriteRegister(0x08, 0x00);//write 0x00 to the Operating Function Control 2 register	
		#endif
		//Get the old value of the reg
		ubOldReg = SI4432ReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
		//reset the RX FIFO
		SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubOldReg|REG_BIT1);
		//write 0x02 to the Operating Function Control 2 register
		SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubOldReg&(~REG_BIT1));//write 0x00 to the Operating Function Control 2 register	
	}
}
#else


//when receive data crc error, reset SI4432 RX FIFO
void SI4432ResetRXFIFO(void)
{

	#if 0
	//reset the RX FIFO
	SpiWriteRegister(0x08, 0x02);
	//write 0x02 to the Operating Function Control 2 register
	SpiWriteRegister(0x08, 0x00);//write 0x00 to the Operating Function Control 2 register	
	#endif
	u_char ubSta;
	//reset the RX FIFO
	//write 0x02 to the Operating Function Control 2 register
	ubSta = SI4432ReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
	ubSta = ubSta | 0x02;
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);
	ubSta = ubSta &(~0x02);
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x02);
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x00);//write 0x00 to the Operating Function Control 2 register	
	
}

#endif



#if 0
// reset SI4432 TX FIFO
void SI4432ResetTXFIFO(SI4432REGSTAT *pstRFRegSta)
{
	u_char ubOldReg = 0;
	
	if (NULL == pstRFRegSta)
	{
		return;
	}

	if ((pstRFRegSta->ubINT1STA & FLAG_ICRCERROR) == FLAG_ICRCERROR)
	{
		//Get the old value of the reg
		ubOldReg = SI4432ReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
		//reset the RX FIFO
		SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubOldReg|REG_BIT0);
		//write 0x02 to the Operating Function Control 2 register
		SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubOldReg&(~REG_BIT0));//write 0x00 to the Operating Function Control 2 register	
	}
}
#else
// reset SI4432 TX FIFO
void SI4432ResetTXFIFO(void)
{
	//reset the TX FIFO
	//write 0x02 to the Operating Function Control 2 register

	u_char ubSta;
	//ubSta = SI4432ReadReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2);
	//ubSta = ubSta | 0x01;
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);
	//ubSta = ubSta &(~0x01);
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, ubSta);

	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x01);
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2, 0x00);//write 0x00 to the Operating Function Control 2 register	
}
#endif


//Turn receiver OFF
void SI4432StopReceive(void)
{
	// Turn Receiver OFF
	//SpiWriteRegister(0x07, 0x01);
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x01);
}

void SI4432StartReceive(void)
{
	// Turn Receiver ON
	//SpiWriteRegister(0x07, 0x04);
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x04);
	
}


//param ubFreq this need to commulate with data rate ,carrer freq and so on
//note FD need to be set with the other si4432 rf param 
void SI4432SetFd(void)
{
	//The Tx deviation register has to set according to the deviation before every transmission (+-30kHz)
	SI4432WriteReg(SI4432_FREQUENCY_DEVIATION, 0x48);//write  to the Frequency Deviation register
}


//set the TX paket data length
void SI4432SetTxPKLen(u_char ubDataLen)
{
	//set the length of the payload to 8bytes
	//SpiWriteRegister(0x3E, 128);//write  to the Transmit Packet Length register
	SI4432WriteReg(SI4432_TRANSMIT_PACKET_LENGTH, ubDataLen);//write  to the Transmit Packet Length register
}

//this funtion used to send data when paket data length > 64 and clear interrupt flag
//I_ENRXFFAFULL
void SI4432OnlyI_ENRXFFAFULL(SI4432REGSTAT*pstRFRegSta)
{
	//Disable all other interrupts and enable the FIFO almost empty interrupt only.
	//SpiWriteRegister(0x05, 0x20); //write 0x20 to the Interrupt Enable 1 register
	//SpiWriteRegister(0x06, 0x00); //write 0x00 to the Interrupt Enable 2 register
	SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1, 0x20); //write 0x20 to the Interrupt Enable 1 register
	SI4432WriteReg(SI4432_INTERRUPT_ENABLE_2, 0x00); //write 0x00 to the Interrupt Enable 2 register
//	pstRFRegSta->ubINT1STA = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
//	pstRFRegSta->ubINT2STA = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
}


/*enable transmitter*/
//The radio forms the packet and send it automatically.
void SI4432ENTXAuto(void)
{
	//SpiWriteRegister(0x07, 0x09); //write 0x09 to the Operating Function Control 1 register
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x09); //write 0x09 to the Operating Function Control 1 register
}



/*
Set Tx power
*/
void SI4432SetTxPower(u_char dbm)
{
	u_char ubRegValue = 0;

	ubRegValue = SI4432ReadReg(SI4432_TX_POWER);
	ubRegValue = ((~(0x07))&ubRegValue) | (dbm & 0x07);
	SI4432WriteReg(SI4432_TX_POWER, ubRegValue);
}

/*
Get Rx signal strenth
Input receive power			RSSI value
-120dbm  					20
-100dbm						45
0dbm						230
120/(230-20) = 0.57/dbm   rssi value to dbm is  (value - 230 ) <<1
when receive power is larger 0dbm, RSSI value is 230,not change

specify the value is 0.5/dbm for  commulate
note this value more biger and the RSSI value more smller.
*/
/*
接收到信号强度指示（RSSI）提供一个在调谐信道上接收到的信号强度的测量。
读出RSSI的结果是0.5dB的增量,类似于一个高分辨率使能精确信道评估需要CCA
（清理信道评估）、CS（载波感应）和LBT（载波侦听）功能。
RSSI (接收信号强度指示)信号接收器打开了在信道的一个信号强度评估,
当使能同步字侦测在同步字已经侦测到后将会冻结RSSI的值，
当禁止同步字侦测或同步字没有侦测到，RSSI的值将连续不断的更新。
从一个8位的寄存器能读出每位精度是0.5dB的RSSI的值，总的127.5dB的RSSI的范围。
信道空闲评估门槛在寄存器可编程27h的rssith[7:0]，
在引导码RSSI评估后，如果在这个信道上无信号强度产生一个判断是在门槛以上或者以下。
如果信号强度在可编程的门槛以上那么一个“1”将出现在器件状态寄存器02h 的RSSI状态位上。
中断状态寄存器04h ，或配置GPIO 
*/
u_char SI4432GetRSSI(void)
{
	u_long udwRSSI = 0;

	udwRSSI = SI4432ReadReg(SI4432_RECEIVED_SIGNAL_STRENGTH_INDICATOR);

	return (u_char)((udwRSSI<<6)>>7);
}

//set  rssi threshold value
//note when set 80dbm,fact is -80dbm
void SI4432SetRSSIThreshold(u_char ubDbm)
{
	u_long ubRSSITH = 0;

	ubRSSITH = 230 - (ubDbm <<7 )>>6;

	SI4432WriteReg(SI4432_RSSI_THRESHOLD, (u_char)ubRSSITH);
}


u_char SI4432SetChannel(u_char ubChannel)
{
	//SpiWriteRegister(0x79, ubChannel);  // 需要跳频  2014 12 09
	SpiWriteRegister(SI4432_FREQUENCY_HOPPING_CHANNEL_SELECT, ubChannel);  // 需要跳频  2014 12 09	
    return 0;
}

// tx fifo always full 
void SI4432SetTXAFTHR(u_char ubTxafthr)
{
	if (ubTxafthr > SI4432FIFOSIZE)
	{
		return ;
	}

	SI4432WriteReg(SI4432_TX_FIFO_CONTROL_1, ubTxafthr);
}

//tx always empty
void SI4432SetTXAETHR(u_char ubTxaethr)
{
	if (ubTxaethr < 1)
	{
		return;
	}
	SI4432WriteReg(SI4432_TX_FIFO_CONTROL_2, ubTxaethr);
}

//rx always full
void SI4432SetRXAFTHR(u_char ubRXafthr)
{
	if (ubRXafthr > SI4432FIFOSIZE)
	{
		return;
	}
	SI4432WriteReg(SI4432_RX_FIFO_CONTROL, ubRXafthr);
}


//get SI4432RECVPAKETDATA STRUCT
SI4432RECVPAKETDATA *SI4432GetRXRecvPak(void)
{
	return &stRecvPaket;
}


//Set channel
u_char SI4432_SetChannel(u_char ubChannel)
{
	return 0;
}


//clear the SI4432RECVPAKETDATA STRUCT
//void *memset(void *s, int ch, size_t n);
void SI4432ClearRxRevcPak(void)
{
	SI4432RECVPAKETDATA *pRxPak = NULL;

	pRxPak = (SI4432RECVPAKETDATA *)SI4432GetRXRecvPak( );

	memset(pRxPak, 0, sizeof(SI4432RECVPAKETDATA));	
}


//when send a frame data, before send data, prepare
void SI4432SendDataPrepare(void)
{
	//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
	//SI4432StopReceive( );
	//set freq fd
	SI4432SetFd( );

	//SI4432ResetTXFIFO( );
	//SpiWriteRegister(0x07, 0x01);	//READY
	SpiWriteRegister(0x07, 0x03);	//READY

	//SI4432WriteReg(0x0e|0x80, 0x02); // 发射状态的天线开关定义

	//SI4432OPENTXSWITCH;
}



/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
void SI4432FrameSend(const u_char *pubdata, u_char ubDataLen)
{
	//when data length > 64, 
	u_char ubDataNLen = 0;
	const u_char *pbuf = pubdata;

	ubDataNLen = ubDataLen;

	//set paket len
	SI4432SetTxPKLen( ubDataLen );

	//Fill data to FIFO first,frame length < 64
	if(ubDataLen < (SI4432FIFOSIZE + 1))
	{
		SI4432WriteMutiData(pbuf, ubDataLen);
		//SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1|0x80, SI4432_PACKET_SENT_INTERRUPT);	// 整包数据发射完后，产生中断
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1|0x80, SI4432_ENPKSENT);	// 整包数据发射完后，产生中断
		SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1|0x80, SI4432_PWRSTATE_TX);  // 进入发射模式
		//	delay_1ms(5);	
		//while(SI4432NIRQSTA);		// 等待中断	
		SI4432WaitInt( );
	}
	else
	{
		SI4432WriteMutiData( pbuf, SI4432FIFOSIZE);

		//Disable all other interrupts and enable the FIFO almost empty interrupt only.
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1, 0x20);		//write 0x20 to the Interrupt Enable 1 register	
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_2, 0x00);		//write 0x00 to the Interrupt Enable 2 register	
		SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
		SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
		/*enable transmitter*/
		//The radio forms the packet and send it automatically.
		SpiWriteRegister(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x09);	//write 0x09 to the Operating Function Control 1 register
		//while(SI4432NIRQSTA);
		SI4432WaitInt( );
		ubDataNLen = ubDataNLen - SI4432FIFOSIZE;
		pbuf = pbuf+SI4432FIFOSIZE;
		while(ubDataNLen > SI4432TXFAFWRTHD)
		{
			SI4432WriteMutiData(pbuf, SI4432TXFAFWRTHD);	
			ubDataNLen = ubDataNLen - SI4432TXFAFWRTHD;
			pbuf = pbuf+SI4432TXFAFWRTHD;
			//while(SI4432NIRQSTA);
			SI4432WaitInt( );
		}
		SI4432WriteMutiData(pbuf, ubDataNLen);
		//Disable all other interrupts and enable the packet sent interrupt only.
		//This will be used for indicating the successfull packet transmission for the MCU
		SpiWriteRegister(0x05, 0x04);	//write 0x04 to the Interrupt Enable 1 register	
		//while(SI4432NIRQSTA);//End send a frame.
		SI4432WaitInt( );
	}
	//LED2(1);//TEST
}



void SI4432DisableReceiveChain(void)
{
	//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
	SpiWriteRegister(0x07, 0x01);													//write 0x01 to the Operating Function Control 1 register			
	return;
}


void SI4432EnableReceiveChain(void)
{
	//disable the receiver chain (but keep the XTAL running to have shorter TX on time!)
	SpiWriteRegister(0x07, 0x05);													//write 0x01 to the Operating Function Control 1 register			
	return;
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
int SI4432RadioTransmit(const u_char *pubdata)
{
	//when data length > 64, 
//	extern volatile enum SI4432_STATE si4432_state;	
	
	u_char ubDataNLen = 0;
//	u_char ubDataLen = 0;
	u_char ItStatus1 = 0;
	u_char ItStatus2 = 0;
	const u_char *pbuf = pubdata;
	int sdwSendTimeFlag = 0;
	int nresult = 0;
	u_char ubDevT = 0;

	ubDataNLen = pubdata[0] + 1;
//	ubDataLen = pubdata[0] + 1;

//	si4432_state = 0x02;	
	//clear tx interrupt value

	ubDevT = SI4432ReadReg(SI4432_DEVICE_TYPE);
	{
		//XPRINTF((10, "DEV T is %02x\r\n", ubDevT));
	}
	
	SI4432DisableReceiveChain( );
	SI4432ResetTXFIFO( );
	//set paket len
	SI4432SetTxPKLen( ubDataNLen );

#ifndef SI4432_B1	
	//SI4432WriteReg(SI4432_FREQUENCY_DEVIATION, 0x48);//write  to the Frequency Deviation register
#endif
	//Fill data to FIFO first,frame length < 64
	if(ubDataNLen < SI4432FIFOSIZE )
	{
		SI4432WriteMutiData(pbuf, ubDataNLen);

		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1|0x80, SI4432_ENPKSENT);	// 整包数据发射完后，产生中断
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_2, 0x00);
		ItStatus1 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
		ItStatus2 = SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
		
		//PRINTF("ItStatus1= %02x   ItStatus2 =%02x \r\n", ItStatus1, ItStatus2);
		SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1|0x80, SI4432_PWRSTATE_TX);  // 进入发射模式
		sdwSendTimeFlag=SI4432Wait_pksent( );
		//sdwSendTimeFlag = SI4432WaitInt( );
		if (sdwSendTimeFlag)//send time out
		{
			nresult = -1;
			SI4432Init( );
			XPRINTF((10, "---------------1s\r\n"));
			goto rxprepare;
		}
		//PRINTF("Sent Finish\r\n");
	}
	else
	{
		SI4432WriteMutiData( pbuf, SI4432FIFOSIZE);
		//Disable all other interrupts and enable the FIFO almost empty interrupt only.
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1, SI4432_ENTXFFAEM);		//write 0x20 to the Interrupt Enable 1 register	
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_2, 0x00);		//write 0x00 to the Interrupt Enable 2 register	
		SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
		SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);

		/*enable transmitter*/
		SpiWriteRegister(SI4432_OPERATING_AND_FUNCTION_CONTROL_1, 0x09);	//write 0x09 to the Operating Function Control 1 register
		sdwSendTimeFlag=SI4432Wait_txffaem( );  //wait for interrupt
		//sdwSendTimeFlag = SI4432WaitInt( );
		if (sdwSendTimeFlag)//send time out
		{
			nresult = -1;
			SI4432Init( );
			XPRINTF((10, "---------------2e\r\n"));
			goto rxprepare;
		}
		
		ubDataNLen = ubDataNLen - SI4432FIFOSIZE;
		pbuf = pbuf+SI4432FIFOSIZE;

		while(ubDataNLen > SI4432TXFAFWRTHD)
		{
			SI4432WriteMutiData(pbuf, SI4432TXFAFWRTHD);
			if (ubDataNLen > SI4432TXFAFWRTHD)
			{
				ubDataNLen = ubDataNLen - SI4432TXFAFWRTHD;
			}
			pbuf = pbuf+SI4432TXFAFWRTHD;
			//clear flag
			SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
			SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);

			sdwSendTimeFlag=SI4432Wait_txffaem( );
			//sdwSendTimeFlag = SI4432WaitInt( );
			if (sdwSendTimeFlag)//send time out
			{
				nresult = -1;
				SI4432Init( );
				XPRINTF((10, "---------------3e\r\n"));
				goto rxprepare;
			}
		}
		SI4432WriteMutiData(pbuf, ubDataNLen);
		//Disable all other interrupts and enable the packet sent interrupt only.
		//This will be used for indicating the successfull packet transmission for the MCU
		SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1|0x80, SI4432_ENPKSENT);	// 整包数据发射完后，产生中断
		SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
		SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
		sdwSendTimeFlag=SI4432Wait_pksent( );
		//sdwSendTimeFlag = SI4432WaitInt( );
		if (sdwSendTimeFlag)//send time out
		{
			nresult = -1;
			SI4432Init( );
			XPRINTF((10, "---------------4s\r\n"));
			goto rxprepare;
		}		
	}
	rxprepare:
	/*disable tx chain again*/
	//SpiWriteRegister(0x07, 0x01);														//write 0x05 to the Operating Function Control 1 register				

	//after packet transmission set the interrupt enable bits according receiving mode
	//Enable three interrupts: 
	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	// c) third shows if the RX fifo almost full: 'irxffafull
	//SpiWriteRegister(0x05, 0x13); 													//write 0x13 to the Interrupt Enable 1 register
	//SpiWriteRegister(SI4432_INTERRUPT_ENABLE_2, SI4432_ENSWDET); 											//write 0x80 to the Interrupt Enable 2 register  rssi
	//Read interrupt status regsiters. It clear all pending interrupts and the nIRQ pin goes back to high.
	//ItStatus1 = SpiReadRegister(0x03);												//read the Interrupt Status1 register
	//ItStatus2 = SpiReadRegister(0x04);												//read the Interrupt Status2 register

	/*enable receiver chain again*/
	//SpiWriteRegister(0x07, 0x05);														//write 0x05 to the Operating Function Control 1 register				
	SI4432RXPrepare( );
	return nresult;
}

const u_char ptbuf[128] = {0xab};

void si4432_for_test_send(void)
{
	int sdwSendTimeFlag = 0;
	int i = 0;
	SI4432DisableReceiveChain( );
	SI4432ResetTXFIFO( );
	//set paket len
	//SI4432SetTxPKLen( 64 );
	//SI4432WriteMutiData(ptbuf, 64);

	SI4432WriteReg(0x71, 0x33);
	//SI4432WriteReg(SI4432_INTERRUPT_ENABLE_1|0x80, SI4432_ENPKSENT);	// 整包数据发射完后，产生中断
	SI4432WriteReg(SI4432_INTERRUPT_ENABLE_2, 0x00);
	SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
	SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);
	
	//PRINTF("ItStatus1= %02x   ItStatus2 =%02x \r\n", ItStatus1, ItStatus2);
	SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_1|0x80, 0x09);  // 进入发射模式
	//SI4432WriteReg(SI4432_OPERATING_AND_FUNCTION_CONTROL_2|0x80, 0x08);  // 进入发射模式
	//sdwSendTimeFlag=SI4432Wait_pksent( );
	while(1);
}


void SI4432RXPrepare(void)
{
	//SI4432ReadReg(SI4432_DEVICE_TYPE);
	SI4432ResetRXFIFO( );

	// a) one which shows that a valid packet received: 'ipkval'
	// b) second shows if the packet received with incorrect CRC: 'icrcerror' 
	// c) third shows if the RX fifo almost full: 'irxffafull
	//SpiWriteRegister(0x05, 0x13); 											//write 0x13 to the Interrupt Enable 1 register
	
	SpiWriteRegister(SI4432_INTERRUPT_ENABLE_1, SI4432_ENRXFFAFULL|SI4432_ENPKVALID|SI4432_ENCRCERROR);//write 0x13 to the Interrupt Enable 1 register

	//SpiWriteRegister(0x06, 0x00); 											//write 0x00 to the Interrupt Enable 2 register
	//SpiWriteRegister(SI4432_INTERRUPT_ENABLE_2, SI4432_ENSWDET|SI4432_ENPREAVAL); 											//write 0x80 to the Interrupt Enable 2 register  rssi
	SpiWriteRegister(SI4432_INTERRUPT_ENABLE_2, SI4432_ENSWDET); 											//write 0x80 to the Interrupt Enable 2 register  rssi

	SI4432ReadReg(SI4432_INTERRUPT_STATUS_1);
	SI4432ReadReg(SI4432_INTERRUPT_STATUS_2);

		
#ifndef SI4432_B1
	//SpiWriteRegister(0x72, 0x1a);		//2014 12 09 disable			//write 0x1F to the Frequency Deviation register	9.6k	

#endif
	//SI4432CLOSESWITCH;//close tx rx switch
	/*enable receiver chain again*/
	SpiWriteRegister(0x07, 0x05);
	//SpiWriteRegister(0x07, 0x07);
//	SpiWriteRegister(0x0e|0x80, 0x04);		// 接收状态的天线开关定义 open receive switch
	//SI4432OPENRXSWITCH;
}



/*----------------------------------------------------------------------------*/
/** \brief  This function will upload a frame from the radio transceiver's frame
 *          buffer.
 *
 *          If the frame currently available in the radio transceiver's frame buffer
 *          is out of the defined bounds. Then the frame length, lqi value and crc
 *          be set to zero. This is done to indicate an error.
 */
void SI4432FrameRead(u_char *pbuf)
{
//    u_char len, dummy, *rx_data=0;
}


void SI4432ReConfigReg(void)
{

	SpiWriteRegister(0x06|0x80, 0x00);  	//  关闭不需要的中断
	SpiWriteRegister(0x07|0x80, SI4432_PWRSTATE_READY); // 进入 Ready 模式
	SpiWriteRegister(0x09|0x80, 0x7f);  	//  负载电容= 12P

	SpiWriteRegister(0x0a|0x80, 0x05);		// 关闭低频输出
	//SpiWriteRegister(0x0b|0x80, 0xea); 		// GPIO 0 当做普通输出口
	//SpiWriteRegister(0x0c|0x80, 0xea); 		// GPIO 1 当做普通输出口 control 
	//SpiWriteRegister(0x0d|0x80, 0xea);  	// GPIO 2 当做普通输出口

	SpiWriteRegister(0x69, 0x60);  //AGC过载	
	
	/*set the physical parameters*/
	//set the center frequency to 470 MHz
	SpiWriteRegister(0x75, 0x57); 	//write  to the Frequency Band Select register
	SpiWriteRegister(0x76, 0x00); 	//write  to the Nominal Carrier Frequency1 register
	SpiWriteRegister(0x77, 0x00); 	//write  to the Nominal Carrier Frequency0 register

	//SpiWriteRegister(0x75, 0x60); 	//write  to the Frequency Band Select register
	//SpiWriteRegister(0x76, 0x19); 	//write  to the Nominal Carrier Frequency1 register


	//set the desired TX data rate (9.6kbps)
	SpiWriteRegister(0x6E, 0x4E); //write  to the TXDataRate 1 register
	SpiWriteRegister(0x6F, 0xA5); //write  to the TXDataRate 0 register
	SpiWriteRegister(0x70, 0x2C);//10 1100 //write  to the Modulation Mode Control 1 register

	/*set the modem parameters according to the excel calculator
	(parameters: 9.6 kbps, deviation: 45 kHz, channel filterBW:112.1 kHz*/
	SpiWriteRegister(0x1C, 0x05); //write  to the IF Filter Bandwidth register
	SpiWriteRegister(0x20, 0xA1); //write  to the Clock Recovery Oversampling Ratio register
	SpiWriteRegister(0x21, 0x20); //write  to the Clock Recovery Offset 2 register
	SpiWriteRegister(0x22, 0x4E); //write  to the Clock Recovery Offset 1 register
	SpiWriteRegister(0x23, 0xA5); //write  to the Clock Recovery Offset 0 register
	SpiWriteRegister(0x24, 0x00); //write  to the Clock Recovery Timing Loop Gain 1 register
	SpiWriteRegister(0x25, 0x13); //write  to the Clock Recovery Timing Loop Gain 0 register
	SpiWriteRegister(0x1D, 0x40); //enable AFC
	//SpiWriteRegister(0x1D, 0x00); //enable AFC  20141209

	SpiWriteRegister(0x72, 0x1F); //+-45kHz

	//SpiWriteRegister(0x72, 0x20); //+-45kHz

	SpiWriteRegister(0x2a|0x80, 0x14);//AFC limiter
	//SpiWriteRegister(0x2a|0x80, 0x00);//AFC limiter  20141209

	/*set the packet structure and the modulation type*/
	SpiWriteRegister(0x30|0x80, 0x8c);		// 使能PH+ FIFO模式，高位在前面，使能CRC校验
	SpiWriteRegister(0x32|0x80, 0xff);		// byte0, 1,2,3 作为头码
	SpiWriteRegister(0x33|0x80, 0x42);		// byte 0,1,2,3 是头码，同步字3,2 是同步字
	SpiWriteRegister(0x34|0x80, 20);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte
	//SpiWriteRegister(0x34|0x80, 24);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte //add 20141209 for more channels
	SpiWriteRegister(0x35|0x80, 0x20);  	// 需要检测4个nibble的Preamble
	SpiWriteRegister(0x36|0x80, 0x2d);  	// 同步字为 0x2dd4
	SpiWriteRegister(0x37|0x80, 0xd4);
	SpiWriteRegister(0x38|0x80, 0x00);
	SpiWriteRegister(0x39|0x80, 0x00);
	SpiWriteRegister(0x3a|0x80, 'j');   	// 发射的头码为： “"
	SpiWriteRegister(0x3b|0x80, 'y');
	SpiWriteRegister(0x3c|0x80, 'u');
	SpiWriteRegister(0x3d|0x80, 'e');
	SpiWriteRegister(0x3e|0x80, 10);    	// 总共发射10个字节的数据
	//SpiWriteRegister(0x3e|0x80, 16);    	// 总共发射16个字节的数据 //12 byte + 2 byte + 4 Byte add 20141209 for more channels
	SpiWriteRegister(0x3f|0x80, 'j');   	// 需要校验的头码为：”"
	SpiWriteRegister(0x40|0x80, 'y');
	SpiWriteRegister(0x41|0x80, 'u');
	SpiWriteRegister(0x42|0x80, 'e');
	SpiWriteRegister(0x43|0x80, 0xff);  // 头码1,2,3,4 的所有位都需要校验
	SpiWriteRegister(0x44|0x80, 0xff);  // 
	SpiWriteRegister(0x45|0x80, 0xff);  // 
	SpiWriteRegister(0x46|0x80, 0xff);  // 

	//需要跳频时，设置0x79为物理信道编号，0x7a为信道间隔
	SpiWriteRegister(0x79|0x80, 0x0);  // 不需要跳频
	SpiWriteRegister(0x7a|0x80, 0x0);  // 不需要跳频

	//SpiWriteRegister(0x79|0x80, 10);  // 需要跳频  2014 12 09
	//SpiWriteRegister(0x7a|0x80, 20);  // 需要跳频  2014 12 09

	//PRINTF("0X79 IS %d\r\n", SpiReadRegister(0x79));
	//PRINTF("0X7a IS %d\r\n", SpiReadRegister(0x7a));
	
	SpiWriteRegister(0x71|0x80, 0x23); // 发射不需要 CLK，FiFo ， GFSK模式

	//set the TX FIFO almost full threshold to 54 bytes
	SpiWriteRegister(0x7C,SI4432TXFAFTHD);	
	//set the TX FIFO almost empty threshold to 10 bytes
	SpiWriteRegister(0x7D,SI4432TXFAETHD);														//write 0x0B to the TX FIFO Control 2 register
	//set the RX FIFO almost full threshold to 54 bytes
	SpiWriteRegister(0x7E,SI4432RXFAFTHD);	

	#if 1
	SpiWriteRegister(0x0C, 0x15); //write 0x12 to the GPIO1 Configuration(set the TX state)
	SpiWriteRegister(0x0D, 0x12); //write 0x15 to the GPIO2 Configuration(set the RX state)
	#endif

	/*set the non-default Si4432 registers*/
	//set the VCO and PLL
	SpiWriteRegister(0x5A, 0x7F); //write 0x7F to the VCO Current Trimming register
	SpiWriteRegister(0x58, 0x80); //write 0x80 to the ChargepumpCurrentTrimmingOverride register
	SpiWriteRegister(0x59, 0x40); //write 0x40 to the Divider Current Trimming register
	//set the AGC
	SpiWriteRegister(0x6A, 0x0B); //write 0x0B to the AGC Override 2 register
	//set ADC reference voltage to 0.9V
	SpiWriteRegister(0x68, 0x04); //write 0x04 to the Deltasigma ADC Tuning 2 register
	SpiWriteRegister(0x1F, 0x03); //write 0x03 to the Clock Recovery Gearshift Override register
	//set Crystal Oscillator Load Capacitance register
	SpiWriteRegister(0x09, 0xD7); //write 0xD7 to the Crystal Oscillator Load Capacitance register

	//SpiWriteRegister(0x6d|0x80, 0x07);  // 设置为最大功率发射
	//SpiWriteRegister(0x6d|0x80, 0x06);  // 
	SpiWriteRegister(0x6d|0x80, 0x18|0x07);  // 

	//set si4432 receive signal level 
	SpiWriteRegister(SI4432_RSSI_THRESHOLD, 60);  //when rssi interrupt open, will generate  interrupt
}







//si4432 ISR service funtion
/* 
*/
/*
1. 使能EXTIx线的时钟和第二功能AFIO时钟
2. 配置EXTIx线的中断优先级
3. 配置EXTI 中断线I/O
4. 选定要配置为EXTI的I/O口线和I/O口的工作模式
5. EXTI 中断线工作模式配置
*/
#define SI4432NIRQPORT	GPIOC
#define	SI4432NIRQPIN	GPIO_Pin_6

void SI4432_NIRQ_Config(void)
{
	GPIO_InitTypeDef gpioStr;
	EXTI_InitTypeDef extiStr;
	NVIC_InitTypeDef nvicStr;

	//clock Enable
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

	//nvic config for  exti interrupt
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );
	nvicStr.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvicStr.NVIC_IRQChannelPreemptionPriority =  0x00;
	nvicStr.NVIC_IRQChannelSubPriority = 0x00;
	//nvicStr.NVIC_IRQChannelPreemptionPriority =  0x03;
	//nvicStr.NVIC_IRQChannelSubPriority = 0x03;
	nvicStr.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStr);

	//EXTI line gpio conifg
	gpioStr.GPIO_Pin = SI4432NIRQPIN;
	gpioStr.GPIO_Mode = GPIO_Mode_IPU;
	//gpioStr.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(SI4432NIRQPORT, &gpioStr);

	//EXTI line mode config
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource6);
	extiStr.EXTI_Line = EXTI_Line6;
	extiStr.EXTI_Mode = EXTI_Mode_Interrupt;
	extiStr.EXTI_Trigger = EXTI_Trigger_Falling;
	extiStr.EXTI_LineCmd = ENABLE;
	EXTI_Init( &extiStr);
}

void SI4432_Disable_Interrupt(void)
{
	
	NVIC_InitTypeDef nvicStr;

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );
	nvicStr.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvicStr.NVIC_IRQChannelPreemptionPriority =  0x00;
	nvicStr.NVIC_IRQChannelSubPriority = 0x00;
	//nvicStr.NVIC_IRQChannelPreemptionPriority =  0x03;
	//nvicStr.NVIC_IRQChannelSubPriority = 0x03;
	nvicStr.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&nvicStr);
}

void SI4432_Enable_Interrupt(void)
{
	NVIC_InitTypeDef nvicStr;

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_1 );
	nvicStr.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvicStr.NVIC_IRQChannelPreemptionPriority =  0x00;
	nvicStr.NVIC_IRQChannelSubPriority = 0x00;
	//nvicStr.NVIC_IRQChannelPreemptionPriority =  0x03;
	//nvicStr.NVIC_IRQChannelSubPriority = 0x03;
	nvicStr.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStr);

}



//io interrupt service funtion for si4432
/*lz在什么时候读取rssi值的，我现在在接收到数据后读取rssi的值，可靠吗？lz能否讲一下 

-------------------------------------------------------------------------------------------------------------------------------------
1、把06H设置为0X80使能同步子侦测。 
2、写个中断查询函数，如果检测到中断，则查询04H是否为0X80（侦测到同步字），如果为真，读26H即可。
*/


#if 1
//9.6kbps
//SI4432 reg value is must be set with the feq,speed,fd and so on
//these value can get from the si4432 excel
void SI4432Init(void)
{

	u_char ItStatus1 = 0;
	u_char ItStatus2 = 0;
	RF_NODE_PARAM_CONFIG *prfCfg = NULL;
	XPRINTF((0, "si4432_int time %d\r\n", clock_time( )));
	#if 1
	//PRINTF("SPI CONFIG1\r\n");
	SPI_Config( );
	//PRINTF("SPI CONFIG2\r\n");
	SI4432SNDTurnON( );
	SI4432NSEL(LOW);
	clock_wait(50);
	ItStatus1 = SpiReadRegister(0x03); //read the Interrupt Status1 register
	ItStatus1 = SpiReadRegister(0x04); //read the Interrupt Status2 register

	SI4432ReadID( );
	#endif

	/* ======================================================== *
	* Initialize the Si4432 ISM chip *
	* ======================================================== */
	//SW reset
	
	SpiWriteRegister(0x07, 0x80);	//write 0x80 to the Operating & Function Control1 register
	XPRINTF((0, "SW reset\r\n"));
	#if 1
	SI4432WaitInt( );
	SI4432WaitInt( );	
	clock_wait(50);
	#else
	while(SpiReadRegister(0x04)&&SI4432_ICHIPRDY == SI4432_ICHIPRDY);
	#endif

	#ifdef UNUSED_NIRQ_EXIT
	ItStatus1 = SpiReadRegister(0x03); //read the Interrupt Status1 register
	ItStatus1 = SpiReadRegister(0x04); //read the Interrupt Status2 register
	#endif
	XPRINTF((0,"INT_1\r\n"));
	SpiWriteRegister(0x06|0x80, 0x00);  	//  关闭不需要的中断
	SpiWriteRegister(0x07|0x80, SI4432_PWRSTATE_READY); // 进入 Ready 模式
	SpiWriteRegister(0x09|0x80, 0x7f);  	//  负载电容= 12P

	SpiWriteRegister(0x0a|0x80, 0x05);		// 关闭低频输出
	//SpiWriteRegister(0x0b|0x80, 0xea); 		// GPIO 0 当做普通输出口
	//SpiWriteRegister(0x0c|0x80, 0xea); 		// GPIO 1 当做普通输出口 control 
	//SpiWriteRegister(0x0d|0x80, 0xea);  	// GPIO 2 当做普通输出口

	SpiWriteRegister(0x69, 0x60);  //AGC过载	
	
	/*set the physical parameters*/
	//set the center frequency to 470 MHz
	SpiWriteRegister(0x75, 0x57); 	//write  to the Frequency Band Select register
	SpiWriteRegister(0x76, 0x00); 	//write  to the Nominal Carrier Frequency1 register
	SpiWriteRegister(0x77, 0x00); 	//write  to the Nominal Carrier Frequency0 register

	//set the desired TX data rate (9.6kbps)
	SpiWriteRegister(0x6E, 0x4E); //write  to the TXDataRate 1 register
	SpiWriteRegister(0x6F, 0xA5); //write  to the TXDataRate 0 register
	SpiWriteRegister(0x70, 0x2C);//10 1100 //write  to the Modulation Mode Control 1 register

	/*set the modem parameters according to the excel calculator
	(parameters: 9.6 kbps, deviation: 45 kHz, channel filterBW:112.1 kHz*/
	SpiWriteRegister(0x1C, 0x05); //write  to the IF Filter Bandwidth register
	SpiWriteRegister(0x20, 0xA1); //write  to the Clock Recovery Oversampling Ratio register
	SpiWriteRegister(0x21, 0x20); //write  to the Clock Recovery Offset 2 register
	SpiWriteRegister(0x22, 0x4E); //write  to the Clock Recovery Offset 1 register
	SpiWriteRegister(0x23, 0xA5); //write  to the Clock Recovery Offset 0 register
	SpiWriteRegister(0x24, 0x00); //write  to the Clock Recovery Timing Loop Gain 1 register
	SpiWriteRegister(0x25, 0x13); //write  to the Clock Recovery Timing Loop Gain 0 register
	SpiWriteRegister(0x1D, 0x40); //enable AFC

	SpiWriteRegister(0x72, 0x1F); //+-45kHz

	SpiWriteRegister(0x2a|0x80, 0x14);//AFC limiter

	/*set the packet structure and the modulation type*/
	SpiWriteRegister(0x30|0x80, 0x8c);		// 使能PH+ FIFO模式，高位在前面，使能CRC校验
	SpiWriteRegister(0x32|0x80, 0xff);		// byte0, 1,2,3 作为头码
	SpiWriteRegister(0x33|0x80, 0x42);		// byte 0,1,2,3 是头码，同步字3,2 是同步字
	SpiWriteRegister(0x34|0x80, 20);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte
	//SpiWriteRegister(0x34|0x80, 24);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte //add 20141209 for more channels
	//SpiWriteRegister(0x35|0x80, 0x20);  	// 需要检测4个nibble的Preamble
	SpiWriteRegister(0x35|0x80, 0x40);  	// 需要检测8个nibble的Preamble
	SpiWriteRegister(0x36|0x80, 0x2d);  	// 同步字为 0x2dd4
	SpiWriteRegister(0x37|0x80, 0xd4);
	SpiWriteRegister(0x38|0x80, 0x00);
	SpiWriteRegister(0x39|0x80, 0x00);
	SpiWriteRegister(0x3a|0x80, 'h');   	// 发射的头码为： “"
	SpiWriteRegister(0x3b|0x80, 'r');
	SpiWriteRegister(0x3c|0x80, 'u');
	SpiWriteRegister(0x3d|0x80, 'n');
	SpiWriteRegister(0x3e|0x80, 10);    	// 总共发射10个字节的数据
	//SpiWriteRegister(0x3e|0x80, 16);    	// 总共发射16个字节的数据 //12 byte + 2 byte + 4 Byte add 20141209 for more channels
	SpiWriteRegister(0x3f|0x80, 'h');   	// 需要校验的头码为：”"
	SpiWriteRegister(0x40|0x80, 'r');
	SpiWriteRegister(0x41|0x80, 'u');
	SpiWriteRegister(0x42|0x80, 'n');
	SpiWriteRegister(0x43|0x80, 0xff);  // 头码1,2,3,4 的所有位都需要校验
	SpiWriteRegister(0x44|0x80, 0xff);  // 
	SpiWriteRegister(0x45|0x80, 0xff);  // 
	SpiWriteRegister(0x46|0x80, 0xff);  // 


	//需要跳频时，设置0x79为物理信道编号，0x7a为信道间隔
	//SpiWriteRegister(0x79|0x80, 0x0);  // 不需要跳频
	//SpiWriteRegister(0x7a|0x80, 0x0);  // 不需要跳频

	SpiWriteRegister(0x79|0x80, 33);  // 需要跳频  2014 12 09  channel number 470M-470Mhz + 33*200kHz
	SpiWriteRegister(0x7a|0x80, 20);  // 需要跳频  2014 12 09  channel step  *10kHz

	
	SpiWriteRegister(0x71|0x80, 0x23); // 发射不需要 CLK，FiFo ， GFSK模式


	//set the TX FIFO almost full threshold to 54 bytes
	SpiWriteRegister(0x7C,SI4432TXFAFTHD);	
	//set the TX FIFO almost empty threshold to 10 bytes
	SpiWriteRegister(0x7D,SI4432TXFAETHD);														//write 0x0B to the TX FIFO Control 2 register
	//set the RX FIFO almost full threshold to 54 bytes
	SpiWriteRegister(0x7E,SI4432RXFAFTHD);	


	#if 1
	SpiWriteRegister(0x0C, 0x15); //write 0x12 to the GPIO1 Configuration(set the TX state)
	SpiWriteRegister(0x0D, 0x12); //write 0x15 to the GPIO2 Configuration(set the RX state)
	#endif

	/*set the non-default Si4432 registers*/
	//set the VCO and PLL
	SpiWriteRegister(0x5A, 0x7F); //write 0x7F to the VCO Current Trimming register
	SpiWriteRegister(0x58, 0x80); //write 0x80 to the ChargepumpCurrentTrimmingOverride register
	SpiWriteRegister(0x59, 0x40); //write 0x40 to the Divider Current Trimming register
	//set the AGC
	SpiWriteRegister(0x6A, 0x0B); //write 0x0B to the AGC Override 2 register
	//set ADC reference voltage to 0.9V
	SpiWriteRegister(0x68, 0x04); //write 0x04 to the Deltasigma ADC Tuning 2 register
	SpiWriteRegister(0x1F, 0x03); //write 0x03 to the Clock Recovery Gearshift Override register
	//set Crystal Oscillator Load Capacitance register
	SpiWriteRegister(0x09, 0xD7); //write 0xD7 to the Crystal Oscillator Load Capacitance register

	//SpiWriteRegister(0x6d|0x80, 0x07);  // 设置为最大功率发射
	//SpiWriteRegister(0x6d|0x80, 0x06);  // 
	SpiWriteRegister(0x6d|0x80, 0x18|0x07);  // 


	//set si4432 receive signal level 
	SpiWriteRegister(SI4432_RSSI_THRESHOLD, 60);  //when rssi interrupt open, will generate  interrupt

	SI4432ResetTXFIFO( );

	XPRINTF((0,"INT Finish\r\n"));

	//config rf channel and txpower.
	prfCfg = get_node_rf_param( );
	if (NULL != prfCfg)
	{
		XPRINTF((0, "set rf channel and txpower\r\n"));
		SI4432SetChannel( prfCfg->ubRFChannel ); //change rf channel
		SI4432SetTxPower( prfCfg->ubTxPower );   //change rf tx power
	}
	
	SI4432RXPrepare( );//Receive mode
	XPRINTF((0, "si4432_int time  2 %d\r\n", clock_time( )));
}
#else
//40kbps
//SI4432 reg value is must be set with the feq,speed,fd and so on
//these value can get from the si4432 excel
void SI4432Init(void)
{

	u_char ItStatus1 = 0;
	u_char ItStatus2 = 0;
	RF_NODE_PARAM_CONFIG *prfCfg = NULL;
	XPRINTF((0, "si4432_int time %d\r\n", clock_time( )));
	#if 1
	SPI_Config( );
	SI4432SNDTurnON( );
	SI4432NSEL(LOW);
	clock_wait(20);
	ItStatus1 = SpiReadRegister(0x03); //read the Interrupt Status1 register
	ItStatus1 = SpiReadRegister(0x04); //read the Interrupt Status2 register
	
	SI4432ReadID( );
	#endif

	/* ======================================================== *
	* Initialize the Si4432 ISM chip *
	* ======================================================== */
	//SW reset
	
	SpiWriteRegister(0x07, 0x80);	//write 0x80 to the Operating & Function Control1 register
	XPRINTF((0, "SW reset\r\n"));
	#if 1
	//while (SI4432ReadReg(SI4432_DEVICE_TYPE)!= SI4432TPYPECODE);
	SI4432WaitInt( );
	SI4432WaitInt( );	
	clock_wait(50);
	ItStatus1 = SpiReadRegister(0x03); //read the Interrupt Status1 register
	ItStatus1 = SpiReadRegister(0x04); //read the Interrupt Status2 register
	
	#else
	while(SpiReadRegister(0x04)&&SI4432_ICHIPRDY == SI4432_ICHIPRDY);
	#endif

	#ifdef UNUSED_NIRQ_EXIT
	ItStatus1 = SpiReadRegister(0x03); //read the Interrupt Status1 register
	ItStatus1 = SpiReadRegister(0x04); //read the Interrupt Status2 register
	#endif
	XPRINTF((0,"INT_1\r\n"));
	SpiWriteRegister(0x06|0x80, 0x00);  	//  关闭不需要的中断
	SpiWriteRegister(0x07|0x80, SI4432_PWRSTATE_READY); // 进入 Ready 模式
	SpiWriteRegister(0x09|0x80, 0x7f);  	//  负载电容= 12P

	SpiWriteRegister(0x0a|0x80, 0x05);		// 关闭低频输出
	SpiWriteRegister(0x69, 0x60);  //AGC过载	

	//set 
	/*set the physical parameters*/
	//set the center frequency to 470 MHz 
	//if freq > 480, reg75.hbsel need to set 1
	SpiWriteRegister(0x75, 0x57); 	//write  to the Frequency Band Select register
	SpiWriteRegister(0x76, 0x00); 	//write  to the Nominal Carrier Frequency1 register
	SpiWriteRegister(0x77, 0x00); 	//write  to the Nominal Carrier Frequency0 register

	//set the desired TX data rate (40kbps)
	SpiWriteRegister(0x6E, 0x0a); //write  to the TXDataRate 1 register
	SpiWriteRegister(0x6F, 0x3d); //write  to the TXDataRate 0 register
	//SpiWriteRegister(0x70, 0x0C);//00 1100 //man code disable, data Rate > 30 kbs
	SpiWriteRegister(0x70, 0x01);//00 1100 //man code disable, data Rate > 30 kbs en writeing
	//SpiWriteRegister(0x72, 0x48);

	#if 0
	#else
	SpiWriteRegister(0x1C, 0xAD); //write  to the IF Filter Bandwidth register
	//SpiWriteRegister(0x1E, 0x0A); //default  
	SpiWriteRegister(0x20, 0x4b); //write  to the Clock Recovery Oversampling Ratio register
	SpiWriteRegister(0x21, 0x01); //default
	SpiWriteRegister(0x22, 0xb4); //
	SpiWriteRegister(0x23, 0xe8); //
	SpiWriteRegister(0x24, 0x01); //
	SpiWriteRegister(0x25, 0x86); //
	SpiWriteRegister(0x1D, 0x40); //enable AFC  default
	SpiWriteRegister(0x72, 0x23); //+-45kHz
	//SpiWriteRegister(0x72, 0x1f);
	#endif
	SpiWriteRegister(0x2a|0x80, 0x14);//AFC limiter
	//SpiWriteRegister(0x2a|0x80, 0x00);//AFC limiter  

	//set 
	/*set the packet structure and the modulation type*/
	//SpiWriteRegister(0x30|0x80, 0x8c);		// 使能PH+ FIFO模式，高位在前面，使能CRC校验 ccit
	SpiWriteRegister(0x30|0x80, 0xac);		// 使能PH+ FIFO模式，高位在前面，使能CRC校验 ccit
	//SpiWriteRegister(0x30|0x80, 0xad);		// 使能PH+ FIFO模式，高位在前面，使能CRC校验 ibm
	SpiWriteRegister(0x32|0x80, 0xff);		// byte0, 1,2,3 作为头码
	SpiWriteRegister(0x33|0x80, 0x42);		// byte 0,1,2,3 是头码，同步字3,2 是同步字
	SpiWriteRegister(0x34|0x80, 20);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte
	//SpiWriteRegister(0x34|0x80, 24);  		// 发射16个Nibble的Preamble  //5NIBBLE  = 2.5BYTE    4 channel 9.6k  12Byte //add 20141209 for more channels
	//SpiWriteRegister(0x35|0x80, 0x20);  	// 需要检测4个nibble的Preamble
	SpiWriteRegister(0x35|0x80, 0x40);  	// 需要检测8个nibble的Preamble
	SpiWriteRegister(0x36|0x80, 0x2d);  	// 同步字为 0x2dd4
	SpiWriteRegister(0x37|0x80, 0xd4);
	SpiWriteRegister(0x38|0x80, 0x00);
	SpiWriteRegister(0x39|0x80, 0x00);
	SpiWriteRegister(0x3a|0x80, 'h');   	// 发射的头码为： “"
	SpiWriteRegister(0x3b|0x80, 'r');
	SpiWriteRegister(0x3c|0x80, 'u');
	SpiWriteRegister(0x3d|0x80, 'n');
	SpiWriteRegister(0x3e|0x80, 10);    	// 总共发射10个字节的数据
	//SpiWriteRegister(0x3e|0x80, 16);    	// 总共发射16个字节的数据 //12 byte + 2 byte + 4 Byte add 20141209 for more channels
	SpiWriteRegister(0x3f|0x80, 'h');   	// 需要校验的头码为：”"
	SpiWriteRegister(0x40|0x80, 'r');
	SpiWriteRegister(0x41|0x80, 'u');
	SpiWriteRegister(0x42|0x80, 'n');
	SpiWriteRegister(0x43|0x80, 0xff);  // 头码1,2,3,4 的所有位都需要校验
	SpiWriteRegister(0x44|0x80, 0xff);  // 
	SpiWriteRegister(0x45|0x80, 0xff);  // 
	SpiWriteRegister(0x46|0x80, 0xff);  //

	//
	//SpiWriteRegister(0x56|0x80, 0x01);  //datasheet has no this reg

	//需要跳频时，设置0x79为物理信道编号，0x7a为信道间隔
	//SpiWriteRegister(0x79|0x80, 0x0);  // 不需要跳频
	//SpiWriteRegister(0x7a|0x80, 0x0);  // 不需要跳频

	SpiWriteRegister(0x79|0x80, 33);  // 需要跳频   channel number 470M-470Mhz + 33*200kHz
	SpiWriteRegister(0x7a|0x80, 20);  // 需要跳频    channel step  *10kHz
	
	SpiWriteRegister(0x71|0x80, 0x23); // 发射不需要 CLK，FiFo ， GFSK模式


	//set the TX FIFO almost full threshold to 54 bytes
	SpiWriteRegister(0x7C,SI4432TXFAFTHD);	
	//set the TX FIFO almost empty threshold to 10 bytes
	SpiWriteRegister(0x7D,SI4432TXFAETHD);														//write 0x0B to the TX FIFO Control 2 register
	//set the RX FIFO almost full threshold to 54 bytes
	SpiWriteRegister(0x7E,SI4432RXFAFTHD);	


	#if 1
	SpiWriteRegister(0x0C, 0x15); //write 0x12 to the GPIO1 Configuration(set the TX state)
	SpiWriteRegister(0x0D, 0x12); //write 0x15 to the GPIO2 Configuration(set the RX state)
	#endif

	/*set the non-default Si4432 registers*/
	//set the VCO and PLL
	SpiWriteRegister(0x5A, 0x7F); //write 0x7F to the VCO Current Trimming register
	SpiWriteRegister(0x58, 0x80); //write 0x80 to the ChargepumpCurrentTrimmingOverride register
	SpiWriteRegister(0x59, 0x40); //write 0x40 to the Divider Current Trimming register
	//set the AGC
	SpiWriteRegister(0x6A, 0x0B); //write 0x0B to the AGC Override 2 register
	//set ADC reference voltage to 0.9V
	SpiWriteRegister(0x68, 0x04); //write 0x04 to the Deltasigma ADC Tuning 2 register
	SpiWriteRegister(0x1F, 0x03); //write 0x03 to the Clock Recovery Gearshift Override register
	//set Crystal Oscillator Load Capacitance register
	SpiWriteRegister(0x09, 0xD7); //write 0xD7 to the Crystal Oscillator Load Capacitance register

	//SpiWriteRegister(0x6d|0x80, 0x07);  // 设置为最大功率发射
	//SpiWriteRegister(0x6d|0x80, 0x06);  // 
	SpiWriteRegister(0x6d|0x80, 0x18|0x07);  // 


	//set si4432 receive signal level 
	SpiWriteRegister(SI4432_RSSI_THRESHOLD, 60);  //when rssi interrupt open, will generate  interrupt

	SI4432ResetTXFIFO( );

	XPRINTF((0,"INT Finish\r\n"));

	//config rf channel and txpower.
	prfCfg = get_node_rf_param( );
	if (NULL != prfCfg)
	{
		XPRINTF((0, "set rf channel and txpower\r\n"));
		SI4432SetChannel( prfCfg->ubRFChannel ); //change rf channel
		SI4432SetTxPower( prfCfg->ubTxPower );   //change rf tx power
	}
	
	SI4432RXPrepare( );//Receive mode
	XPRINTF((0, "si4432_int time  2 %d\r\n", clock_time( )));
}

#endif





