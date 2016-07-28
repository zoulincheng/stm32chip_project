解决办法1:
用IAR6.3打开IAR6.0 的工程，编译的时候出现提示错误：
extern uint32_t __get_PSP(void); 已经在C:\Program Files\IAR Systems\Embedded Workbench 6.0\arm\inc\c\intrinsics.h文件中定义
extern uint32_t __get_MSP(void);已经在C:\Program Files\IAR Systems\Embedded Workbench 6.0\arm\inc\c\intrinsics.h文件中定义
。。。。。。

这是由于IAR6.3已经加入了CM3的内核定义，就不需要ST库中的core_cm3.c 和 core_cm3.h 这两个文件的定义了

解决办法从ST网站找到:
直接删除core_cm3.c 和 core_cm3.h 这两个文件，然后在工程设置里面勾选 "Use CMSIS"




转自：http://www.cnblogs.com/ele-eye/archive/2011/11/28/2266229.html

方法二：
把core_cm3.h里面这句给注释掉，然后加上一句
#if defined (__ICCARM__)
//#include <intrinsics.h> /* IAR Intrinsics */
__intrinsic void __DSB(void);
#endif
完全没有任何警告和错误。
我的是IAR6.3+STM32F10x_StdPeriph_Driver V3.5.0
转自：http://www.amobbs.com/thread-5172279-1-1.html

方法三：
直接用IAR6.3安装文件下的core_cm3.h 



在使用IAR 进行编译的时候出现以下情况：

Error[Li005]: no definition for "__program_start" [referenced fromC:\Users\tsacy\Desktop\USB\USBLib\demos\JoyStickMouse\project\EWARM\Debug\Obj\stm32f10x_vector.o]
Error while running Linker
如何解决的呢？
在网上查询了很久以后，发现：
这是因为在高版本下编译低版本的项目工程而造成的，在高版本编译的时候，
使用如下方法可完好的解决上述问题：
打开stm32f10x_vector.c 注意下划线长度的区别
void __program_start( void);    改为：void__iar_program_start(void);
__program_start,                改为：__iar_program_start,
完成后，编译OK！！！