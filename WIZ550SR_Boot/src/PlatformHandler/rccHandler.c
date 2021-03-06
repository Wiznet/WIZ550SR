/**
 * @file	rccHandler.c
 * @brief	RCC Handler Example
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author	
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include "stm32f10x.h"
#include "rccHandler.h"
#include "common.h"

/**
 * @brief  Sets System clock frequency to 72MHz and configure HCLK, PCLK2
 *         and PCLK1 prescalers.
 * @param  None
 * @return None
 */
#if 1
void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;

	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer */
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state */
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK2 = HCLK */
		RCC_PCLK2Config(RCC_HCLK_Div1);

		/* PCLK1 = HCLK/2 */
		RCC_PCLK1Config(RCC_HCLK_Div2);

		/* PLLCLK = 8MHz * 9 = 72 MHz */
#if !defined(CHINA_BOARD)
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
#else
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
#endif
		//    RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_9);

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}

		/* TIM2 clock enable */
		//  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2|RCC_APB1Periph_TIM3|RCC_APB1Periph_USART2, ENABLE);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

#if (WIZ550SR_ENABLE == 1)
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
#else
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
#endif

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
				| RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1, ENABLE);
	}
}
#else
void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
			| RCC_APB2Periph_AFIO  | RCC_APB2Periph_USART1, ENABLE);
}
#endif
