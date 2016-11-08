#ifndef _DEVICE_H_
#define _DEVICE_H_


//#include "vfile.h"

#if 1


#if 0
typedef struct _tgFILE FILE;

struct _tgFILE{
    int     iob_fd;		/*!< \internal Associated file, device or socket descriptor. */
    u_short iob_mode;		/*!< \internal Access mode, see fcntl.h. */
    u_char  iob_flags;		/*!< \internal Status flags. */
    int     iob_unget;		/*!< \internal Unget buffer. */
};

#endif
/*!
 * \brief Device structure type.
 */
typedef struct _CONTIKIDEVICE CONTIKIDEVICE;


typedef struct _CONTIKIFILE CONTIKIFILE;


struct _CONTIKIFILE{
    /*! 
     * \brief Link to the next file structure. 
     */
    CONTIKIFILE *nf_next;

    /*! 
     * \brief Device containing this file.
     */
    CONTIKIDEVICE *nf_dev;

    /*! 
     * \brief Device specific file control block.
     */
    void *nf_fcb;

    FILE	devFile;			/*add for _open*/
};

#define CONTIKIFILE_EOF ((CONTIKIFILE *)(-1))






/*!
 * \struct _NUTDEVICE device.h sys/device.h
 * \brief Device structure.
 */
struct _CONTIKIDEVICE {
    CONTIKIDEVICE *dev_next;       /*!< \brief Link to the next device structure. */
    u_char  dev_name[9];    /*!< \brief Unique device name. */
    u_char  dev_type;       /*!< \brief Type of interface. 
                             * May be any of the following:
                             * - IFTYP_RAM
                             * - IFTYP_ROM
                             * - IFTYP_STREAM
                             * - IFTYP_NET
                             */
//u_short dev_base;                             
    u_ptrint dev_base;       /*!< \brief Hardware base address. 
                             *   Will be set by calling NutRegisterDevice(). 
                             *   On some device drivers this address may
                             *   be fixed.
                             */
    u_char  dev_irq;        /*!< \brief Interrupt registration number. 
                             *   Will be set by calling NutRegisterDevice(). 
                             *   On some device drivers the interrupt may
                             *   be fixed.
                             */
    void   *dev_icb;        /*!< \brief Interface control block.
                             *   With stream devices, this points to the
                             *   IFSTREAM structure and with network
                             *   devices this is a pointer to the IFNET
                             *   structure.
                             */
    void   *dev_dcb;        /*!< \brief Driver control block.
                             *   Points to a device specific information block.
                             */
    int   (*dev_init)(CONTIKIDEVICE *);   /*!< \brief Driver initialization routine. 
                                    *   With stream devices this is called during
                                    *   NutDeviceOpen(). For network devices
                                    *   this routine is called within NutNetIfConfig().
                                    */
    int   (*dev_ioctl)(CONTIKIDEVICE *, int, void *);  /*!< \brief Driver control function. 
                                                 *   Used to modify or query device 
                                                 *   specific settings.
                                                 */
    /*! 
     * \brief Read from device. 
     */
    int (*dev_read)(CONTIKIFILE *, void *, int);

    /*! 
     * \brief Write to device. 
     */
    int (*dev_write)(CONTIKIFILE *, const void *, int);

    /*! 
     * \brief Open a device or file. 
     */
    CONTIKIFILE *(*dev_open)(CONTIKIDEVICE *, const char *, int, int);

    /*! 
     * \brief Close a device or file. 
     */
    int (*dev_close)(CONTIKIFILE *);

    /*! 
     * \brief Request file size. 
     */
    long (*dev_size)(CONTIKIFILE *);
};
/*@}*/

#endif

void ContikiDeviceInit(void);

CONTIKIDEVICE *ContikiDeviceLookup(const char *name);

int ContikiRegisterDevice(CONTIKIDEVICE * dev, uintptr_t base, uint8_t irq);

void ContikiUnRegister(CONTIKIDEVICE * dev);

#endif

