#include <string.h>
#include <stdarg.h>

//#include "contiki.h"
#include "basictype.h"



//#include "vfile.h"
//#include "device.h"
#include "userdev.h"


int errno;			/* global error number */
int verrno;



/*! \internal
 * \brief Return the flags for a given mode string.
 *
 * \return Flags or EOF to indicate an error.
 */
int _fmode(const char *mode)
{
    int mflags = _O_TEXT;

    switch (*mode) {
    case 'r':
        mflags |= _O_RDONLY;
        break;
    case 'w':
        mflags |= _O_WRONLY | _O_CREAT | _O_TRUNC;
        break;
    case 'a':
        mflags |= _O_APPEND | _O_CREAT;
        break;
    default:
        verrno = EINVAL;
        return EOF;
    }
    while (*++mode) {
        switch (*mode) {
        case '+':
            mflags &= ~(_O_RDONLY | _O_WRONLY);
            mflags |= _O_RDWR | _O_CREAT | _O_TRUNC;
            break;
        case 'b':
            mflags &= ~_O_TEXT;
            mflags |= _O_BINARY;
            break;
        default:
            verrno = EINVAL;
            return EOF;
        }
    }
    return mflags;
}


/*!
 * \brief Open a file.
 *
 * \param name The name of a registered device, optionally followed by a
 *             colon and a filename.
 * \param mode Operation mode. May be any of the following:
 * - _O_APPEND Always write at the end. 
 * - _O_BINARY Raw mode.
 * - _O_CREAT Create file if it does not exist. 
 * - _O_EXCL Open only if it does not exist. 
 * - _O_RDONLY Read only. 
 * - _O_RDWR Read and write. 
 * - _O_TEXT End of line translation. 
 * - _O_TRUNC Truncate file if it exists. 
 * - _O_WRONLY Write only.
 *
 * \return File descriptor for the opened file or -1 to indicate an error.
 */
int  _open(const char *name, int mode)
{
    CONTIKIDEVICE *dev;
    u_char dev_name[9];
    u_char nidx;
    const char *nptr = name;

    /*
     * Extract device name.
     */
    for (nidx = 0; *nptr && *nptr != ':' && nidx < 8; nidx++) {
        dev_name[nidx] = *nptr++;
    }
    dev_name[nidx] = 0;


    /*
     * Get device structure of registered device. In later releases we
     * try to open a file on a root device.
     */
    if ((dev = ContikiDeviceLookup((char*)dev_name)) == 0) {
        verrno = ENOENT;
        return -1;
    }

    /*
     * We should check, if the mode flags are device conformant.
     */

    /*
     * If a device name was specified, open this device. Otherwise open
     * a file on the device.
     */
    if (*nptr++ != ':')
        nptr = 0;
   // PRINTF((0,"_OPEN\r\n"));
    return (int) ((uptr_t) ((*dev->dev_open) (dev, nptr, mode, 0)));
}



/*!
 * \brief Write data to a file, device or socket.
 *
 * \param fd    Descriptor of a previously opened file, device or
 *              connected socket.
 * \param data  Pointer to data in program space to be written.
 * \param count Number of bytes to write.
 *
 * \return The number of bytes written, which may be less than the number
 *         of bytes specified. A return value of -1 indicates an error.
 *
 * \note   The write implementation of the underlying driver does not need
 *         to be thread-safe. Parallel writes using device usartavr will
 *         lead to intermixed data (if data doesn't fit into ringbuffer
 *         on the first try )
 */
int _write(int fd, const void *data, unsigned int count)
{
    CONTIKIFILE *fp = (CONTIKIFILE *) ((uptr_t) fd);
    CONTIKIDEVICE *dev = fp->nf_dev;

    return (*dev->dev_write) (fp, data, count);
}

/*!
 * \brief Read data from a file, device or socket.
 *
 * \param fd     Descriptor of a previously opened file, device or
 *               connected socket.
 * \param buffer Pointer to the buffer that receives the data.
 * \param count  Maximum number of bytes to read.
 *
 * \return The number of bytes read, which may be less than the number
 *         of bytes specified. A return value of -1 indicates an error.
 */
int _read(int fd, void *buffer, unsigned int count)
{
    CONTIKIFILE *fp = (CONTIKIFILE *) ((uptr_t) fd);
    CONTIKIDEVICE *dev = fp->nf_dev;

    return (*dev->dev_read) (fp, buffer, count);
}


/*!
 * \brief Get the file descriptor associated with a stream.
 *
 * In contrast to other implementations, the standard streams stdin, 
 * stdout and stderr do not return 0, 1 and 2 resp.
 *
 * \param stream Pointer to a previously opened stream.
 *
 * \return The file descriptor.
 *
 * \warning The function will not check, if the stream pointer points
 *          to a valid stream.
 */
int _fileno(FILE * stream)
{
    return stream->iob_fd;
}



/*!
 * \brief Perform device specific control functions.
 *
 * Check the specific device driver for a list of supported control 
 * functions.
 *
 * \param fd   Descriptor of a previously opened device or
 *             connected socket.
 * \param cmd  Requested control function.
 * \param data Points to a buffer that contains any data required for
 *             the given control function or receives data from that
 *             function.
 */
int _ioctl(int fd, int cmd, void *data)
{
    CONTIKIFILE *fp = (CONTIKIFILE *) ((uptr_t) fd);
    CONTIKIDEVICE *dev = fp->nf_dev;

     return dev->dev_ioctl ? (*dev->dev_ioctl) (dev, cmd, data) : -1;
//   return (*dev->dev_ioctl) (dev, cmd, data);
}

int fioctl(FILE*fp,int cmd,void*data)
{
   return _ioctl(fp->iob_fd,cmd,data);
}

/*!
 * \brief Open a stream.
 *
 * \param name The name of a registered device, optionally followed by a
 *             colon and a filename.
 * \param mode Specifies the access mode.
 *	       \li \c "r"  Read only.
 *	       \li \c "w"  Write only.
 *	       \li \c "a"  Write only at the end of file.
 *	       \li \c "r+" Read and write existing file.
 *	       \li \c "w+" Read and write, destroys existing file contents.
 *	       \li \c "a+" Read and write, preserves existing file contents.
 *         \li \c "b"  May be appended to any of the above strings to 
 *                        specify binary access.
 *
 * \return A pointer to the open stream or a null pointer to indicate 
 *         an error.
 */
FILE *fopen(const char *name, const char *mode)
{
    int mflags = _O_TEXT;

    /*
     * Translate file mode.
     */
    if ((mflags = _fmode(mode)) == EOF)
        return 0;
    {
		
        FILE fp;
        FILE *fpr = NULL;
        CONTIKIFILE *fCF = NULL;
       // PRINTF((0, "OPEN 0\r\n"));
        if ((fp.iob_fd = _open(name, mflags)) == -1) 
        {
			//PRINTF((0, "OPEN 0_1\r\n"));
            return NULL;
        }
       // PRINTF((0, "OPEN 0_2\r\n"));
       // PRINTF((0, "fp.iob_fd is %08x\r\n", fp.iob_fd));
        fCF = (CONTIKIFILE *)fp.iob_fd;
        fCF->devFile.iob_fd = fp.iob_fd;
        fCF->devFile.iob_flags = 0;
        fCF->devFile.iob_mode = mflags;
        fCF->devFile.iob_unget = 0;

        fpr = (FILE*)(&fCF->devFile);
        //fp->iob_mode = mflags;
        //fp->iob_flags = 0;
        //fp->iob_unget = 0;
       // PRINTF((0, "OPEN 1\r\n"));
        return fpr;
    }
}





/*!
 * \brief Write data to a stream.
 *
 * \param data   Pointer to items to be written.
 * \param size   Item size in bytes.
 * \param count  Number of items to write.
 * \param stream Pointer to a previously opened stream.
 *
 * \return The number of items written, which may be less than the 
 *         specified number.
 *
 * \warning The function will not check, if the stream pointer points
 *          to a valid stream.
 */
size_t fwrite(const void *data, size_t size, size_t count,FILE * stream)
{
    size_t rc;

    stream->iob_flags &= ~_IOERR;
    if (size > 1)
        count *= size;
    if ((int) (rc = (size_t) _write(stream->iob_fd, data, count)) <= 0)
    {
        stream->iob_flags |= _IOERR;
        return 0;
    }
    if (size > 1)
        rc /= size;
    return rc;
}





/*!
 * \brief Read data from a stream.
 *
 * \param buffer Pointer to the buffer that receives the data.
 * \param size   Item size in bytes.
 * \param count  Maximum number of items to read.
 * \param stream Pointer to a previously opened stream.
 *
 * \return The number of full items read, which may be less then the
 *         specified number.
 *
 * \warning The function will not check, if the stream pointer points
 *          to a valid stream.
 */
size_t fread(void *buffer, size_t size, size_t count,FILE * stream )
{
    size_t rc;
    size_t nu = 0;

    if (size > 1)
        count *= size;
    if (stream->iob_flags & _IOUNG) {
        stream->iob_flags &= ~_IOUNG;
        *((char *) buffer) = (char) stream->iob_unget;
        buffer = ((char *) buffer) + 1;
        count--;
        nu++;
    }
    
    stream->iob_flags &= ~_IOERR;
    rc = (size_t) _read(stream->iob_fd, buffer, count);
    if (rc == 0) {
        stream->iob_flags |= _IOEOF;
    }
    else if(rc == (size_t)-1) { // time out or io error
        stream->iob_flags |= _IOERR;
        rc = 0;
    }
    rc += nu;
    if (size > 1) {
        rc /= size;
    }
    return rc;
}



