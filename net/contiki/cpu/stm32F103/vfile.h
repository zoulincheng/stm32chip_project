#ifndef __VFILE_H_
#define __VFILE_H_



#define fopen(...)   CONTIKI_fopen(__VA_ARGS__)
#define fwrite(...)   CONTIKI_fwrite(__VA_ARGS__)
#define fioctl(...)   CONTIKI_fioctl(__VA_ARGS__)
#define fread(...)   CONTIKI_fread(__VA_ARGS__)


#define FILE CONTIKI_FILE




#if 1
typedef struct _tgFILE FILE;

struct _tgFILE{
    int     iob_fd;		/*!< \internal Associated file, device or socket descriptor. */
    u_short iob_mode;		/*!< \internal Access mode, see fcntl.h. */
    u_char  iob_flags;		/*!< \internal Status flags. */
    int     iob_unget;		/*!< \internal Unget buffer. */
};

#endif


#if 1
/*@{*/

/*! \brief UART _ioctl() command code to set the line speed.
 *
 * The configuration parameter specifies the input and output bit rate 
 * per second.
 */
#define UART_SETSPEED           0x0101

/*! \brief UART _ioctl() command code to query the line speed.
 *
 * The configuration parameter is set to the input and output bit rate 
 * per second.
 */
#define UART_GETSPEED           0x0102

/*! \brief UART _ioctl() command code to set the number of data bits.
 *
 * The configuration parameter specifies the number of data bits, 5, 6, 
 * 7, 8 or 9.
 */
#define UART_SETDATABITS        0x0103

/*! \brief UART _ioctl() command code to query the number of data bits.
 *
 * The configuration parameter is set to the number of data bits, 5, 6, 
 * 7, 8 or 9.
 */
#define UART_GETDATABITS        0x0104

/*! \brief UART _ioctl() command code to set the parity mode.
 *
 * The configuration parameter specifies the type of the parity bit, 
 * 0 (none), 1 (odd) or 2 (even).
 */
#define UART_SETPARITY          0x0105

/*! \brief UART _ioctl() command code to query the parity mode.
 *
 * The configuration parameter is set to the type of the parity bit, 
 * 0 (none), 1 (odd) or 2 (even).
 */
#define UART_GETPARITY          0x0106

/*! \brief UART _ioctl() command code to set the number of stop bits.
 *
 * The configuration parameter specifies the number of stop bits, 1 or 2.
 */
#define UART_SETSTOPBITS        0x0107

/*! \brief UART _ioctl() command code to query the number of stop bits.
 *
 * The configuration parameter is set to the number of stop bits, 1 or 2.
 */
#define UART_GETSTOPBITS        0x0108

/*! \brief UART _ioctl() command code to set the status.
 *
 * The configuration parameter specifies the status to set.
 */
#define UART_SETSTATUS          0x0109

/*! \brief UART _ioctl() command code to query the status.
 *
 * The configuration parameter is set to the current status.
 */
#define UART_GETSTATUS          0x010a

/*! \brief UART _ioctl() command code to set the read timeout.
 *
 * The configuration parameter specifies the read timeout in 
 * milliseconds.
 */
#define UART_SETREADTIMEOUT     0x010b

/*! \brief UART _ioctl() command code to query the read timeout.
 *
 * The configuration parameter is set to the read timeout in 
 * milliseconds.
 */
#define UART_GETREADTIMEOUT     0x010c

/*! \brief UART _ioctl() command code to set the write timeout.
 *
 * The configuration parameter specifies the write timeout in 
 * milliseconds.
 */
#define UART_SETWRITETIMEOUT    0x010d

/*! \brief UART _ioctl() command code to query the write timeout.
 *
 * The configuration parameter is set to the write timeout in 
 * milliseconds.
 */
#define UART_GETWRITETIMEOUT    0x010e

/*! \brief UART _ioctl() command code to set the local echo mode.
 *
 * The configuration parameter specifies the local echo mode, 
 * 0 (off) or 1 (on).
 */
#define UART_SETLOCALECHO       0x010f

/*! \brief UART _ioctl() command code to query the local echo mode.
 *
 * The configuration parameter is set to the local echo mode, 
 * 0 (off) or 1 (on).
 */
#define UART_GETLOCALECHO       0x0110

/*! \brief UART _ioctl() command code to set the flow control mode.
 *
 * The configuration parameter specifies the flow control mode.
 */
#define UART_SETFLOWCONTROL     0x0111

/*! \brief UART _ioctl() command code to query the flow control mode.
 *
 * The configuration parameter is set to the flow control mode.
 */
#define UART_GETFLOWCONTROL     0x0112

/*! \brief UART _ioctl() command code to set the cooking mode.
 *
 * The configuration parameter specifies the character cooking mode, 
 * 0 (raw) or 1 (EOL translation).
 */
#define UART_SETCOOKEDMODE      0x0113

/*! \brief UART _ioctl() command code to query the cooking mode.
 *
 * The configuration parameter is set to the character cooking mode, 
 * 0 (raw) or 1 (EOL translation).
 */
#define UART_GETCOOKEDMODE      0x0114

/*! \brief UART _ioctl() command code to set the buffering mode.
 *
 * The configuration parameter specifies the buffering mode.
 */
#define UART_SETBUFFERMODE      0x0115

/*! \brief UART _ioctl() command code to query the buffering mode.
 *
 * The configuration parameter is set to the buffering mode.
 */
#define UART_GETBUFFERMODE      0x0116

/*! \brief UART _ioctl() command code to set the network interface mode.
 *
 * The configuration parameter specifies the network interface mode.
 */
#define HDLC_SETIFNET           0x0117

/*! \brief UART _ioctl() command code to query the network interface mode.
 *
 * The configuration parameter is set to the network interface mode.
 */
#define HDLC_GETIFNET           0x0118

/*! \brief UART _ioctl() command code to set the clock mode.
 *
 * The configuration parameter specifies the clock mode.
 */
#define UART_SETCLOCKMODE       0x0119

/*! \brief UART _ioctl() command code to query the clock mode.
 *
 * The configuration parameter is set to the clock mode.
 */
#define UART_GETCLOCKMODE       0x011a

/*! \brief UART _ioctl() command code to set the transmit buffer size.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_SETTXBUFSIZ        0x011b

/*! \brief UART _ioctl() command code to query the transmit buffer size.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_GETTXBUFSIZ        0x011c

/*! \brief UART _ioctl() command code to set the receive buffer size.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_SETRXBUFSIZ        0x011d

/*! \brief UART _ioctl() command code to query the receive buffer size.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_GETRXBUFSIZ        0x011e

/*! \brief UART _ioctl() command code to set the transmit buffer low watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_SETTXBUFLWMARK     0x0120

/*! \brief UART _ioctl() command code to query the transmit buffer low watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_GETTXBUFLWMARK     0x0121

/*! \brief UART _ioctl() command code to set the transmit buffer high watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_SETTXBUFHWMARK     0x0122

/*! \brief UART _ioctl() command code to query the transmit buffer high watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_GETTXBUFHWMARK     0x0123

/*! \brief UART _ioctl() command code to set the receive buffer low watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_SETRXBUFLWMARK     0x0124

/*! \brief UART _ioctl() command code to query the receive buffer low watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_GETRXBUFLWMARK     0x0125

/*! \brief UART _ioctl() command code to set the receive buffer high watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_SETRXBUFHWMARK     0x0126

/*! \brief UART _ioctl() command code to query the receive buffer high watermark.
 *
 * The configuration parameter specifies the number of bytes.
 */
#define UART_GETRXBUFHWMARK     0x0127

/*! \brief UART _ioctl() command code to set the block read mode
*
* The configuration parameter specifies the block read mode.
*/
#define UART_SETBLOCKREAD       0x0128

/*! \brief UART _ioctl() command code to query the receive buffer high watermark.
*
* The configuration parameter specifies the block read mode.
*/
#define UART_GETBLOCKREAD       0x0129

/*! \brief UART _ioctl() command code to set physical device to the raw mode.
 *
 * The configuration parameter specifies the raw mode for device. In raw mode
 * data encapsulation is not allowed to be done. This allows other processing to
 * be done on physical device.
 */
#define UART_SETRAWMODE         0x012a

/*! \brief UART _ioctl() command code to query the raw mode.
 *
 * The configuration parameter specified the raw mode.
 */
#define UART_GETRAWMODE         0x012b

/*! \brief AHDLC _ioctl() command code to set the ACCM (Control Character Mask).
 *
 * During the LCP stage of PPP negotiation both peers inform each other which
 * of the control characters (0-31) will not require escaping when being
 * transmitted.  This allows the PPP layer to tell the HDLC layer about this
 * so that data may be transmitted quicker (no escapes).
 */
#define HDLC_SETTXACCM          0x012c

/*! \brief AHDLC _ioctl() command code to get the ACCM (Control Character Mask).
 *
 * Just in case someone ever wants to see what it currently is.
 */
#define HDLC_GETTXACCM          0x012d

/*! \brief UART _ioctl() command code to set physical device to half duplex mode.
 *
 * The configuration parameter specifies the halfduplex mode for device. In raw mode
 */
#define UART_SETHDPXMODE        0x012c

/*! \brief UART _ioctl() command code to query the halfduplex mode.
 *
 */
#define UART_GETHDPXMODE        0x012d

/*! \brief UART _ioctl() command code to set irDA mode.
 *
 */
#define UART_SET_IRDA_MODE       0x012e

/*! \brief UART _ioctl() command code to set read filter procedure.
 *
 */
#define UART_SET_READ_FILTER       0x012f

/*! \brief UART _ioctl() command code to set write filter procedure.
 *
 */
#define UART_SET_WRITE_FILTER       0x0130

/*! \brief UART _ioctl() command code to set data receive interval time out.
 *
 */
#define UART_SET_INTERVAL_TIME_OUT       0x0131

/*! \brief UART _ioctl() command code to set data receive interval time out.
 *
 */
#define UART_GET_INTERVAL_TIME_OUT       0x0132

/*! \brief UART _ioctl() command code to set data receive interval time out.
 *
 */
#define UART_SETRECEIVEMODE       0x0133

/*! \brief UART _ioctl() command code to set write procedure.
 *
 */
#define UART_SET_WRITE_PROC       0x0134

/*! \brief UART _ioctl() command code to set read procedure.
 *
 */
#define UART_SET_READ_PROC       0x0135

/*! \brief UART _ioctl() command code to set read line procedure.
 *  \param 1, read line; 0 default raw read
 */
#define UART_SET_READ_LINE       0x0136


/*
* \brief Open or close the interrupt of the RX FLAG
*/
#define UART_SET_INT_ENABLE       0x139

#define UART_SET_CONFIG                   0x13a
#define UART_GET_CONFIG                   0x13b

#define UART_SET_LOCK             0x13c


/*!
 * \addtogroup xgUARTStatus
 * \brief UART device status flags,
 *
 * A combination of these status flags is used by the _ioctl() commands
 * \ref UART_SETSTATUS and \ref UART_GETSTATUS. 
 */
/*@{*/

/*! \brief Framing error.
 *
 * \ref UART_SETSTATUS will clear this error.
 */
#define UART_FRAMINGERROR   0x00000001UL

/*! \brief Overrun error. 
 *
 * \ref UART_SETSTATUS will clear this error.
 */
#define UART_OVERRUNERROR   0x00000002UL

/*! \brief Parity error. 
 *
 * \ref UART_SETSTATUS will clear this error.
 */
#define UART_PARITYERROR    0x00000004UL

/*! \brief UART errors.
 *
 * \ref UART_SETSTATUS will clear all errors.
 */
#define UART_ERRORS         (UART_FRAMINGERROR | UART_OVERRUNERROR | UART_PARITYERROR)

/*! \brief Receiver buffer empty. 
 */
#define UART_RXBUFFEREMPTY  0x00000040UL

/*! \brief Transmitter buffer empty.
 *
 * \ref UART_SETSTATUS will immediately clear the buffer. It will not 
 * wait until the remaining characters have been transmitted.
 */
#define UART_TXBUFFEREMPTY  0x00000080UL

/*! \brief RTS handshake output enabled. 
 */
#define UART_RTSENABLED     0x00000100UL

/*! \brief RTS handshake output disabled. 
 */
#define UART_RTSDISABLED    0x00000200UL

/*! \brief CTS handshake input enabled. 
 */
#define UART_CTSENABLED     0x00000400UL

/*! \brief CTS handshake input disabled. 
 */
#define UART_CTSDISABLED    0x00000800UL

/*! \brief DTR handshake output enabled. 
 */
#define UART_DTRENABLED     0x00001000UL

/*! \brief DTR handshake output disabled. 
 */
#define UART_DTRDISABLED    0x00002000UL

/*! \brief Receiver enabled. 
 */
#define UART_RXENABLED      0x00010000UL

/*! \brief Receiver enabled. 
 */
#define UART_RXDISABLED     0x00020000UL

/*! \brief Transmitter enabled. 
 */
#define UART_TXENABLED      0x00040000UL

/*! \brief Transmitter enabled. 
 */
#define UART_TXDISABLED     0x00080000UL

/*! \brief Receive address frames only.
 *
 * Used in multidrop communication. May only work if 9 databits have 
 * been configured.
 */
#define UART_RXADDRFRAME    0x00100000UL

/*! \brief Receive all frames.
 *
 * Used in multidrop communication.
 */
#define UART_RXNORMFRAME    0x00200000UL

/*! \brief Transmit as address frame.
 *
 * Used in multidrop communication. May only work if 9 databits have 
 * been configured.
 */
#define UART_TXADDRFRAME    0x00400000UL

/*! \brief Transmit as normal frame.
 *
 * Used in multidrop communication.
 */
#define UART_TXNORMFRAME    0x00800000UL


/*@}*/

/*!
 * \addtogroup xgUARTHS
 * \brief UART handshake modes.
 *
 * Any of these values may be used by the _ioctl() commands
 * \ref UART_SETFLOWCONTROL and \ref UART_GETFLOWCONTROL.
 */
/*@{*/

/*! \brief RTS / CTS hardware handshake.
 *
 * Nut/OS uses DTE definitions, where RTS is output and CTS is input.
 */
#define UART_HS_RTSCTS      0x0003

/*! \brief Full modem hardware handshake.
 *
 * Not supported yet by the standard drivers.
 */
#define UART_HS_MODEM       0x001F

/*! \brief XON / XOFF software handshake.
 *
 * It is recommended to set a proper read timeout with software handshake.
 * In this case a timeout may occur, if the communication peer lost our 
 * last XON character. The application may then use ioctl() to disable the 
 * receiver and do the read again. This will send out another XON.
 */
#define UART_HS_SOFT        0x0020

/*! \brief Half duplex mode.
 */
#define UART_HS_HALFDUPLEX  0x0400

/*@}*/


/*!
 * \addtogroup xgUARTClock
 * \brief UART device clock modes.
 *
 * Any of these values may be used by the _ioctl() commands
 * \ref UART_SETCLOCKMODE and \ref UART_GETCLOCKMODE. Most drivers
 * require to set the bit rate after modifying the clock mode. In order
 * to avoid unknown clock output frequencies in master mode, set the
 * clock mode to \ref UART_SYNCSLAVE first, than use \ref UART_SETSPEED 
 * to select the bit rate and finally switch to \ref UART_SYNCMASTER or 
 * \ref UART_NSYNCMASTER.
 */
/*@{*/

#define UART_SYNC           0x01
#define UART_MASTER         0x02
#define UART_NCLOCK         0x04
#define UART_HIGHSPEED      0x20

/*! \brief Normal asynchronous mode.
 */
#define UART_ASYNC          0x00

/*! \brief Synchronous slave mode.
 *
 * Transmit data changes on rising edge and receive data is sampled on 
 * the falling edge of the clock input.
 */
#define UART_SYNCSLAVE     UART_SYNC

/*! \brief Synchronous master mode.
 *
 * Transmit data changes on rising edge and receive data is sampled on 
 * the falling edge of the clock output.
 */
#define UART_SYNCMASTER    (UART_SYNC | UART_MASTER)

/*! \brief Synchronous slave mode, clock negated.
 *
 * Similar to \ref UART_SYNCSLAVE, but transmit data changes on falling 
 * edge and receive data is sampled on the rising edge of the clock input.
 */
#define UART_NSYNCSLAVE    (UART_SYNC | UART_NCLOCK)

/*! \brief Synchronous master mode, clock negated
 *
 * Similar to \ref UART_SYNCMASTER, but transmit data changes on falling 
 * edge and receive data is sampled on the rising edge of the clock output.
 */
#define UART_NSYNCMASTER   (UART_SYNC | UART_NCLOCK | UART_MASTER)

/*! \brief Asynchronous high speed mode.
 *
 * More deviation sensitive than normal mode, but supports higher speed.
 */
#define UART_ASYNC_HS      UART_HIGHSPEED



//add 
#define IFTYP_RAM       0   /*!< \brief RAM device */
#define IFTYP_ROM       1   /*!< \brief ROM device */
#define IFTYP_STREAM    2   /*!< \brief Stream device */
#define IFTYP_NET       3   /*!< \brief Net device */
#define IFTYP_TCPSOCK	4	/*!< \brief TCP socket */

#define IFTYP_CHAR      5	/*!< \brief Character stream device */
#define IFTYP_CAN       6       /*!< \brief CAN device */
#define IFTYP_BLKIO     7   /*!< \brief Block I/O device */
#define IFTYP_FS       16   /*!< \brief file system device */



#define	_O_RDONLY	0x0000	/*!< Read only. */
#define _O_WRONLY	0x0001	/*!< Write only. */
#define _O_RDWR		0x0002	/*!< Read and write. */
#define _O_APPEND   0x0008  /*!< Start writing at the end. */
#define	_O_CREAT	0x0100	/*!< Create file if it does not exist. */
#define	_O_TRUNC	0x0200	/*!< Truncate file if it exists. */
#define	_O_EXCL		0x0400	/*!< Open only if it does not exist. */
#define	_O_TEXT		0x4000	/*!< EOL translation. */
#define	_O_BINARY	0x8000	/*!< Raw mode. */

#define	EINVAL		22		/* Invalid argument */
#define	ENOENT		2		/* No such file or directory */


#define _IOUNG	    0x08    /*!< \internal Unget buffer filled. */
#define _IOERR	    0x10    /*!< \internal Error occured. */
#define _IOEOF      0x20    /*!< \internal End of file reached. */
#define _IOPGM	    0x40    /*!< \internal Input from program memory. */
#define _IOTMO	    0x80    /*!< \IO Time out. */

#endif

#if 1
#ifndef EOF
#define EOF (-1)
#endif
#endif



int fioctl(FILE*fp,int cmd,void*data);
FILE *fopen(const char *name, const char *mode);
int  fwrite(const void *data, int  size, int  count,FILE * stream);
int  fread(void *buffer, int  size, int  count,FILE * stream );

#endif


//#endif

