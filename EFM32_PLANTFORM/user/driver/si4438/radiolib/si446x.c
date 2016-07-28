#include "contiki.h"

#include "sysprintf.h"
#include "basictype.h"
#include "si4438.h"

#include "si446x_defs_u.h"
#include "si446x.h"

#include "bsp.h"
#include "si4438_int_ctr_def.h"

#include "em32lg_config.h"


void SI446X_RX_FIFO_RESET( void );
void SI446X_TX_FIFO_RESET( void );
void SI446X_TXRX_FIFO_RESET( void );
void SI446X_GLOGAL_CFG_CMMBINED_FIFO(void);
void SI446X_GPIO_CONFIG( INT8U G0, INT8U G1, INT8U G2, INT8U G3,INT8U IRQ, INT8U SDO, INT8U GEN_CONFIG );
const u8 si446x_cfg[] = RADIO_CONFIGURATION_DATA_ARRAY;
extern u8 SPI_SendByteData(u8 byte);

/*
=================================================================================
SI446X_WAIT_CTS( );
Function : wait the device ready to response a command
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
void SI446X_WAIT_CTS( void )
{
    INT8U cts;
    do
    {
        RF_SPINSEL(0);
        SPI_SendByteData( READ_CMD_BUFF );
        cts = SPI_SendByteData(0xFF);
        RF_SPINSEL(1);
    }while( cts != 0xFF );
}


/*
=================================================================================
SI446X_CMD( );
Function : Send a command to the device
INTPUT   : cmd, the buffer stores the command array
           cmdsize, the size of the command array
OUTPUT   : NONE
=================================================================================
*/

static void SI446X_CMD( INT8U *cmd, INT8U cmdsize )
{
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
    while( cmdsize -- )
    {
        SPI_SendByteData( *cmd++ );
    }
	RF_SPINSEL(1);
}


/*
=================================================================================
SI446X_POWER_UP( );
Function : Power up the device
INTPUT   : f_xtal, the frequency of the external high-speed crystal
OUTPUT   : NONE
=================================================================================
*/
void SI446X_POWER_UP( INT32U f_xtal )
{
    INT8U cmd[7];
    cmd[0] = POWER_UP;
    cmd[1] = 0x01;
    cmd[2] = 0x00;
    cmd[3] = f_xtal>>24;
    cmd[4] = f_xtal>>16;
    cmd[5] = f_xtal>>8;
    cmd[6] = f_xtal;
    SI446X_CMD( cmd, 7 );
}




/*
=================================================================================
SI446X_READ_RESPONSE( );
Function : read a array of command response
INTPUT   : buffer,  a buffer, stores the data responsed
           size,    How many bytes should be read
OUTPUT   : NONE
=================================================================================
*/
void SI446X_READ_RESPONSE( INT8U *buffer, INT8U size )
{
	//u_char ubCTS = 0x00;
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);
	SPI_SendByteData( READ_CMD_BUFF );
	//ubCTS = SPI_SendByteData( 0xFF );
	//if (ubCTS == 0xff)
	{
		while( size -- )
	    {
	        *buffer++ = SPI_SendByteData( 0xFF );
	    }
    }
    RF_SPINSEL(1);
}



/*
=================================================================================
SI446X_NOP( );
Function : NO Operation command
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
INT8U SI446X_NOP( void )
{
    INT8U cts;
    RF_SPINSEL(0);
    cts = SPI_SendByteData( NOP );
    RF_SPINSEL(1);
	return cts;
}




/*
=================================================================================
SI446X_PART_INFO( );
Function : Read the PART_INFO of the device, 8 bytes needed
INTPUT   : buffer, the buffer stores the part information
OUTPUT   : NONE
=================================================================================
*/
void SI446X_PART_INFO( INT8U *buffer )
{
    INT8U cmd = PART_INFO;
    //u_char ubData[16] = {0x00};
    SI446X_CMD( &cmd, 1 );
    SI446X_READ_RESPONSE(buffer, 9);
    //SI446X_READ_RESPONSE(ubData, 8);
    //MEM_DUMP(0,"part", ubData, 8);
}



/*
=================================================================================
SI446X_FUNC_INFO( );
Function : Read the FUNC_INFO of the device, 7 bytes needed
INTPUT   : buffer, the buffer stores the FUNC information
OUTPUT   : NONE
=================================================================================
*/
void SI446X_FUNC_INFO( INT8U *buffer )
{
    INT8U cmd = FUNC_INFO;
    SI446X_CMD( &cmd, 1 );
    SI446X_READ_RESPONSE( buffer, 7 );
}



/*
=================================================================================
SI446X_INT_STATUS( );
Function : Read the INT status of the device, 9 bytes needed
INTPUT   : buffer, the buffer stores the int status
OUTPUT   : NONE
=================================================================================
*/
void SI446X_INT_STATUS( INT8U *buffer )
{
    INT8U cmd[4];
    cmd[0] = GET_INT_STATUS;
    cmd[1] = 0;
    cmd[2] = 0;
    cmd[3] = 0;

    SI446X_CMD( cmd, 4 );
    SI446X_READ_RESPONSE( buffer, 9 );
}







/*
=================================================================================
SI446X_GET_PROPERTY( );
Function : Read the PROPERTY of the device
INTPUT   : buffer, the buffer stores the PROPERTY value
           GROUP_NUM, the group and number of the parameter
           NUM_GROUP, number of the group
OUTPUT   : NONE
=================================================================================
*/
void SI446X_GET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, INT8U NUM_PROPS, INT8U *buffer  )
{
    INT8U cmd[4];

    cmd[0] = GET_PROPERTY;
    cmd[1] = GROUP_NUM>>8;
    cmd[2] = NUM_PROPS;
    cmd[3] = GROUP_NUM;

    SI446X_CMD( cmd, 4 );
    SI446X_READ_RESPONSE( buffer, NUM_PROPS + 1 );
}




/*
=================================================================================
SI446X_SET_PROPERTY_X( );
Function : Set the PROPERTY of the device
INTPUT   : GROUP_NUM, the group and the number of the parameter
           NUM_GROUP, number of the group
           PAR_BUFF, buffer stores parameters
OUTPUT   : NONE
=================================================================================
*/
void SI446X_SET_PROPERTY_X( SI446X_PROPERTY GROUP_NUM, INT8U NUM_PROPS, INT8U *PAR_BUFF )
{
    INT8U  cmd[20], i = 0;
    if( NUM_PROPS >= 16 )   { return; }
    cmd[i++] = SET_PROPERTY;
    cmd[i++] = GROUP_NUM>>8;
    cmd[i++] = NUM_PROPS;
    cmd[i++] = GROUP_NUM;
    while( NUM_PROPS-- )
    {
        cmd[i++] = *PAR_BUFF++;
    }
    SI446X_CMD( cmd, i );
}



/*
=================================================================================
SI446X_SET_PROPERTY_1( );
Function : Set the PROPERTY of the device, only 1 byte
INTPUT   : GROUP_NUM, the group and number index
           proerty,  the value to be set
OUTPUT   : NONE
=================================================================================
*/
void SI446X_SET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM, INT8U proerty )
{
    INT8U  cmd[5];
    
	//SI446X_WAIT_CTS( );
	
    cmd[0] = SET_PROPERTY;
    cmd[1] = GROUP_NUM>>8;
    cmd[2] = 1;
    cmd[3] = GROUP_NUM;
    cmd[4] = proerty;
    SI446X_CMD( cmd, 5 );
}




/*
=================================================================================
SI446X_GET_PROPERTY_1( );
Function : Get the PROPERTY of the device, only 1 byte
INTPUT   : GROUP_NUM, the group and number index
OUTPUT   : the PROPERTY value read from device
=================================================================================
*/
INT8U SI446X_GET_PROPERTY_1( SI446X_PROPERTY GROUP_NUM )
{
    INT8U  cmd[4];

    cmd[0] = GET_PROPERTY;
    cmd[1] = GROUP_NUM>>8;
    cmd[2] = 1;
    cmd[3] = GROUP_NUM;
    SI446X_CMD( cmd, 4 );
    SI446X_READ_RESPONSE( cmd, 2 );
    return cmd[1];
}
/*
=================================================================================
SI446X_RESET( );
Function : reset the SI446x device
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
void SI446X_RESET( void )
{
    //INT16U x = 255;
   
    RF_SDN(1);
    //while( x-- );
    clock_wait(1);
    RF_SDN(0);
    clock_wait(50);
    RF_SPINSEL(1);
}


/*!
 * This function is used to load all properties and commands with a list of NULL terminated commands.
 * Before this function @si446x_reset should be called.
 */
U8 SI446X_CFG_PARAM_INIT(const U8* pSetPropCmd)
{
	U8 col;
	U8 numOfBytes;
	u_char ubacmd[16];

	/* While cycle as far as the pointer points to a command */
	while (*pSetPropCmd != 0x00)
	{
		/* Commands structure in the array:
		* --------------------------------
		* LEN | <LEN length of data>
		*/

		numOfBytes = *pSetPropCmd++;

		if (numOfBytes > 16u)
		{
			/* Number of command bytes exceeds maximal allowable length */
			return SI446X_COMMAND_ERROR;
		}

		for (col = 0u; col < numOfBytes; col++)
		{
			ubacmd[col] = *pSetPropCmd;
			pSetPropCmd++;
		}
		MEM_DUMP(0, "CMD", ubacmd, numOfBytes);
		SI446X_CMD(ubacmd, numOfBytes);
	}

	return SI446X_SUCCESS;
}



/*
=================================================================================
SI446X_CONFIG_INIT( );
Function : configuration the device
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
void SI446X_CONFIG_INIT( void )
{
	#if 0
    INT8U i;
    INT16U j = 0;
	u_char *pcfg = (u_char *)si446x_cfg;
    while( ( i = pcfg[j] ) != 0 )
    {
        j += 1;
        SI446X_CMD( pcfg + j, i );
        j += i;
    }
	#else
	SI446X_CFG_PARAM_INIT(si446x_cfg);
	#endif
    
#if PACKET_LENGTH > 0           //fixed packet length
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, PACKET_LENGTH );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0xA2 );
    SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x05 );
#else                           //variable packet length
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
    #else
    //SI446X_SET_PROPERTY_1( PKT_CONFIG1, 0x00 ); //crc not ivert,lisbyte first,msbit first
    //SI446X_SET_PROPERTY_1( PKT_CRC_CONFIG, 0x04 );//crc seed 0, CRC_16_IBM
    SI446X_SET_PROPERTY_1( PKT_LEN_FIELD_SOURCE, 0x01 );//field is packet length
    SI446X_SET_PROPERTY_1( PKT_LEN, 0x2A );//msbyte first,1byte len,len save fifo,field 2 is var field
    SI446X_SET_PROPERTY_1( PKT_LEN_ADJUST, (INT8U)-3);//0xfd 1 byte packet field, 2 bytes crc
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_12_8, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_1_LENGTH_7_0, 0x01 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_1_CONFIG, 0x00 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_1_CRC_CONFIG, 0x80|0x02);//start crc from this field, and enable
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_12_8, 0x00 );
    SI446X_SET_PROPERTY_1( PKT_FIELD_2_LENGTH_7_0, 0x10);//max byte 129
    //SI446X_SET_PROPERTY_1( PKT_FIELD_2_CONFIG, 0x00 );
    //SI446X_SET_PROPERTY_1( PKT_FIELD_2_CRC_CONFIG, 0x20|0x08|0x02);//send crc, check crc, crc enable
    #endif
#endif //PACKET_LENGTH

    //重要： 4463的GDO2，GDO3控制射频开关，  0X20 ,0X21 
    //发射时必须： GDO2=1，GDO3=0
    //接收时必须： GDO2=0，GDO3=1
    SI446X_GPIO_CONFIG( 0, 0, 0x20, 0x21, 0, 0, 0 );//重要

}


/*
=================================================================================
SI446X_RADIO_CONFIG_INIT( );
Function : configuration the device
INTPUT   : NONE
OUTPUT   : NONE
=================================================================================
*/
void SI446X_RADIO_CONFIG_INIT( void )
{
	SI446X_CFG_PARAM_INIT(si446x_cfg);
}

/*
=================================================================================
SI446X_RADIO_PKT_COMMON_CFG( );
Function : configuration pkt tx and rx common
INTPUT   : NONE
OUTPUT   : NONE
0x120d-0x1220
=================================================================================
*/
void SI446X_RADIO_PKT_COMMON_CFG( void )
{
    SI446X_SET_PROPERTY_1(PKT_CRC_CONFIG, 0x05);//used all 0 for crc seed,CCIT-16: X16+X12+X5+1.
    SI446X_SET_PROPERTY_1(PKT_CONFIG1,0x80);//filed split,packet handler enable, manch 01, crc no invert, crc low byte first, MSB first
}

/*
=================================================================================
SI446X_RADIO_TX_FIELD_CFG( );
Function : configuration tx field data
INTPUT   : NONE
OUTPUT   : NONE
0x120d-0x1220
=================================================================================
*/
void SI446X_RADIO_TX_FIELD_CFG( void )
{
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_LENGTH_7_0, 0x01);  //1 byte for packet length
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_CONFIG, 0x00); //
	SI446X_SET_PROPERTY_1(PKT_FIELD_1_CRC_CONFIG, 0x80);//start crc, disable crc

	SI446X_SET_PROPERTY_1(PKT_FIELD_2_LENGTH_7_0, 0x80);//max length 128
	SI446X_SET_PROPERTY_1(PKT_FIELD_2_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(PKT_FIELD_2_CONFIG, 0x00);
	SI446X_SET_PROPERTY_1(PKT_FIELD_2_CRC_CONFIG, 0x22);//send crc, enable crc

	//open rx ,tx ,crc, syde 
	SI446X_SET_PROPERTY_1(INT_CTL_PH_ENABLE, SI446X_INT_CTL_PH_PRX_EN|SI446X_INT_CTL_PH_CRCE_EN|SI446X_INT_CTL_PH_PSENT_EN);
	SI446X_SET_PROPERTY_1(INT_CTL_MODEM_ENABLE, SI446X_INT_CTL_MODEM_SYDE_EN);
}


/*
=================================================================================
SI446X_RADIO_RX_FIELD_CFG( );
Function : configuration rx field data
INTPUT   : NONE
OUTPUT   : NONE
0x1221-0x1234
=================================================================================
*/
void SI446X_RADIO_RX_FIELD_CFG( void )
{
	SI446X_SET_PROPERTY_1(PKT_LEN, 0x2a);//rx msb first, 1 byte length,length put in fifo, field 2 is variable length field.
	SI446X_SET_PROPERTY_1(PKT_LEN_FIELD_SOURCE, 0x01);//field is packet length
	SI446X_SET_PROPERTY_1(PKT_LEN_ADJUST, (INT8U)(0x00));//paket format is l byte length, packet data, 2 byte crc

	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_LENGTH_7_0, 0x01);//1byte packet length
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_LENGTH_12_8, 0x00);//
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_CONFIG, 0x00);
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_1_CRC_CONFIG, 0x00);//start crc, enable crc

	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_LENGTH_12_8, 0x00);
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_LENGTH_7_0, 0x80);//128
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_CONFIG, 0x00);
	SI446X_SET_PROPERTY_1(PKT_RX_FIELD_2_CRC_CONFIG, 0x00);//check crc, enable crc
}




/*
=================================================================================
SI446X_W_TX_FIFO( );
Function : write data to TX fifo
INTPUT   : txbuffer, a buffer stores TX array
           size,  how many bytes should be written
OUTPUT   : NONE
=================================================================================
*/
void SI446X_W_TX_FIFO( INT8U *txbuffer, INT8U size )
{
    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
    while( size -- )    
    { 
    	SPI_SendByteData( *txbuffer++ ); 
    }
    RF_SPINSEL(1);
}
/*
=================================================================================
SI446X_SEND_PACKET( );
Function : send a packet
INTPUT   : txbuffer, a buffer stores TX array
           size,  how many bytes should be written
           channel, tx channel
           condition, tx condition
OUTPUT   : sent flag : 1 success, 0 failed
=================================================================================
*/
int  SI446X_SEND_PACKET( INT8U *txbuffer, INT8U size, INT8U channel, INT8U condition )
{
    INT8U cmd[5];
    INT8U tx_len = size;
    int nResult = 0;

//    SI446X_TX_FIFO_RESET( );
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
#if PACKET_LENGTH == 0
    //tx_len ++;
    tx_len += 1;//1 byte length, 2byte crc
    SPI_SendByteData( size );
#endif
    while( size -- )    
    { 
    	SPI_SendByteData( *txbuffer++ ); 
    }
    RF_SPINSEL(1);
    cmd[0] = START_TX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = 0;
    cmd[4] = tx_len;
    SI446X_CMD( cmd, 5 );
	#if 0
    while(RF_NIRQS!=0)
    {
    	XPRINTF((0, "W\r\n"));
    }
    #else
    nResult = si446x_wait_pksent( );
    #endif
    RF_SPINSEL(1);
    cmd[0] = CHANGE_STATE;
    cmd[1] = 0x03;
    SI446X_CMD( cmd, 2 );

    return nResult;
}

/*
=================================================================================
SI446X_SEND_DATA( );
Function : send data , data length > 64
INTPUT   : txbuffer, a buffer stores TX array
           size,  how many bytes should be written
           channel, tx channel
           condition, tx condition
OUTPUT   : sent flag : 1 success, 0 failed
=================================================================================
*/
void  SI446X_SEND_DATA( INT8U *txbuffer, INT8U size, INT8U channel, INT8U condition )
{
    INT8U cmd[5];
    INT8U tx_len = size;
    INT8U i = 0;
    int nResult = 0;

//    SI446X_TX_FIFO_RESET( );
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    RF_SPINSEL(0);
    SPI_SendByteData( WRITE_TX_FIFO );
#if PACKET_LENGTH == 0
    //tx_len ++;
    tx_len += 1;//
    SPI_SendByteData( size );
#endif
	#if 0
    while( size -- )    
    { 
    	SPI_SendByteData( *txbuffer++ ); 
    }
    #endif
    for (i = 0; i < size; i++)
    {
    	SPI_SendByteData( txbuffer[i]); 
    }
    RF_SPINSEL(1);
    cmd[0] = START_TX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = 0;
    cmd[4] = tx_len;
    SI446X_CMD( cmd, 5 );
	RF_SPINSEL(1);
}




/*
=================================================================================
SI446X_START_TX( );
Function : start TX command
INTPUT   : channel, tx channel
           condition, tx condition
           tx_len, how many bytes to be sent
OUTPUT   : NONE
=================================================================================
*/
void SI446X_START_TX( INT8U channel, INT8U condition, INT16U tx_len )
{
    INT8U cmd[5];

    cmd[0] = START_TX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = tx_len>>8;
    cmd[4] = tx_len;
    SI446X_CMD( cmd, 5 );
}
/*
=================================================================================
SI446X_READ_PACKET( );
Function : read RX fifo
INTPUT   : buffer, a buffer to store data read
OUTPUT   : received bytes
=================================================================================
*/
INT8U SI446X_READ_PACKET( INT8U *buffer )
{
    INT8U length = 0;
    INT8U i = 0;;
    //user add
    //INT8U packetLen = 0;
    INT8U *pBuf = buffer+1;
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);

    SPI_SendByteData( READ_RX_FIFO );
#if PACKET_LENGTH == 0
    length = SPI_SendByteData( 0xFF );
    //user add
    
    buffer[0] = SPI_SendByteData( 0xFF );

    XPRINTF((0, "L = %02x, bL = %02x\r\n", length, buffer[0]));
    if (length >= (buffer[0] + 1))
    	length = buffer[0];
    else
    	length = buffer[0];
#else
    length = PACKET_LENGTH;
#endif
    i = length;
    while( length -- )
    {
        //*buffer++ = SPI_SendByteData( 0xFF );
        *pBuf++ = SPI_SendByteData( 0xFF );
    }
    RF_SPINSEL(1);
    return i;
}

void SI446X_READ_PACKET_LEN( INT8U *pibuf )
{
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);

    SPI_SendByteData( READ_RX_FIFO );
#if PACKET_LENGTH == 0
    pibuf[0] = SPI_SendByteData( 0xFF );
    pibuf[1] = SPI_SendByteData( 0xFF );
    RF_SPINSEL(1);
#endif
}




/*
=================================================================================
SI446X_READ_DATA_TO_BUF( );
Function : read RX fifo
INTPUT   : buffer, a buffer to store data read
OUTPUT   : received bytes
=================================================================================
*/
INT8U SI446X_READ_DATA_TO_BUF( INT8U *pbuf , INT8U ubNum)
{
	INT8U length;
    INT8U i = 0;
    SI446X_WAIT_CTS( );
    RF_SPINSEL(0);

    SPI_SendByteData( READ_RX_FIFO );
    length = ubNum;
    i = length;
    while( length -- )
    {
        //*buffer++ = SPI_SendByteData( 0xFF );
        *pbuf++ = SPI_SendByteData( 0xFF );
    }
    RF_SPINSEL(1);
    return i;
}



/*
=================================================================================
SI446X_START_RX( );
Function : start RX state
INTPUT   : channel, receive channel
           condition, receive condition
           rx_len, how many bytes should be read
           n_state1, next state 1
           n_state2, next state 2
           n_state3, next state 3
OUTPUT   : NONE
=================================================================================
*/
void SI446X_START_RX( INT8U channel, INT8U condition, INT16U rx_len,
                      INT8U n_state1, INT8U n_state2, INT8U n_state3 )
{
    INT8U cmd[8];
    //SI446X_RX_FIFO_RESET( );
    //SI446X_TX_FIFO_RESET( );
    SI446X_TXRX_FIFO_RESET( );
    SI446X_GLOGAL_CFG_CMMBINED_FIFO( );

    cmd[0] = START_RX;
    cmd[1] = channel;
    cmd[2] = condition;
    cmd[3] = rx_len>>8;
    cmd[4] = rx_len;
    cmd[5] = n_state1;
    cmd[6] = n_state2;
    cmd[7] = n_state3;
    SI446X_CMD( cmd, 8 );
}
/*
=================================================================================
SI446X_RX_FIFO_RESET( );
Function : reset the RX FIFO of the device
INTPUT   : None
OUTPUT   : NONE
=================================================================================
*/
void SI446X_RX_FIFO_RESET( void )
{
    INT8U cmd[2];

    cmd[0] = FIFO_INFO;
    cmd[1] = 0x02;
    SI446X_CMD( cmd, 2 );
}
/*
=================================================================================
SI446X_TX_FIFO_RESET( );
Function : reset the TX FIFO of the device
INTPUT   : None
OUTPUT   : NONE
=================================================================================
*/
void SI446X_TX_FIFO_RESET( void )
{
    INT8U cmd[2];

    cmd[0] = FIFO_INFO;
    cmd[1] = 0x01;
    SI446X_CMD( cmd, 2 );
}


/*
=================================================================================
SI446X_TXRX_FIFO_RESET( );
Function : reset the TX AND RX FIFO of the device
INTPUT   : None
OUTPUT   : NONE
=================================================================================
*/
void SI446X_TXRX_FIFO_RESET( void )
{
    INT8U cmd[2];

    cmd[0] = FIFO_INFO;
    cmd[1] = 0x03;
    SI446X_CMD( cmd, 2 );
}



/*
=================================================================================
SI446X_PKT_INFO( );
Function : read packet information
INTPUT   : buffer, stores the read information
           FIELD, feild mask
           length, the packet length
           diff_len, diffrence packet length
OUTPUT   : NONE
=================================================================================
*/
void SI446X_PKT_INFO( INT8U *buffer, INT8U FIELD, INT16U length, INT16U diff_len )
{
    INT8U cmd[6];
    cmd[0] = PACKET_INFO;
    cmd[1] = FIELD;
    cmd[2] = length >> 8;
    cmd[3] = length;
    cmd[4] = diff_len >> 8;
    cmd[5] = diff_len;

    SI446X_CMD( cmd, 6 );
    SI446X_READ_RESPONSE( buffer, 3 );
}






/*
=================================================================================
SI446X_PKT_INFO_LEN( );
Function : read packet information
INTPUT   : buffer, stores the read information
           FIELD, feild mask
           length, the packet length
           diff_len, diffrence packet length
OUTPUT   : NONE
=================================================================================
*/
INT8U SI446X_PKT_INFO_LEN(INT8U FIELD, INT16U length, INT16U diff_len )
{
    INT8U cmd[6];
    cmd[0] = PACKET_INFO;
    cmd[1] = FIELD;
    cmd[2] = length >> 8;
    cmd[3] = length;
    cmd[4] = diff_len >> 8;
    cmd[5] = diff_len;

    SI446X_CMD( cmd, 6 );
    SI446X_READ_RESPONSE( cmd, 3 );
    return cmd[2];
}




/*
=================================================================================
SI446X_FIFO_INFO( );
Function : read fifo information
INTPUT   : buffer, stores the read information
OUTPUT   : NONE
=================================================================================
*/
void SI446X_FIFO_INFO( INT8U *buffer )
{
    INT8U cmd[2];
    cmd[0] = FIFO_INFO;
    cmd[1] = 0x03;

    SI446X_CMD( cmd, 2 );
    SI446X_READ_RESPONSE( buffer, 3);
}


/*
=================================================================================
SI446X_GET_FIFO_INFO_RX_BYTES( );
Function : read fifo information
INTPUT   : none
OUTPUT   : rx fifo bytes
=================================================================================
*/
INT8U SI446X_GET_FIFO_INFO_RX_BYTES(void)
{
    INT8U cmd[2];
    INT8U ubaBuf[3] = {0x00};
    cmd[0] = FIFO_INFO;
    cmd[1] = 0x00;

    SI446X_CMD( cmd, 2 );
    SI446X_READ_RESPONSE( ubaBuf, 3);
    return ubaBuf[1];
}


/*
=================================================================================
SI446X_GPIO_CONFIG( );
Function : config the GPIOs, IRQ, SDO
INTPUT   :
OUTPUT   : NONE
=================================================================================
*/
void SI446X_GPIO_CONFIG( INT8U G0, INT8U G1, INT8U G2, INT8U G3,
                         INT8U IRQ, INT8U SDO, INT8U GEN_CONFIG )
{
    INT8U cmd[10];
    cmd[0] = GPIO_PIN_CFG;
    cmd[1] = G0;
    cmd[2] = G1;
    cmd[3] = G2;
    cmd[4] = G3;
    cmd[5] = IRQ;
    cmd[6] = SDO;
    cmd[7] = GEN_CONFIG;
    SI446X_CMD( cmd, 8 );
    SI446X_READ_RESPONSE( cmd, 8 );
}





/*
* \breif config global_config for reconfig fifo tx and rx  two FIFOs are combined 
*		 into a single 129-byte shared FIFO
* note user add                    
*/

void SI446X_GLOGAL_CFG_CMMBINED_FIFO(void)
{
	#if 0
	u_char ubacmd[5] = {0x00};
	ubacmd[0] = SI446X_CMD_ID_SET_PROPERTY;	//set cmd
    ubacmd[1] = 0x00;							//group id
    ubacmd[2] = 0x01;							//config data length
    ubacmd[3] = 0x03;							//index
	ubacmd[4] = 0x30;							//config data FIFOMODE set 1
	//radio_comm_SendCmd(0x05, Pro2Cmd);
	#endif
	SI446X_SET_PROPERTY_1(GLOBAL_CONFIG, 0x30);
}


/*! This function sends the PART_INFO command to the radio and receives the answer
 *  into @Si446xCmd union.
 */
void SI446X_GET_PART_INFO(void)
{
  u_char ubacmd[16] = {0};
	u_char *pcmd = (u_char *)&ubacmd[1];
	
	SI446X_PART_INFO(ubacmd);
	
    Si446xCmd.PART_INFO.CHIPREV         = pcmd[0];
    Si446xCmd.PART_INFO.PART            = ((U16)pcmd[1] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.PART           |= (U16)pcmd[2] & 0x00FF;
    Si446xCmd.PART_INFO.PBUILD          = pcmd[3];
    Si446xCmd.PART_INFO.ID              = ((U16)pcmd[4] << 8) & 0xFF00;
    Si446xCmd.PART_INFO.ID             |= (U16)pcmd[5] & 0x00FF;
    Si446xCmd.PART_INFO.CUSTOMER        = pcmd[6];
    Si446xCmd.PART_INFO.ROMID           = pcmd[7];
}

/* Full driver support functions */

/*!
 * Sends the FUNC_INFO command to the radio, then reads the resonse into @Si446xCmd union.
 */
void SI446X_GET_FUNC_INFO(void)
{
	u_char ubacmd[16] = {0x00};
	u_char *pcmd = (u_char *)&ubacmd[1];

	SI446X_FUNC_INFO(ubacmd);
	
    Si446xCmd.FUNC_INFO.REVEXT          = pcmd[0];
    Si446xCmd.FUNC_INFO.REVBRANCH       = pcmd[1];
    Si446xCmd.FUNC_INFO.REVINT          = pcmd[2];
    Si446xCmd.FUNC_INFO.FUNC            = pcmd[5];
}

/*!
 * Get the Interrupt status/pending flags form the radio and clear flags if requested.
 *
 * @param PH_CLR_PEND     Packet Handler pending flags clear.
 * @param MODEM_CLR_PEND  Modem Status pending flags clear.
 * @param CHIP_CLR_PEND   Chip State pending flags clear.
 */
void SI446X_GET_INT_STATUS(void)
{
	u_char ubacmd[16] = {0x00};
	u_char *pcmd = (u_char *)&ubacmd[1];

	SI446X_INT_STATUS(ubacmd);
	
    Si446xCmd.GET_INT_STATUS.INT_PEND       = pcmd[0];
    Si446xCmd.GET_INT_STATUS.INT_STATUS     = pcmd[1];
    Si446xCmd.GET_INT_STATUS.PH_PEND        = pcmd[2];
    Si446xCmd.GET_INT_STATUS.PH_STATUS      = pcmd[3];
    Si446xCmd.GET_INT_STATUS.MODEM_PEND     = pcmd[4];
    Si446xCmd.GET_INT_STATUS.MODEM_STATUS   = pcmd[5];
    Si446xCmd.GET_INT_STATUS.CHIP_PEND      = pcmd[6];
    Si446xCmd.GET_INT_STATUS.CHIP_STATUS    = pcmd[7];
}



/*!
 * Requests the current state of the device and lists pending TX and RX requests
 */
void SI446X_REQUEST_DEVICE_STATE(void)//si446x_request_device_state(void)
{
	#if 0
    Pro2Cmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;
    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd );
    #endif
                              
    u_char ubacmd[16] = {0x00};
	u_char *pcmd = (u_char *)&ubacmd[1];
	
    ubacmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;

    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE+1);

    Si446xCmd.REQUEST_DEVICE_STATE.CURR_STATE       = pcmd[0];
    Si446xCmd.REQUEST_DEVICE_STATE.CURRENT_CHANNEL  = pcmd[1];
}


/*!
 * Requests the current state of the device and lists pending TX and RX requests
 */
void SI446X_REQUEST_DEVICE_STATE_CH(u_char *pState, u_char *pChannel)//si446x_request_device_state(void)
{
	#if 0
    Pro2Cmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;
    radio_comm_SendCmdGetResp( SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd,
                              SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE,
                              Pro2Cmd );
    #endif
                              
    u_char ubacmd[16] = {0x00};
	u_char *pcmd = (u_char *)&ubacmd[1];
	
    ubacmd[0] = SI446X_CMD_ID_REQUEST_DEVICE_STATE;

    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_REQUEST_DEVICE_STATE);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_REQUEST_DEVICE_STATE+1);

    *pState = pcmd[0]&0x0f;
    *pChannel = pcmd[1];
}



/*!
 * Gets the Modem status flags. Optionally clears them.
 *
 * @param MODEM_CLR_PEND Flags to clear.
 */
void SI446X_GET_MODEM_STATUS( U8 MODEM_CLR_PEND )
{

    u_char ubacmd[16] = {0x00};
	u_char *pcmd = (u_char *)&ubacmd[1];

    ubacmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    ubacmd[1] = MODEM_CLR_PEND;
    
    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_GET_MODEM_STATUS);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS+1);


    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = pcmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = pcmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = pcmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = pcmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = pcmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = pcmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET =  ((U16)pcmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (U16)pcmd[7] & 0x00FF;
}






/*!
 * GET current rssi
 *
 * @param MODEM_CLR_PEND Flags to clear.
 */
u_char SI446X_GET_CUR_RSSI(void)
{

    u_char ubacmd[16] = {0x00};
	u_char *pcmd = (u_char *)&ubacmd[1];

    ubacmd[0] = SI446X_CMD_ID_GET_MODEM_STATUS;
    ubacmd[1] = 0x7f;
    
    SI446X_CMD(ubacmd, SI446X_CMD_ARG_COUNT_GET_MODEM_STATUS);
    SI446X_READ_RESPONSE(ubacmd, SI446X_CMD_REPLY_COUNT_GET_MODEM_STATUS+1);

	/*
    Si446xCmd.GET_MODEM_STATUS.MODEM_PEND   = pcmd[0];
    Si446xCmd.GET_MODEM_STATUS.MODEM_STATUS = pcmd[1];
    Si446xCmd.GET_MODEM_STATUS.CURR_RSSI    = pcmd[2];
    Si446xCmd.GET_MODEM_STATUS.LATCH_RSSI   = pcmd[3];
    Si446xCmd.GET_MODEM_STATUS.ANT1_RSSI    = pcmd[4];
    Si446xCmd.GET_MODEM_STATUS.ANT2_RSSI    = pcmd[5];
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET =  ((U16)pcmd[6] << 8) & 0xFF00;
    Si446xCmd.GET_MODEM_STATUS.AFC_FREQ_OFFSET |= (U16)pcmd[7] & 0x00FF;
    */
    return pcmd[2];
}


