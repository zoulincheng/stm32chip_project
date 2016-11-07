/**
  ******************************************************************************
  * @file    main.c
  * @author  zjj
  * @version V1.0
  * @date    2014-10-22
  * @brief   串口中断接收测试
  ******************************************************************************
  * @attention
  *
  * 实验平台: STM32 开发板
  * 论坛    :
  * 淘宝    :
  *
  ******************************************************************************
  */ 
 
#include "sys_config.h"
#include "led.h"

#define BUFFER_SIZE                                 15 // Define the payload size here

static uint16_t BufferSize = BUFFER_SIZE;			// RF buffer size
static uint8_t Buffer[BUFFER_SIZE];					// RF buffer

static uint8_t EnableMaster = true; 				// Master/Slave selection

tRadioDriver *Radio = NULL;

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

/*
 * Manages the master operation
 */
void OnMaster( void )
{
    uint8_t i;
    
    switch( Radio->Process( ) )
    {
    case RF_RX_TIMEOUT:
        // Send the next PING frame
        Buffer[0] = 'P';
        Buffer[1] = 'I';
        Buffer[2] = 'N';
        Buffer[3] = 'G';
        for( i = 4; i < BufferSize; i++ )
        {
            Buffer[i] = i - 4;
        }
        Radio->SetTxPacket( Buffer, BufferSize );
        break;
    case RF_RX_DONE:
        Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize );
    
        if( BufferSize > 0 )
        {
            if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
            {
                // Indicates on a LED that the received frame is a PONG
                LED0 = !LED0;//LedToggle( LED_GREEN );
    
                // Send the next PING frame            
                Buffer[0] = 'P';
                Buffer[1] = 'I';
                Buffer[2] = 'N';
                Buffer[3] = 'G';
                // We fill the buffer with numbers for the payload 
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                Radio->SetTxPacket( Buffer, BufferSize );
            }
            else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
            { // A master already exists then become a slave
                EnableMaster = false;
                LED1 = 1;//LedOff( LED_RED );
            }
        }            
        break;
    case RF_TX_DONE:
        // Indicates on a LED that we have sent a PING
        LED1 = !LED1;//LedToggle( LED_RED );
        Radio->StartRx( );
        break;
    default:
        break;
    }
}

/*
 * Manages the slave operation
 */
void OnSlave( void )
{
    uint8_t i;

    switch( Radio->Process( ) )
    {
    case RF_RX_DONE:
        Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize );
    
        if( BufferSize > 0 )
        {
            if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
            {
                // Indicates on a LED that the received frame is a PING
                LED0 = !LED0;//LedToggle( LED_GREEN );

               // Send the reply to the PONG string
                Buffer[0] = 'P';
                Buffer[1] = 'O';
                Buffer[2] = 'N';
                Buffer[3] = 'G';
                // We fill the buffer with numbers for the payload 
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }

                Radio->SetTxPacket( Buffer, BufferSize );
            }
        }
        break;
    case RF_TX_DONE:
        // Indicates on a LED that we have sent a PONG
        LED1 = !LED1;//LedToggle( LED_RED );
        Radio->StartRx( );
        break;
    default:
        break;
    }
}

int main(void)
 {        
	//stm32 config
	sys_Configuration();
	
	BoardInit( );
        
    LED_Init();
    Radio = RadioDriverInit( );
    
    Radio->Init( );

    Radio->StartRx( );
    
    while( 1 )
    {
        if( EnableMaster == true )
        {
            OnMaster( );
        }
        else
        {
            OnSlave( );
        }    
    }
	return 0;
}



