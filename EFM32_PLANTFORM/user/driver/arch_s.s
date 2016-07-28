		MODULE  ?_arch
		PRESERVE8
		SECTION .text:CODE:NOROOT(2)
		
		
		THUMB
		
;/////////////////////////////////////////////////////////////////////////////////////////////////////////
;/////////////////////////////////////////////////////////////////////////////////////////////////////////
; void sysEnable(void);
; restore the primask saved by sysDisable()
;sysEnable   proc
		PUBLIC sysEnable
sysEnable:
		msr     primask,r0
		bx      lr		


;/////////////////////////////////////////////////////////////////////////////////////////////////////////
; void sysEnable(void);
; disable the exception and return the primask value
;sysDisable  proc
		PUBLIC sysDisable
sysDisable:
		mrs     r0,primask
		cpsid   I
		bx      lr
		
AIRCR       EQU     0xe000ed0c
SYSRESETREQ EQU     0x05fa0004
;sysReset	proc
		PUBLIC sysReset
sysReset:
		cpsid   i
		mrs     r1,CONTROL
		and     r1,r1,#~2
		msr     CONTROL,r1
		isb
		ldr     r0,=AIRCR
		ldr     r1,=SYSRESETREQ
		str     r1,[r0]
		b       .
			;endp

;sysGetLR    proc
		PUBLIC sysGetLR
sysGetLR:
		mrs       r0,MSP
		ldr       r0,[r0]
		bx        lr
		
;sysGetSP    proc
;		PUBLIC sysGetSP
;sysGetSP:
;		cmp       r0,#0
;		mrseq     r0,PSP                     ;if bit 2 LR is zero, then its using MSP befor enter handle
;		mrsne     r0,MSP
;		bx        lr		
		
		END