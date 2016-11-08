
#ifndef _BASICTYPE_H
#define _BASICTYPE_H


//typedef unsigned short uint16_t;
//typedef unsigned char uint8_t;

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;



typedef unsigned char BOOL;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

//typedef signed   char  int8;
//typedef signed   short int16;
//typedef signed   int   int32;

typedef float          fp32;
typedef double         fp64;

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;

typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;

typedef unsigned char  INT8U;
typedef unsigned short INT16U;
typedef unsigned int   INT32U;

typedef signed   char  INT8;
typedef signed   short INT16;
typedef signed   int   INT32;

typedef float          FP32;
typedef double         FP64;

//related with C Compiler setting
typedef unsigned long      ULONG;
typedef signed   long      LONG;
typedef signed   long long int64;
typedef unsigned long long uint64;
typedef unsigned long long INT64U;
typedef signed   long long INT64;
typedef unsigned long long UINT64;

typedef int 	BITF;


/*! \brief Unsigned 8-bit value. */
typedef unsigned char      u_char;
/*! \brief Unsigned 16-bit value. */
typedef unsigned short     u_short;
/*! \brief Unsigned 16-bit value. */
typedef unsigned int       u_int;
/*! \brief Unsigned 32-bit value */
typedef unsigned long      u_long;
/*! \brief Unsigned 64-bit value */
typedef unsigned long long u_longlong;

typedef unsigned char U8;
typedef unsigned short U16;
typedef short			S16;
typedef unsigned int U32;
typedef int			S32;
//typedef unsigned char uint8_t;
/*! \brief Void pointer */
typedef void * HANDLE;

typedef unsigned int   uptr_t;
typedef const char      prog_char;
//typedef unsigned int size_t;

//typedef unsigned int size_t;
//typedef int *va_list[1];

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL  ((void*)0)
#endif

typedef unsigned int u_ptrint;

typedef enum
{
    false   = 0,
    true    = 1
} bool;


#define countof(const_array)  (sizeof(const_array)/sizeof((const_array)[0]))


#endif






