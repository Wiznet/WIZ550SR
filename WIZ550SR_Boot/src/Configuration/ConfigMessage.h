
#ifndef __CONFIGMESSAGE_H__
#define __CONFIGMESSAGE_H__

#include "stm32f10x.h"

/* UDP port numbers for Remote Configuration */
#define REMOTE_CLIENT_PORT		6550	/* from client to server */
#define REMOTE_CLIENT_PORT_EX	1460	/* from client to server */
#define REMOTE_UPDATE_PORT		1470	/* from client to server */

#define STX						0xA5

#define DISCOVERY_ALL			0xA1
#define DISCOVERY_PRODUCT_CODE	0xA2
#define DISCOVERY_MAC_ADDRESS	0xA3
#define DISCOVERY_ALIAS			0xA4
#define DISCOVERY_MIXED_COND	0xA5

#define FIRMWARE_UPLOAD_INIT	0xD1
#define FIRMWARE_UPLOAD_DONE	0xD2

#define DISCOVERY				0xA0
#define GET_INFO				0xB0
#define SET_INFO				0xC0
#define FIRMWARE_UPLOAD			0xD0
#define REMOTE_RESET			0xE0
#define FACTORY_RESET			0xF0

#define WIZNET_REQUEST			0xAA
#define WIZNET_REPLY			0x55

typedef struct __WIZnet_Header {
	uint8_t stx;
	uint8_t valid;
	uint8_t unicast;
	uint8_t op_code[2];
	uint16_t length;
} __attribute__((packed)) WIZnet_Header;

typedef struct __WIZnet_Discovery_All {
} __attribute__((packed)) WIZnet_Discovery_All;

typedef struct __WIZnet_Discovery_Product_Code {
	uint8_t product_code[3];
	uint8_t fw_version[3];
	uint8_t option;
} __attribute__((packed)) WIZnet_Discovery_Product_Code;

typedef struct __WIZnet_Discovery_MAC_Address {
	uint8_t start_mac_address[6];
	uint8_t end_mac_address[6];
} __attribute__((packed)) WIZnet_Discovery_MAC_Address;

typedef struct __WIZnet_Discovery_Alias {
	uint8_t alias[32];
	uint8_t option;
} __attribute__((packed)) WIZnet_Discovery_Alias;

typedef struct __WIZnet_Discovery_Mixed_Condition {
	uint8_t product_code[3];
	uint8_t fw_version[3];
	uint8_t fw_version_option;
	uint8_t start_mac_address[6];
	uint8_t end_mac_address[6];
	uint8_t alias[32];
	uint8_t alias_option;
} __attribute__((packed)) WIZnet_Discovery_Mixed_Condition;

typedef struct __WIZnet_Discovery_Reply {
	uint8_t product_code[3];
	uint8_t fw_version[3];
	uint8_t mac_address[6];
} __attribute__((packed)) WIZnet_Discovery_Reply;

typedef struct __WIZnet_Get_Info {
	uint8_t dst_mac_address[6];
} __attribute__((packed)) WIZnet_Get_Info;

typedef struct __WIZnet_Get_Info_Reply {
	uint8_t src_mac_address[6];
	S2E_Packet system_info;
} __attribute__((packed)) WIZnet_Get_Info_Reply;

typedef struct __WIZnet_Set_Info {
	uint8_t dst_mac_address[6];
	uint8_t set_pw_len;
	uint8_t set_pw[16];
	S2E_Packet system_info;
} __attribute__((packed)) WIZnet_Set_Info;

typedef struct __WIZnet_Set_Info_Reply {
	uint8_t src_mac_address[6];
	S2E_Packet system_info;
} __attribute__((packed)) WIZnet_Set_Info_Reply;

typedef struct __WIZnet_Firmware_Upload_Init {
	uint8_t dst_mac_address[6];
	uint8_t set_pw_len;
	uint8_t set_pw[16];
	uint8_t server_ip[4];
	uint16_t server_port;
	uint8_t file_name[50];
} __attribute__((packed)) WIZnet_Firmware_Upload_Init;

typedef struct __WIZnet_Firmware_Upload_Init_Reply {
	uint8_t src_mac_address[6];
} __attribute__((packed)) WIZnet_Firmware_Upload_Init_Reply;

typedef struct __WIZnet_Firmware_Upload_Done_Reply {
	uint8_t src_mac_address[6];
} __attribute__((packed)) WIZnet_Firmware_Upload_Done_Reply;

typedef struct __WIZnet_Reset {
	uint8_t dst_mac_address[6];
	uint8_t set_pw_len;
	uint8_t set_pw[16];
} __attribute__((packed)) WIZnet_Reset;

typedef struct __WIZnet_Reset_Reply {
	uint8_t src_mac_address[6];
} __attribute__((packed)) WIZnet_Reset_Reply;

typedef struct __WIZnet_Factory_Reset {
	uint8_t dst_mac_address[6];
	uint8_t set_pw_len;
	uint8_t set_pw[16];
} __attribute__((packed)) WIZnet_Factory_Reset;

typedef struct __WIZnet_Factory_Reset_Reply {
	uint8_t src_mac_address[6];
} __attribute__((packed)) WIZnet_Factory_Reset_Reply;

typedef struct __Remote_Info {
	uint8_t ip[4];
	uint16_t port;
} __attribute__((packed)) Remote_Info;

typedef struct __Firmware_Upload_Info {
	WIZnet_Header wiznet_header;
	Remote_Info configtool_info;
	Remote_Info tftp_info;
	uint8_t filename[50];
} __attribute__((packed)) Firmware_Upload_Info;

#define	REMOTE_FIND		1
#define	REMOTE_SETT		2
#define CONFIG_MSG_SIZE		163		// 90 + 64 + 9
typedef struct _CONFIG_MSG
{
	u8  op[4];
	u8  Mac[6];
	u8  Kind; 	// 0: Client 1: Mixed 2: Server mode.
	u8  Lip[4];
	u8  Subnet[4];
	u8  Gw[4];
	u8	LPort[2];
	u8  Sip[4];
	u8	SPort[2];
	u8  Baud;
	u8  Dsize;
	u8  Parity;
	u8  Stopbit;
	u8  Flow;
	u8  D_ch;
	u8	D_size[2];
	u8	D_time[2];
	u8	I_time[2];
	u8  Debug;
	u8	SW_Ver[2];
	u8  DHCP;
	u8  UDP;
	u8  Status;
	u8  DNS_Flag;			// 50 bytes
	u8  DNS_Server_IP[4];
	u8  Domain_Name[32];
	u8  SCfg;
	u8  SCfgStr[3];

	// ver 2.0
	u8  PPPoE_ID[32];
	u8  PPPoE_PWD[32];

	// ver 2.1
	u8  EnConnPass;
	u8  ConnPass[8];
}
CONFIG_MSG;

typedef union _IGM_INFO {
	CONFIG_MSG s;
	u8 c[CONFIG_MSG_SIZE];
} IGM_INFO;

void do_udp_config(uint8_t sock);
void do_udp_configex(uint8_t sock);
void do_fw_update(void);
void reply_firmware_upload_done(uint8_t sock);

#endif
