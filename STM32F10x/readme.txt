����취1:
��IAR6.3��IAR6.0 �Ĺ��̣������ʱ�������ʾ����
extern uint32_t __get_PSP(void); �Ѿ���C:\Program Files\IAR Systems\Embedded Workbench 6.0\arm\inc\c\intrinsics.h�ļ��ж���
extern uint32_t __get_MSP(void);�Ѿ���C:\Program Files\IAR Systems\Embedded Workbench 6.0\arm\inc\c\intrinsics.h�ļ��ж���
������������

��������IAR6.3�Ѿ�������CM3���ں˶��壬�Ͳ���ҪST���е�core_cm3.c �� core_cm3.h �������ļ��Ķ�����

����취��ST��վ�ҵ�:
ֱ��ɾ��core_cm3.c �� core_cm3.h �������ļ���Ȼ���ڹ����������湴ѡ "Use CMSIS"




ת�ԣ�http://www.cnblogs.com/ele-eye/archive/2011/11/28/2266229.html

��������
��core_cm3.h��������ע�͵���Ȼ�����һ��
#if defined (__ICCARM__)
//#include <intrinsics.h> /* IAR Intrinsics */
__intrinsic void __DSB(void);
#endif
��ȫû���κξ���ʹ���
�ҵ���IAR6.3+STM32F10x_StdPeriph_Driver V3.5.0
ת�ԣ�http://www.amobbs.com/thread-5172279-1-1.html

��������
ֱ����IAR6.3��װ�ļ��µ�core_cm3.h 



��ʹ��IAR ���б����ʱ��������������

Error[Li005]: no definition for "__program_start" [referenced fromC:\Users\tsacy\Desktop\USB\USBLib\demos\JoyStickMouse\project\EWARM\Debug\Obj\stm32f10x_vector.o]
Error while running Linker
��ν�����أ�
�����ϲ�ѯ�˺ܾ��Ժ󣬷��֣�
������Ϊ�ڸ߰汾�±���Ͱ汾����Ŀ���̶���ɵģ��ڸ߰汾�����ʱ��
ʹ�����·�������õĽ���������⣺
��stm32f10x_vector.c ע���»��߳��ȵ�����
void __program_start( void);    ��Ϊ��void__iar_program_start(void);
__program_start,                ��Ϊ��__iar_program_start,
��ɺ󣬱���OK������