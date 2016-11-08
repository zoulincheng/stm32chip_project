#ifndef _MP3APP_H
#define _MP3APP_H


#define MP3_HEAD		0x7e
#define MP3_END			0xef


/*
Frame format
BY1680-12P内置标准UART异步串口接口，为3.3V TTL电平接口。可通过MAX3232芯片转换成RS232电平或者通过USB转TTL模块与PC通讯进行调试。
通讯数据格式是：起始位：1位；数据位：8位；奇偶位：无；停止位：1位。使用电脑串口调试助手，需要正确设置串口的参数：
协议命令格式：
起始码  长度  操作码  参数  校验码 结束码
0X7E   见下文 见下文  见下文 见下文 0XEF
注意： 数据全部为十六进制数。
“长度”是指：长度+操作码+参数(有些没有参数，有些有两位参数)+校验码的个数；
“校验码”是指：长度<异或>操作码<异或>参数的值，既按顺序分别异或的值。

校验码客户可通过计算器计算得到：例如，设置音量指令为  7E 04 3119 2C EF
     长度04是这样得到：就是“04”，“31”，“19”，“2C”4个数；
校验码2C是这样得到：
首先打开计算器选择程序员模式；
然后选择16进制、双字；
最后点击进行计算  04 Xor 31 Xor19 = 2C
*/

/*
7.1 指令列表
通信控制指令（指令发送成功返回OK，歌曲播放完停止返回STOP）。
*/
//CMD详解							对应功能				参数(ASCK码)
#define MP3_CMD_PLAY		0x11		//播放						无
#define MP3_CMD_PAUSE		0x12		//暂停						无
#define MP3_CMD_NEXT		0x13		//下一曲
#define MP3_CMD_PREV		0x14		//上一曲
#define MP3_CMD_V_ADD		0x15		//音量加
#define MP3_CMD_V_SUB		0x16		//音量减
#define MP3_CMD_RST			0x19		//复位
#define MP3_CMD_SPEED		0x1a		//快进
#define MP3_CMD_NSPEED		0x1b		//快退
#define MP3_CMD_PLAY_PAUSE	0x1c	//播放/暂停
#define MP3_CMD_STOP		0x1e		//停止

//CMD详解							对应功能				参数(8位HEX)
#define MP3_CMD_SET_V			0x31		//设置音量				0-30级可调(音量掉电记忆)
#define MP3_CMD_SET_EQ			0x32		//设置EQ				0-5(NO\POP\ROCK\JAZZ\CLASSIC\BASS)
#define MP3_CMD_SET_LOOP_MODE	0x33	//设置循环模式		0-4(全盘/文件夹/单曲/随机/不循环
#define MP3_CMD_CHANGE_DIR		0x34	//文件夹切换		0（上一文件夹），1(下一文件夹) ，flash内没有此功能
#define MP3_CMD_CHANGE_DEV		0x35	//设备切换			0（U盘），1（SD）
#define MP3_CMD_ADKEY_UP		0x36	//ADKEY软件上拉		1开上拉（10K电阻），0关上拉，默认0
#define MP3_CMD_ADKEY_EN		0x37	//ADKEY使能			1开起，0关闭，默认1
#define MP3_CMD_BUSY_LEVL		0x38	//BUSY电平切换  1为播放输出高电平，0为播放输出低电平，默认1

//CMD详解					参数(16hex)（16位）
#define MP3_CMD_SELECT			0x41	//选择播放曲目		1-最大首目
#define MP3_CMD_SELECT_FILE		0x42	//指定文件夹曲目播放	高八位为文件夹号(00-99)，低八位为歌曲名字(001-255)，flash内没有此功能
#define MP3_CMD_INSERT_PLAY		0x43	//插播功能			1-最大首目
#define MP3_CMD_INSERT_PLAY_FILE	0x44	//插播指定文件夹里面的歌曲播放	插播指定文件夹里面的歌曲播放



//通信查询命令
//	注意事项：连续发送两条命令之间间隔在20MS以上，组合播放功能两条命令在6MS以内。
//CMD详解					返回参数(ASCK码)（16位）
#define MP3_CMD_PLAY_STATE		0x20		//查询播放状态			0(停止)1(播放) 2(暂停) 3(快进)4(快退)
#define MP3_CMD_V_STATE			0x21		//查询音量大小			0-30(掉电记忆)
#define MP3_CMD_EQ_STATE		0x22		//查询当前 EQ			0-5(NO\POP\ROCK\JAZZ\CLASSIC\BASS
#define	MP3_CMD_CUR_PLAY_MODE	0x23		//查询当前播放模式		0-4(全盘/文件夹/单曲/随机/无循环)
#define	MP3_CMD_CUR_VERSION		0x24		//查询版本号			1.0
#define MP3_CMD_FLASH_FILES		0x25		//查询FLASH的总文件数	1-255
#define MP3_CMD_U_FILES			0x2a		//查询U盘的当前曲目		1-65536
#define MP3_CMD_PLAY_TIME		0x2c		//查询当前播放歌曲的时间	反回时间（秒）
#define MP3_CMD_PLAY_TOTAL_TIME	0x2d		//查询当前播放歌曲总时间	反回时间（秒）
#define MP3_CMD_PLAY_NAME		0x2e		//查询当前播放歌曲歌名		反回歌曲名（只能返回前两位数），flash内没有此功能
#define MP3_CMD_CUR_PLAY_NUMS	0x2f		//查询当前播放文件夹内总数量	0-65536，flash内没有此功能



typedef struct _mp3_common_head
{
	u_char ubHead;
	u_char ubLen;		//ublen 1 + opt code 1 + params L + check code 1
	u_char ubOPT;		//cmd
}MP3_COMMON_HEAD;

typedef struct _mp3_con_no_param
{
	MP3_COMMON_HEAD comHead;
	u_char ubXorCode;
	u_char ubEnd;
}MP3_CON_NO_PARAM;

//b one byte param
typedef struct _mp3_con_b_param
{
	MP3_COMMON_HEAD comHead;
	u_char ubParam;
	u_char ubXorCode;
	u_char ubEnd;
}MP3_CON_B_PARAM;


//w two bytes param
typedef struct _mp3_con_w_param
{
	MP3_COMMON_HEAD comHead;
	u_char ubParamH;
	u_char ubParamL;
	u_char ubXorCode;
	u_char ubEnd;
}MP3_CON_W_PARAM;

#endif
