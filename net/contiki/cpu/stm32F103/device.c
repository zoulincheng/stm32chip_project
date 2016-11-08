#include <string.h>

//#include "contiki.h"
#include "basictype.h"

#include "list.h"
//#include "vfile.h"
//#include "device.h"
//#include "userdev.h"



#if 1

//DEV  LIST

LIST(gContikiDevList);// device list 

/*---------------------------------------------------------------------------*/
void ContikiDeviceInit(void)
{
  list_init(gContikiDevList);
}


/*!
 * \brief Find device entry by name.
 *
 * \param  name Unique device name.
 *
 * \return Pointer to the ::DEVICE structure.
 */
CONTIKIDEVICE *ContikiDeviceLookup(const char *name)
{
    CONTIKIDEVICE *dev;
    for(dev = list_head(gContikiDevList); dev != NULL; dev = list_item_next(dev))
        if(strcmp((char*)dev->dev_name, name) == 0)
             return dev;

	return NULL;
}





/*!
 * \brief Register a device.
 *
 * Initializes the device and adds it to the system device list.
 * Applications should call this function during initialization
 * for each device they intend to use. Once registered, stream
 * devices must be accessed by calling NutDeviceOpen() first.
 * However, network devices should not be opened but configured
 * by calling NutNetIfConfig() or NutNetAutoConfig().
 *
 * If called for a device, which had been registered previously,
 * this function will update the base address and interrupt
 * number. However, most device drivers do not expect these
 * itemes being changed and may crash.
 *
 * \note Virtual stream devices don't need to be registered.
 *
 * \param dev  Pointer to the ::NUTDEVICE structure, which is
 *             provided by the device driver. Should point to
 *              - devUart0 for the first on-chip UART.
 *              - devUart1 for the second on-chip UART.
 *              - devUarts[0-7] for one of the 8 SPI UARTs.
 *              - devEth0 for the Ethernet device.
 * \param base Hardware base address of this device. Set to 0,
 *             if the device driver has a fixed hardware address.
 * \param irq  Hardware interrupt used by this device. Set to 0,
 *             if the device driver doesn't support configurable
 *             interupts.
 * \return 0 if the device has been registered for the first time.
 *         The function returns -1 if the device had been registered
 *         previously or the ::NUTDEVICE structure is invalid.
 */
int ContikiRegisterDevice(CONTIKIDEVICE * dev, uintptr_t base, uint8_t irq)
{
    int rc = -1;

    //NUTASSERT(dev != NULL);
    if (NULL == dev)
    	return rc;
    	
    if (base)
        dev->dev_base = base;
    if (irq)
        dev->dev_irq = irq;

	list_add(gContikiDevList, dev);	

	rc = 0;
    return rc;
}


/*---------------------------------------------------------------------------*/
void ContikiUnRegister(CONTIKIDEVICE * dev)
{
	list_remove(gContikiDevList, dev);
}

#endif

