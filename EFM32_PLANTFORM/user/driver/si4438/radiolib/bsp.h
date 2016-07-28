/*! @file bsp.h
 * @brief This file contains application specific definitions and includes.
 *
 * @b COPYRIGHT
 * @n Silicon Laboratories Confidential
 * @n Copyright 2012 Silicon Laboratories, Inc.
 * @n http://www.silabs.com
 */

#ifndef BSP_H
#define BSP_H

/*------------------------------------------------------------------------*/
/*            Application specific global definitions                     */
/*------------------------------------------------------------------------*/
/*! Platform definition */
/* Note: Plaform is defined in Silabs IDE project file as
 * a command line flag for the compiler. */
//#define SILABS_PLATFORM_WMB930

/*! Extended driver support 
 * Known issues: Some of the example projects 
 * might not build with some extended drivers 
 * due to data memory overflow */
#define RADIO_DRIVER_EXTENDED_SUPPORT
#define RADIO_DRIVER_FULL_SUPPORT



#ifndef SILABS_RADIO_SI446X
#define SILABS_RADIO_SI446X
#endif
/*------------------------------------------------------------------------*/
/*            Application specific includes                               */
/*------------------------------------------------------------------------*/

#include "basictype.h"
#include "application\radio_config.h"
#include "application\si446xradio.h"




#include "drivers\radio\radio_hal.h"
#include "drivers\radio\radio_comm.h"

#ifdef SILABS_RADIO_SI446X
#include "drivers\radio\Si446x\si446x_api_lib.h"
#include "drivers\radio\Si446x\si446x_defs.h"
#include "drivers\radio\Si446x\si446x_nirq.h"
//#include "drivers\radio\Si446x\si446x_patch.h"
#endif

#ifdef SILABS_RADIO_SI4455
#include "drivers\radio\Si4455\si4455_api_lib.h"
#include "drivers\radio\Si4455\si4455_defs.h"
#include "drivers\radio\Si4455\si4455_nirq.h"
#endif

#endif //BSP_H
