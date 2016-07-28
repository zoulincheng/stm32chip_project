#include "basictype.h"
#include "si4438.h"
#include "bsp.h"
#include "si4438_int_ctr_def.h"
//#inlcude ""
#include "em32lg_config.h"
#include "sysprintf.h"

#include "si446x_defs_u.h"

#define RF_NIRQ_ENABLE()	//SI4432_Enable_Interrupt( ) 
#define RF_NIRQ_DISABLE()	//SI4432_Disable_Interrupt( )

volatile u_char si446x_ubpksent = 0;
//static u_char ubaPacketBuf[130];

void si446xStartRX(void);
//wait SI4438_pksent interrupt,finish a paket sent.
//for send data

void si446x_set_pksent(void)
{
	si446x_ubpksent = 1;
}


static int si446x_wait_pksent(void)
{
	#if 1
	u_long timeout = 0;
	si446x_ubpksent = 0;
	while((!si446x_ubpksent))
	{
		timeout++;
		if ( timeout > 720000)//time out
		{
			XPRINTF((10,"send error 1\n"));
			return 1;
		}
	}
	si446x_ubpksent = 0;
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


//close nirq pin interrupt
void si446x_nirq_disable(void)
{
	RF_NIRQ_DISABLE();
}


#define SI446XNIRQPORT	GPIOC
#define	SI446XNIRQPIN	GPIO_Pin_6

/**************************************************************************//**
 * @brief Setup GPIO interrupt to set the time
 *****************************************************************************/
void si446x_gpioSetup(void)
{
	u_long nirq_Priority = 0;
	/* Configure  */
	GPIO_PinModeSet(RF_nIRQ_PORT, RF_nIRQ_PIN, gpioModeInput, 0);

	/* Set falling edge interrupt  */
	GPIO_IntConfig(RF_nIRQ_PORT, RF_nIRQ_PIN, false, true, true);

	/* Enable interrupt in core for even and odd gpio interrupts */
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
	nirq_Priority = NVIC_EncodePriority(INT_RF_nIRQ_GROUP, INT_RF_nIRQ_PREP, INT_RF_nIRQ_SUBP);
	NVIC_SetPriority(GPIO_EVEN_IRQn, nirq_Priority);
//	NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
//	NVIC_EnableIRQ(GPIO_ODD_IRQn);
}




void si446x_nIRQ_Config(void)
{
	si446x_gpioSetup( );
}



//open nirq pin interrupt
void si446x_nirq_enable(void)
{
	RF_NIRQ_ENABLE();
}

#define SEND_PACKET_TIMEOUT		(240000)



#define HWLH_TEST_CH	6

#if 0
void si446xRadioInit(void)
{
	SPI_Config( );
    SI446X_RESET( );        //SI446X 模块复位
    SI446X_CONFIG_INIT( );  //SI446X 模块初始化配置函数
    si446x_nIRQ_Config( );
    si446xStartRX( );
}


void si446xStartRX(void)
{
	//SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PRX_EN|SI446X_INT_CTL_PH_CRCE_EN|SI446X_INT_CTL_PH_RFAF_EN);
	#if 1
    SI446X_SET_PROPERTY_1( PKT_CONFIG1, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_LEN_FIELD_SOURCE, 0x01 );
    SI446X_SET_PROPERTY_1( PKT_LEN, 0x2A );
    SI446X_SET_PROPERTY_1( PKT_LEN_ADJUST, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, 0x01 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_12_8, 0x00 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x10 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x80 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_CONFIG, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_CRC_CONFIG, 0x00 );	
	#endif
	SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PRX_EN|SI446X_INT_CTL_PH_CRCE_EN);
	
	SI446X_START_RX( HWLH_TEST_CH, 0, 0,0,0,3 );
}

/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
int si446xRadioSend(const u_char *pubdata)
{
	//when data length > 64, 
	u_char ubPH;
	u_char ubDataNLen = 0;
	const u_char *pbuf = pubdata;
	int sdwSendTimeFlag = 0;
	int nResult = 0;
	ubDataNLen = pubdata[0] + 1;

#if 0
	//Fill data to FIFO first,frame length < 64
	if(ubDataNLen < 64 )
	{
		//SI446X_SEND_PACKET( ubTdata, sizeof(ubTdata), 2, 0 );
		SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
		nResult = SI446X_SEND_PACKET(pubdata, ubDataNLen, 2, 0);
	}
	else
	{
		SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
		ubPH = SI446X_GET_PROPERTY_1(INT_CTL_PH_ENABLE);
		XPRINTF((0, "ubPH = %02x\r\n", ubPH));
		SI446X_SEND_DATA(pbuf, ubDataNLen, 2, 0);
		//ubDataNLen -= SI446X_FIFO_SIZE; 
		//pbuf += SI446X_FIFO_SIZE;
		nResult = si446x_wait_pksent( );
		#if 0
		while( ubDataNLen > SI446X_FIFO_TFAF_TH)
		{
			si446xWriteMutiData(pbuf, SI446X_FIFO_TFAF_TH);
			if (ubDataNLen > SI446X_FIFO_TFAF_TH)
			{
				ubDataNLen = ubDataNLen - SI446X_FIFO_TFAF_TH;
			}
			pbuf = pbuf+SI446X_FIFO_TFAF_TH;
			nResult = si446x_wait_txafe( );
		}
		//si446xWriteMutiData(pbuf, SI446X_FIFO_TFAF_TH);
		#endif
		//SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
	}
#endif
	SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
	ubPH = SI446X_GET_PROPERTY_1(INT_CTL_PH_ENABLE);
	XPRINTF((0, "ubPH = %02x\r\n", ubPH));
	SI446X_SEND_DATA(pbuf, ubDataNLen, 2, 0);
	nResult = si446x_wait_pksent( );
	return nResult;
}



/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
int si446xRadioSendFixLenData(const u_char *pubdata)
{
	//when data length > 64, 
	u_char ubPH;
	u_char ubDataNLen = 0;
	const u_char *pbuf = pubdata;
	int sdwSendTimeFlag = 0;
	int nResult = 0;
	ubDataNLen = pubdata[0] + 1;

	//MEM_DUMP(0, "->tx", pubdata, ubDataNLen);
	//SI446X_SET_PROPERTY_1();
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0xA2 );
    SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x05 );

	SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, ubDataNLen);
	SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PSENT_EN);
	//ubPH = SI446X_GET_PROPERTY_1(INT_CTL_PH_ENABLE);
	//XPRINTF((0, "ubPH = %02x\r\n", ubPH));
	SI446X_SEND_DATA(pbuf, ubDataNLen, HWLH_TEST_CH, 0);
	nResult = si446x_wait_pksent( );

	si446xStartRX( );
	return nResult;
}
#else


void si446xRadioInit(void)
{
	SPI_Config( );
    SI446X_RESET( );        //SI446X 模块复位
    
    SI446X_RADIO_CONFIG_INIT( ); //config radio param

    SI446X_RADIO_PKT_COMMON_CFG( );
    SI446X_RADIO_TX_FIELD_CFG( );
    SI446X_RADIO_RX_FIELD_CFG( );
    
    si446x_nIRQ_Config( );
    si446xStartRX( );
}

void si446xStartRX( )
{
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

	SI446X_START_RX( HWLH_TEST_CH, 0, 0,0,0,3 );
}


void si446x_read_fifo(void)
{
	u_char i = 0;
	u_char ubaBuf[128] = {0x00};
	//XPRINTF((10, "ubLen = %02x\r\n", ubLen));
	SI446X_WAIT_CTS( );

    RF_SPINSEL(0);
    SPI_SendByteData( READ_RX_FIFO );
    
    for (i = 0; i < 128; i++)
    {
    	ubaBuf[i]=SPI_SendByteData(0xFF);
    }
	RF_SPINSEL(1);

	MEM_DUMP(0, "CRCE", ubaBuf, 128);
}


/*----------------------------------------------------------------------------*/
/** \brief  This function will download a frame to the radio transceiver's frame
 *          buffer.
 *
 *  \param  data        Pointer to data that is to be written to frame buffer.
 *  \param  len         Length of data. The maximum length is large 64 bytes.
 */
int si446xRadioSendFixLenData(const u_char *pubdata)
{
	//when data length > 64, 
	u_char ubPH;
	u_char ubDataNLen = 0;
	const u_char *pbuf = pubdata;
	int sdwSendTimeFlag = 0;
	int nResult = 0;
	int i = 0;
	
	ubDataNLen = pubdata[0] + 1;

    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    RF_SPINSEL(0);
    SPI_SendByteData(WRITE_TX_FIFO);
    for (i = 0; i < ubDataNLen; i++)
    {
    	SPI_SendByteData( pbuf[i]); 
    }
    RF_SPINSEL(1);

	//si446x_read_fifo( );
	SI446X_START_TX(HWLH_TEST_CH, 0, ubDataNLen);
	nResult = si446x_wait_pksent( );

	si446xStartRX( );
	return nResult;
}

#endif




void si446xChangeState(u_char ubNextState)
{
	u_char ubacmd[2] = {0x00};
	ubacmd[0] = CHANGE_STATE;
    ubacmd[1] = ubNextState;
    SI446X_CMD(ubacmd, 2);
}



void si446xWriteMutiData(const u_char *pcBuf, u_char ubDataLen)
{
	u_char i = 0;
	SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
    #if 0
    while( ubDataLen -- )
    {
        SPI_SendByteData( *pcBuf++ );
    }
    #else
    for (i = 0; i < ubDataLen; i++)
    {
    	SPI_SendByteData( pcBuf[i]);
    }
    #endif
	RF_SPINSEL(1);
}





void si446xReadMutiData(u_char *pBuf)
{
	u_char i = 0;
	u_char ubLen = 0;
	//u_char *pbuf = (u_char *)&pBuf[1];
//	ubLen = SI446X_GET_FIFO_INFO_RX_BYTES();
	ubLen = SI446X_GET_FIFO_INFO_RX_BYTES();
//	XPRINTF((10, "ubLen = %02x\r\n", ubLen));
	SI446X_WAIT_CTS( );

    RF_SPINSEL(0);
    SPI_SendByteData( READ_RX_FIFO );
    //pBuf[0]=SPI_SendByteData(0xFF);
    #if 0
    while( ubDataLen -- )
    {
        //*buffer++ = SPI_SendByteData( 0xFF );
        *pBuf++ = SPI_SendByteData( 0xFF );
    }
    #else
    //error rx
    
    if (ubLen+1 > 128 )
    {
    	pBuf[0] = 0;
    	return;
    }
    for (i = 0; i < ubLen; i++)
    {
    	pBuf[i]=SPI_SendByteData(0xFF);
    }
    #endif
	RF_SPINSEL(1);
	//MEM_DUMP(0, "rxda", pBuf, ubLen);
}






#if 0
void EXTI9_5_IRQHandler(void)
{	
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		U8 si446x_ph_pend;
		U8 si446x_modem_pend;
		U8 si446x_chip_pend;
		U8 ubph_en;
		U8 ubmodem_en;
		U8 ubph_flag;
		U8 ubmodem_flag;

		EXTI_ClearITPendingBit(EXTI_Line6);	
				
		SI446X_GET_INT_STATUS( );
		si446x_ph_pend = Si446xCmd.GET_INT_STATUS.PH_PEND;
		si446x_modem_pend = Si446xCmd.GET_INT_STATUS.MODEM_PEND;
		si446x_chip_pend = Si446xCmd.GET_INT_STATUS.CHIP_PEND;
		ubph_en = SI446X_GET_PROPERTY_1(INT_CTL_PH_ENABLE);
		ubmodem_en = SI446X_GET_PROPERTY_1(INT_CTL_MODEM_ENABLE);

		ubph_flag = ubph_en&si446x_ph_pend;
		ubmodem_flag = ubmodem_en&si446x_modem_pend;

		//packet sent finish
		//if ((si446x_ph_pend& SI446X_INT_CTL_PH_PSENT_EN) == SI446X_INT_CTL_PH_PSENT_EN)//packet sent
		if ((ubph_flag& SI446X_INT_CTL_PH_PSENT_EN) == SI446X_INT_CTL_PH_PSENT_EN)//packet sent
		{
			XPRINTF((0, "Send\r\n"));
			si446x_ubpksent = 1;
		}
		
		//rf
		#if 1
		//else if ((si446x_ph_pend& SI446X_INT_CTL_PH_RFAF_EN) == SI446X_INT_CTL_PH_RFAF_EN)
		else if ((ubph_flag& SI446X_INT_CTL_PH_RFAF_EN) == SI446X_INT_CTL_PH_RFAF_EN)
		{

		}
		#endif
		//packet rx finish
		//else if ((si446x_ph_pend & SI446X_INT_CTL_PH_PRX_EN) == SI446X_INT_CTL_PH_PRX_EN)
		else if ((ubph_flag & SI446X_INT_CTL_PH_PRX_EN) == SI446X_INT_CTL_PH_PRX_EN)
		{
			//u_char ubLen = 0;
			//XPRINTF((0, "2 ph=%02x mo=%02x ch=%02x\r\n", si446x_ph_pend, si446x_modem_pend, si446x_chip_pend));
			XPRINTF((0, "ubph_flag = %02x\r\n", ubph_flag));
			si446xReadMutiData(ubaPacketBuf);
			MEM_DUMP(0, "<-rx", ubaPacketBuf, ubaPacketBuf[0]+1);
			memset(ubaPacketBuf, 0 , 128);
			si446xStartRX();
		}
		//packet crc error
		//else if ((si446x_ph_pend & SI446X_INT_CTL_PH_CRCE_EN) == SI446X_INT_CTL_PH_CRCE_EN)
		else if ((ubph_flag & SI446X_INT_CTL_PH_CRCE_EN) == SI446X_INT_CTL_PH_CRCE_EN)
		{
			XPRINTF((0, "crce\r\n"));	
			//SI446X_START_RX( 2, 0, 0,0,0,3 );
			si446xStartRX();
		}

		//else if ((si446x_ph_pend & SI446X_INT_CTL_PH_TFAE_EN) == SI446X_INT_CTL_PH_TFAE_EN)
		else if ((ubph_flag & SI446X_INT_CTL_PH_TFAE_EN) == SI446X_INT_CTL_PH_TFAE_EN)
		{
		}
		
		//if ((si446x_modem_pend & SI446X_INT_CTL_MODEM_SYDE_EN) == SI446X_INT_CTL_MODEM_SYDE_EN)
		if ((ubmodem_flag & SI446X_INT_CTL_MODEM_SYDE_EN) == SI446X_INT_CTL_MODEM_SYDE_EN)
		{
			u_char ubrssi = 0;
			SI446X_GET_MODEM_STATUS(0x00);
			ubrssi = Si446xCmd.GET_MODEM_STATUS.CURR_RSSI;
			XPRINTF((0, "syde\r\n"));
			XPRINTF((0, "ubmodem_flag = %02x rssi is %d  latchrssi %d\r\n", ubmodem_flag, ubrssi, Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI));
			ubaPacketBuf[0] = 0;
		}
		memset(Si446xCmd.RAW, 0, 16);
	}
}
#endif


