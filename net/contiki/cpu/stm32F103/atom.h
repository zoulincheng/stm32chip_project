#ifndef _SYS_ATOM_H
#define _SYS_ATOM_H


#define OSInitCritical(p)  (void)(*p)
#define OSEnterCritical(p) *(p)=sysDisable()
#define OSExitCritical(p)    sysEnable(*(p))
#define OSOutCritical(p)  OSExitCritical(p)

typedef		u32		OSCRITICAL;


#endif



