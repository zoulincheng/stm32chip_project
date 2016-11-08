#ifndef _APPKEY_H
#define _APPKEY_H


typedef struct key_msg
{
	u_char ubKeyValue;
	u_char ubCountTime;
}KEY_MSG;


#define KEY_ALARM		0x01
#define KEY_SILENCE		0x02
#define KEY_SELFTEST	0x04

#endif
