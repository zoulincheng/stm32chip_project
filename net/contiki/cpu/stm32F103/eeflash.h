#ifndef _EEFLASH_H
#define _EEFLASH_H




#ifdef __cplusplus
extern "C"{
#endif

//#define 		MCU_STM32F103RDT6
#define 		MCU_STM32F103RBT6


extern u_long eeBlockGetBase(void);
extern u_long eeBlockGetSize(void);

extern u_long eeBlockRead(u_long dwAbsAddr,void*poBuf,u_long dwiSize);
extern u_long eeBlockWrite(u_long dwAbsAddr,const void*pciBuf,u_long dwiSize);


#ifdef __cplusplus
}
#endif

#endif
