
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

#define MAJOR_VER		1
#define MINOR_VER		2
#define MAINTENANCE_VER	2

#define SOCK_DATA		0
#define SOCK_TFTP		1
#define SOCK_CONFIG		2
#define SOCK_DHCP		3
#define SOCK_DNS		4
#define SOCK_CONFIGEX	5

#define OP_COMMAND		0
#define OP_DATA			1

#define APP_BASE		0x08007000			// Boot Size 28K
#define WORK_BUF_SIZE	2048

#define WIZ550SR_ENABLE	1
//#define WIZ1x0SR_CFGTOOL
//#define CHINA_BOARD

extern uint8_t op_mode;

#endif
