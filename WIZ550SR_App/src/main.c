//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "common.h"
#include "rccHandler.h"
#include "spiHandler.h"
#include "uartHandler.h"
#include "flashHandler.h"
#include "storageHandler.h"
#include "gpioHandler.h"
#include "timerHandler.h"
#include "ConfigData.h"
#include "ConfigMessage.h"
#include "atcmd.h"
#include "dhcp.h"
#include "dns.h"
#include "S2E.h"
#include "dhcp_cb.h"
#include "i2cHandler.h"
#include "eepromHandler.h"
#include "stm32f10x_iwdg.h"

// ----------------------------------------------------------------------------
//
// STM32F1 empty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

///////////////////////////////////////
// Debugging Message Printout enable //
///////////////////////////////////////
#define _MAIN_DEBUG_

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
uint8_t g_send_buf[WORK_BUF_SIZE];
uint8_t g_recv_buf[WORK_BUF_SIZE];

uint8_t run_dns = 1;
uint8_t op_mode;

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int main(int argc, char* argv[])
{
	S2E_Packet *value = get_S2E_Packet_pointer();
	uint8_t dns_server_ip[4];

	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x7000);

	RCC_Configuration();
	GPIO_Configuration();

	LED_Init(LED1);
	LED_Init(LED2);
	
	LED_On(LED1);
	LED_Off(LED2);

#if (WIZ550SR_ENABLE == 1)
	STAT_Init();
	STAT_Off();
#endif

	//BOOT_Pin_Init();
	//Board_factory_Init();

	/* Initialize the I2C EEPROM driver ----------------------------------------*/
#if defined(EEPROM_ENABLE)
#if defined(EEPROM_ENABLE_BYI2CPERI)
	I2C1Initialize();
#elif defined(EEPROM_ENABLE_BYGPIO)
	EE24AAXX_Init();
#endif
#endif

#if defined(MULTIFLASH_ENABLE)
	probe_flash();
#endif

	/* Load Configure Information */
	load_S2E_Packet_from_storage();

	USART1_Configuration();
#if !defined(CHINA_BOARD)
	USART2_Configuration();
#endif

	W5500_SPI_Init();
	W5500_Init();
	Timer_Configuration();

	Mac_Conf();
	DHCP_init(SOCK_DHCP, g_send_buf);
	reg_dhcp_cbfunc(w5500_dhcp_assign, w5500_dhcp_assign, w5500_dhcp_conflict);

#ifdef _MAIN_DEBUG_
    printf("\r\nApplication started...\r\n");
    printf("=======================================\r\n");
	printf(" WIZnet WIZ550SR Revision 1.0\r\n");
	printf(" Embedded Serial to Ethernet Module\r\n");
	printf(" Firmware Version %d.%d.%d\r\n", MAJOR_VER, MINOR_VER, MAINTENANCE_VER);
	printf("=======================================\r\n");

	printf(" # Device Name : %s\r\n\r\n", value->module_name);
#endif

	/* Initialize Network Information */
	if(value->options.dhcp_use) {		// DHCP
		uint32_t ret;
		uint8_t dhcp_retry = 0;

		//printf("Start DHCP...\r\n");
		while(1) {
			ret = DHCP_run();

			if(ret == DHCP_IP_LEASED)
				break;
			else if(ret == DHCP_FAILED)
				dhcp_retry++;

			if(dhcp_retry > 3) {
				Net_Conf();
				break;
			}
			do_udp_config(SOCK_CONFIG);
		}
		display_Net_Info();
	} else 								// Static
		Net_Conf();

	DNS_init(SOCK_DNS, g_send_buf);
	if(value->options.dns_use) {
		uint8_t dns_retry = 0;

		memcpy(dns_server_ip, value->options.dns_server_ip, sizeof(dns_server_ip));

		while(1) {
			if(DNS_run(dns_server_ip, (uint8_t *)value->options.dns_domain_name, value->network_info[0].remote_ip) == 1)
				break;
			else
				dns_retry++;

			if(dns_retry > 3) {
				break;
			}

			do_udp_config(SOCK_CONFIG);

			if(value->options.dhcp_use)
				DHCP_run();
		}
	}

	atc_init();

	op_mode = OP_DATA;

	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_256);
	IWDG_SetReload(156);
	IWDG_ReloadCounter();
	IWDG_Enable();

	while (1) {
		IWDG_ReloadCounter();

		if(op_mode == OP_COMMAND) {			// Command Mode
			atc_run();
			sockwatch_run();
		} else {							// DATA Mode
			s2e_run(SOCK_DATA);
		}

#if defined(WIZ1x0SR_CFGTOOL)
		do_udp_configex(SOCK_CONFIGEX);
#else
		do_udp_config(SOCK_CONFIG);
#endif

		if(value->options.dhcp_use)
			DHCP_run();

		if(value->options.dns_use && run_dns == 1) {
			memcpy(dns_server_ip, value->options.dns_server_ip, sizeof(dns_server_ip));

			if(DNS_run(dns_server_ip, (uint8_t *)value->options.dns_domain_name, value->network_info[0].remote_ip) == 1) {
				run_dns = 0;
			}
		}
	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
