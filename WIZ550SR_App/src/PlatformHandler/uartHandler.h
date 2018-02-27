/**
 * @file	uartHandler.h
 * @brief	Header File for UART Handler Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#ifndef __UARTHANDLER_H__
#define __UARTHANDLER_H__

#include "stm32f10x.h"
#include "ring_buffer.h"
#include "ConfigData.h"

#define UART_DEBUG USART1
#define UART_DATA USART2
#define USART2_ENABLE	1

#define DATA7BIT_ENABLE 0

#define USART1_TX		GPIO_Pin_9	// out
#define USART1_RX		GPIO_Pin_10	// in
#define USART1_CTS		GPIO_Pin_11	// in
#define USART1_RTS		GPIO_Pin_12	// out

#define USART2_CTS		GPIO_Pin_0	// in
#define USART2_RTS		GPIO_Pin_1	// out
#define USART2_TX		GPIO_Pin_2	// out
#define USART2_RX		GPIO_Pin_3	// in

#define UART_SRB_SIZE 2048	/* Send */
#define UART_RRB_SIZE 2048	/* Receive */

//#define RS485_ENABLE
#if defined(RS485_ENABLE)
#define RS485_TX_ENABLE()   GPIO_SetBits(GPIOA, USART2_RTS)
#define RS485_TX_DISABLE()  GPIO_ResetBits(GPIOA, USART2_RTS)
#endif

extern uint32_t baud_table[11];
extern RINGBUFF_T txring, rxring;
void USART1_Configuration(void);
void USART2_Configuration(void);

uint32_t Chip_UART_SendRB(USART_TypeDef *pUART, RINGBUFF_T *pRB, const void *data, int bytes);
int Chip_UART_ReadRB(USART_TypeDef *pUART, RINGBUFF_T *pRB, void *data, int bytes);
int UART_read(void *data, int bytes);
uint32_t UART_write(void *data, int bytes);
int UART_read_blk(void *data, int bytes);
void UART_buffer_flush(RINGBUFF_T *buf);
void myprintf(char *fmt, ...);
void serial_info_init(USART_TypeDef *pUART, struct __serial_info *serial);

#if (DATA7BIT_ENABLE == 1)
extern uint8_t g_data_7bit_flag;
extern uint8_t g_data_7bit_none_flag;
#endif

#endif

