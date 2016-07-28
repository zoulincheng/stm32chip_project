#ifndef _HWLH_LIGHTING_H
#define _HWLH_LIGHTING_H


#define HL_HEARER			0x7b
#define HL_END				0x7d

#define HL_CMD_T1_RED_LED_2HZ			0x01
#define HL_CMD_T2_RED_LED_5HZ			0x02
#define HL_CMD_T3						0x03
#define HL_CMD_GET_CURRENT_SLAVE_STATE	0x05
#define HL_CMD_CLEAR_CURRENT_SLAVE_STATE	0x20;
#define HL_CMD_EXIT_TEST_MODE			0x21
#define HL_CMD_SLAVE_SET_ADDR			0x30


#define HL_STATE_T1MODE		0x01
#define HL_STATE_T2MODE		0x02
#define HL_STATE_T3MODE		0x03
#define HL_STATE_EMERGENCY_MODE	0x04
#define HL_STATE_GET_CURRENT	0x05
#define HL_STATE_SLAVE_SEND		0x06


#define HL_ACK_SLAVE	0x02
#define HL_ACK_MASTER	0x01



typedef struct _hl_cmd
{
	u_char ubHead;
	u_char ubaGroupAddr[2];
	u_char ubaID[2];
	u_char ubCMD;
	u_char ubaCRC[2];
	u_char ubEnd;
}HL_CMD_ST;


typedef struct _hl_state
{
	u_char ubHead;
	u_char ubaGroupAddr[2];
	u_char ubaID[2];
	//state
	u_char ubSlaveStateM;
	u_char ubEmergencyTime;	//minute
	u_char ubaBatteryVoltage[2]; 	//[1]<<8+[0] d0d1 = 0x70 08 = 2160
	u_char ubaLoadVoltage[2];		//[1]<<8+[0]
	u_char ubaLoadCurrent[2];
	u_char ubaBatteryFullVoltage[2];
	u_char ubaBatteryFullTime[2];
	u_char ubaBatteryChargeTime[2];

	u_char ubaCRC[2];
	u_char ubaEnd;
}HL_STATE_ST;


typedef struct _hl_ack
{
	u_char ubHead;
	u_char ubaGroupAddr[2];
	u_char ubaID[2];
	u_char ubACK;

	u_char ubaCRC[2];
	u_char ubaEnd;
}HL_ACK_ST;


typedef union _hl_frame
{
	u_char 		ubaData[32];
	HL_CMD_ST 	st_HL_CMD;
	HL_STATE_ST st_HL_STATE;
	HL_ACK_ST	st_HL_ACK;
}HL_FRAME_U;




#endif

