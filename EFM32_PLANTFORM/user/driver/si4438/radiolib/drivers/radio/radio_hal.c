/*!
 * File:
 *  radio_hal.c
 *
 * Description:
 *  This file contains RADIO HAL.
 *
 * Silicon Laboratories Confidential
 * Copyright 2011 Silicon Laboratories, Inc.
 */

                /* ======================================= *
                 *              I N C L U D E              *
                 * ======================================= */

#include "bsp.h"

#include "em32lg_platform_io.h"
#include "em32lg_config.h"
#include "arch_spi.h"

                /* ======================================= *
                 *          D E F I N I T I O N S          *
                 * ======================================= */

                /* ======================================= *
                 *     G L O B A L   V A R I A B L E S     *
                 * ======================================= */

                /* ======================================= *
                 *      L O C A L   F U N C T I O N S      *
                 * ======================================= */

                /* ======================================= *
                 *     P U B L I C   F U N C T I O N S     *
                 * ======================================= */





/*
This file need to implements by ourself
*/
void radio_hal_AssertShutdown(void)
{
	//GPIO_PinOutSet(RF_SDN_PORT, RF_SDN_PIN);
	RF_SDN(1);
}

void radio_hal_DeassertShutdown(void)
{
	//GPIO_PinOutClear(RF_SDN_PORT, RF_SDN_PIN);
	RF_SDN(0);
}


void radio_hal_ClearNsel(void)
{
	//GPIO_PinOutClear(RF_SPICS_PORT, RF_SPICS_PIN);
	RF_SPINSEL(0);
}

void radio_hal_SetNsel(void)
{
	//GPIO_PinOutSet(RF_SPICS_PORT, RF_SPICS_PIN);
	RF_SPINSEL(1);
}

//BIT radio_hal_NirqLevel(void)
U8 radio_hal_NirqLevel(void)
{
	//return GPIO_PinInGet(RF_nIRQ_PORT, RF_nIRQ_PIN);
  return RF_NIRQS;
}

void radio_hal_SpiWriteByte(U8 byteToWrite)
{
	SPI_SendByteData(byteToWrite);
}

U8 radio_hal_SpiReadByte(void)
{
	return SPI_SendByteData(0xff);
}

void radio_hal_SpiWriteData(U8 byteCount, U8* pData)
{
	SpiWriteDataBurst(byteCount, pData);
}

void radio_hal_SpiReadData(U8 byteCount, U8* pData)
{
	SpiReadDataBurst(byteCount, pData);
}

#ifdef RADIO_DRIVER_EXTENDED_SUPPORT

//BIT radio_hal_Gpio0Level(void)
U8 radio_hal_Gpio0Level(void)
{
	//return GPIO_PinInGet(RF_GPIO0_PORT, RF_GPIO0_PIN);
  return 0;
}

//BIT radio_hal_Gpio1Level(void)
U8 radio_hal_Gpio1Level(void)
{
	//return GPIO_PinInGet(RF_GPIO1_PORT, RF_GPIO1_PIN);
	return 0;
}

//BIT radio_hal_Gpio2Level(void)
U8 radio_hal_Gpio2Level(void)
{
	//return GPIO_PinInGet(RF_GPIO2_PORT, RF_GPIO2_PIN);
  return 0;
}

//BIT radio_hal_Gpio3Level(void)
U8 radio_hal_Gpio3Level(void)
{
	//return GPIO_PinInGet(RF_GPIO3_PORT, RF_GPIO3_PIN);
  return 0;
}

#endif
