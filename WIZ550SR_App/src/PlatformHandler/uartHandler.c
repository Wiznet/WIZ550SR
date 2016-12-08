/**
 * @file	uartHandler.c
 * @brief	UART Handler Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "uartHandler.h"
#include "ConfigData.h"

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/*******************************************************************************
* Function Name  : PUTCHAR_PROTOTYPE
* Description    : Retargets the C library printf function to the USART.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
PUTCHAR_PROTOTYPE
{
    /* Write a character to the USART */
    USART_SendData(UART_DEBUG, (u8) ch);

    /* Loop until the end of transmission */
    while(USART_GetFlagStatus(UART_DEBUG, USART_FLAG_TXE) == RESET){}

    return ch;
}

ssize_t _write (int fd __attribute__((unused)), const char* buf __attribute__((unused)), size_t nbyte __attribute__((unused)))
{
    int n;

    switch (fd) {
    case 1: /*stdout*/
    case 2: /* stderr */
        for (n = 0; n < nbyte; n++) {
        	__io_putchar(*buf++ & (uint16_t)0x01FF);
        }
        break;
    default:
        errno = EBADF;
        return -1;
    }

    return nbyte;
}

#if (DATA7BIT_ENABLE == 1)
uint8_t g_data_7bit_flag;
uint8_t g_data_7bit_none_flag;
#endif

RINGBUFF_T txring, rxring;
static uint8_t rxbuff[UART_RRB_SIZE], txbuff[UART_SRB_SIZE];

uint32_t baud_table[11] = {
	600,
	1200,
	2400,
	4800,
	9600,
	19200,
	38400,
	57600,
	115200, 
	230400
};

uint32_t Chip_UART_SendRB(USART_TypeDef *pUART, RINGBUFF_T *pRB, const void *data, int bytes)
{
	uint32_t ret;
	uint8_t *p8 = (uint8_t *) data;

	/* Don't let UART transmit ring buffer change in the UART IRQ handler */
	USART_ITConfig(pUART, USART_IT_TXE, DISABLE);

	/* Move as much data as possible into transmit ring buffer */
	ret = RingBuffer_InsertMult(pRB, p8, bytes);

	/* Enable UART transmit interrupt */
	USART_ITConfig(pUART, USART_IT_TXE, ENABLE);

	return ret;
}

int Chip_UART_ReadRB(USART_TypeDef *pUART, RINGBUFF_T *pRB, void *data, int bytes)
{
	(void) pUART;
	int ret;

	USART_ITConfig(pUART, USART_IT_RXNE, DISABLE);

	ret = RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);

	USART_ITConfig(pUART, USART_IT_RXNE, ENABLE);

	return ret;
}

int Chip_UART_ReadRB_BLK(USART_TypeDef *pUART, RINGBUFF_T *pRB, void *data, int bytes)
{
	(void) pUART;
	int ret;

	while(RingBuffer_IsEmpty(pRB));

	USART_ITConfig(pUART, USART_IT_RXNE, DISABLE);

	ret = RingBuffer_PopMult(pRB, (uint8_t *) data, bytes);

	USART_ITConfig(pUART, USART_IT_RXNE, ENABLE);

	return ret;
}

/**
 * @brief  USART Handler
 * @param  None
 * @return None
 */
void Chip_UART_IRQRBHandler(USART_TypeDef *pUART, RINGBUFF_T *pRXRB, RINGBUFF_T *pTXRB)
{
	uint8_t ch;

	/* Handle transmit interrupt if enabled */
	if(USART_GetITStatus(pUART, USART_IT_TXE) != RESET) {
		if(RingBuffer_Pop(pTXRB, &ch))
		{
#if defined(RS485_ENABLE)
			RS485_TX_ENABLE();
#endif
#if (DATA7BIT_ENABLE == 1)
			//if(pUART == USART2 && g_data_7bit_none_flag == 1)
			//	USART_SendData(pUART, (ch|0x80));
			//else
				USART_SendData(pUART, ch);
			//if (pUART == USART2)
			//	printf("T0x%x %d\r\n", ch, g_data_7bit_flag);
#else
			USART_SendData(pUART, ch);
#endif
		}
		else												// RingBuffer Empty
		{
			USART_ITConfig(pUART, USART_IT_TXE, DISABLE);
#if defined(RS485_ENABLE)
			USART_ITConfig(pUART, USART_IT_TC, ENABLE);
#endif
		}
	}

	/* Handle receive interrupt */
	if(USART_GetITStatus(pUART, USART_IT_RXNE) != RESET) {
		if(RingBuffer_IsFull(pRXRB)) {
			// Buffer Overflow
		} else {

#if (DATA7BIT_ENABLE == 1)
			if(pUART == USART2 && g_data_7bit_flag == 1)
				ch = (uint8_t)(USART_ReceiveData(pUART) & 0x7F);
			else
				ch = (uint8_t)USART_ReceiveData(pUART);
			//if (pUART == USART2)
			//	printf("R0x%x %d\r\n", ch, g_data_7bit_flag);
#else
			ch = (uint8_t)USART_ReceiveData(pUART);
#endif
			RingBuffer_Insert(pRXRB, &ch);
		}
	}

#if defined(RS485_ENABLE)
	// interrupt was for send (TC)
	if(USART_GetITStatus(pUART, USART_IT_TC) != RESET)
	{
		// Disable TC Interrupt and TC pending bit clear
	    USART_ITConfig(pUART, USART_IT_TC, DISABLE);
	    USART_ClearFlag(pUART, USART_FLAG_TC);

	    RS485_TX_DISABLE();
	    USART_ITConfig(pUART, USART_IT_RXNE, ENABLE);
	}
#endif
}

/**
 * @brief  USART1 Interrupt Handler
 * @param  None
 * @return None
 */
void USART1_IRQHandler(void)
{
#if (USART2_ENABLE == 0)
	Chip_UART_IRQRBHandler(USART1, &rxring, &txring);
#endif
}

/**
 * @brief  Configures the USART1
 * @param  None
 * @return None
 */
void USART1_Configuration(void)
{
#if (USART2_ENABLE == 1)
	USART_InitTypeDef USART_InitStructure;
#endif
	USART_ClockInitTypeDef USART_ClockInitStruct;

	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

#if (USART2_ENABLE == 0)
	S2E_Packet *value = get_S2E_Packet_pointer();

	/* Ring Buffer */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);
#endif

	/* Enable the USART Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  USART1_TX;// | USART1_RTS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = USART1_RX;// | USART1_CTS;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USARTx configuration ------------------------------------------------------*/
	/* USARTx configured as follow:
	   - BaudRate = 115200 baud
	   - Word Length = 8 Bits
	   - One Stop Bit
	   - No parity
	   - Hardware flow control disabled (RTS and CTS signals)
	   - Receive and transmit enabled
	 */
#if (USART2_ENABLE == 1)
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
#endif

	USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable;

	/* Configure the USARTx */
#if (USART2_ENABLE == 0)
	serial_info_init(USART1, &(value->serial_info[0])); // Load USART1 Settings from Flash
#endif
	USART_ClockInit(USART1, &USART_ClockInitStruct);

	/* Enable USARTy Receive and Transmit interrupts */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 

	/* Enable the USARTx */
	USART_Cmd(USART1, ENABLE);
}

void USART2_IRQHandler(void)
{
#if (USART2_ENABLE == 1)
	Chip_UART_IRQRBHandler(USART2, &rxring, &txring);
#endif
}

void USART2_Configuration(void)
{
#if (USART2_ENABLE == 0)
	USART_InitTypeDef USART_InitStructure;
#endif
	USART_ClockInitTypeDef USART_ClockInitStruct;

	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

#if (USART2_ENABLE == 1)
	S2E_Packet *value = get_S2E_Packet_pointer();

	/* Ring Buffer */
	RingBuffer_Init(&rxring, rxbuff, 1, UART_RRB_SIZE);
	RingBuffer_Init(&txring, txbuff, 1, UART_SRB_SIZE);
#endif

	/* Enable the USART Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

#if defined(RS485_ENABLE)
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  USART2_TX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  USART2_RTS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = USART2_RX;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  USART2_TX | USART2_RTS;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = USART2_RX | USART2_CTS;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif

	/* USARTx configuration ------------------------------------------------------*/
	/* USARTx configured as follow:
	   - BaudRate = 115200 baud
	   - Word Length = 8 Bits
	   - One Stop Bit
	   - No parity
	   - Hardware flow control disabled (RTS and CTS signals)
	   - Receive and transmit enabled
	 */
#if (USART2_ENABLE == 0)
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
#endif
	USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;
	USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;
	USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge;
	USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable;

	/* Configure the USARTx */
#if (USART2_ENABLE == 1)
	serial_info_init(USART2, &(value->serial_info[0])); // Load USART2 Settings from Flash
#endif
	USART_ClockInit(USART2, &USART_ClockInitStruct);

	/* Enable USARTy Receive and Transmit interrupts */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	/* Enable the USARTx */
	USART_Cmd(USART2, ENABLE);
}

int UART_read(void *data, int bytes)
{
	return Chip_UART_ReadRB(UART_DATA , &rxring, data, bytes);
}

uint32_t UART_write(void *data, int bytes)
{
	return Chip_UART_SendRB(UART_DATA, &txring, data, bytes);
}

int UART_read_blk(void *data, int bytes)
{
	return Chip_UART_ReadRB_BLK(UART_DATA , &rxring, data, bytes);
}

void UART_buffer_flush(RINGBUFF_T *buf)
{
	RingBuffer_Flush(buf);
}

void myprintf(char *fmt, ...)
{
	va_list arg_ptr;
	char etxt[128]; // buffer size

	va_start(arg_ptr, fmt);
	vsprintf(etxt, fmt, arg_ptr);
	va_end(arg_ptr);

	UART_write(etxt, strlen(etxt));
}

void serial_info_init(USART_TypeDef *pUART, struct __serial_info *serial)
{
	USART_InitTypeDef USART_InitStructure;

	uint32_t i, loop, valid_arg = 0;

	loop = sizeof(baud_table) / sizeof(baud_table[0]);
	for(i = 0 ; i < loop ; i++) {
		if(serial->baud_rate == baud_table[i]) {
			USART_InitStructure.USART_BaudRate = serial->baud_rate;
			valid_arg = 1;
			break;
		}
	}
	if(!valid_arg)
		USART_InitStructure.USART_BaudRate = baud_115200;

	/* Set Stop Bits */
	switch(serial->stop_bits) {
		case stop_bit1:
			USART_InitStructure.USART_StopBits = USART_StopBits_1;
			break;
		case stop_bit2:
			USART_InitStructure.USART_StopBits = USART_StopBits_2;
			break;
		default:
			USART_InitStructure.USART_StopBits = USART_StopBits_1;
			serial->stop_bits = stop_bit1;
			break;
	}

	/* Set Parity Bits */
	switch(serial->parity) {
		case parity_none:
			USART_InitStructure.USART_Parity = USART_Parity_No;
			break;
		case parity_odd:
			USART_InitStructure.USART_Parity = USART_Parity_Odd;
			break;
		case parity_even:
			USART_InitStructure.USART_Parity = USART_Parity_Even;
			break;
		default:
			USART_InitStructure.USART_Parity = USART_Parity_No;
			serial->parity = parity_none;
			break;
	}

	/* Flow Control */
	switch(serial->parity) {
	case flow_none:
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		break;
	case flow_rts_cts:
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
		break;
	default:
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		break;
	}

	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	if (serial->data_bits == word_len8 && serial->parity != parity_none)
	{
		USART_InitStructure.USART_WordLength = USART_WordLength_9b;
	}

#if (DATA7BIT_ENABLE == 1)
	if (serial->data_bits == word_len7)
	{
		g_data_7bit_flag = 1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		//if (serial->parity == parity_none)
		//{
		//	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
		//	g_data_7bit_none_flag = 1;
		//}
		//else
		//{
		//	g_data_7bit_none_flag = 0;
		//}
	}
	else
	{
		g_data_7bit_flag = 0;
		//g_data_7bit_none_flag = 0;
	}
	//printf("7:%d %d", serial->data_bits, g_data_7bit_flag);
#endif

	/* Configure the USARTx */
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(pUART, &USART_InitStructure);
}
