#ifndef SI4438_INT_CTR_DEF_H
#define SI4438_INT_CTR_DEF_H

//INT SOURCE ENABLE
#define RF_CHIP_INT_STATUS_EN		0x04
#define RF_MODEM_INT_STATUS_EN		0x02
#define RF_PH_INT_STATUS_EN			0x01

#define RF_CHIP_INT_DATA_INDEX		0x03
#define RF_MODEM_INT_DATA_INDEX		0x02
#define RF_PH_INT_DATA_INDEX		0x01




//INT_CTL_PH_ENABLE
#define RF_INT_CTL_PH_FILTER_MATCH_EN				0x80
#define RF_INT_CTL_PH_FILTER_MISS_EN				0x40
#define RF_INT_CTL_PH_PACKET_SENT_EN				0x20
#define RF_INT_CTL_PH_PACKET_RX_EN					0x10
#define RF_INT_CTL_PH_CRC_ERROR_EN					0x08
#define RF_INT_CTL_PH_ALT_CRC_ERROR_EN				0x04
#define RF_INT_CTL_PH_TX_FIFO_ALMOST_EMPTY_EN		0x02
#define RF_INT_CTL_PH_RX_FIFO_ALMOST_FULL_EN		0x01

//INT_CTL_MODEM_ENABLE
#define RF_INT_CTL_MODEM_RSSI_LATCH_EN				0x80
#define RF_INT_CTL_MODEM_POSTAMBLE_DETECT_EN		0x40
#define RF_INT_CTL_MODEM_INVALID_SYNC_EN			0x20
#define RF_INT_CTL_MODEM_RSSI_JUMP_EN				0x10
#define RF_INT_CTL_MODEM_RSSI_EN					0x08
#define RF_INT_CTL_MODEM_INVALID_PREAMBLE_EN		0x04
#define RF_INT_CTL_MODEM_PREAMBLE_DETECT_EN			0x02
#define RF_INT_CTL_MODEM_SYNC_DETECT_EN				0x01

//INT_CTL_CHIP_ENABLE
#define INT_CTL_CHIP_CAL_EN								0x40
#define INT_CTL_CHIP_FIFO_UNDERFLOW_OVERFLOW_ERROR_EN	0x20
#define INT_CTL_CHIP_STATE_CHANGE_EN					0x10
#define INT_CTL_CHIP_CMD_ERROR_EN						0x08
#define INT_CTL_CHIP_CHIP_READY_EN						0x04
#define INT_CTL_CHIP_LOW_BATT_EN						0x02
#define INT_CTL_CHIP_WUT_EN								0x01



#endif

