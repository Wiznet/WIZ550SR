
#ifndef __GPIOHANDLER_H__
#define __GPIOHANDLER_H__

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

#define LEDn                             2

#if (WIZ550SR_ENABLE == 1)
#define LED1_PIN						GPIO_Pin_0
#define LED1_GPIO_PORT					GPIOB
#define LED1_GPIO_CLK					RCC_APB2Periph_GPIOB

#define LED2_PIN						GPIO_Pin_1
#define LED2_GPIO_PORT					GPIOB
#define LED2_GPIO_CLK					RCC_APB2Periph_GPIOB
#else
#if !defined(CHINA_BOARD)
#define LED1_PIN						GPIO_Pin_8
#define LED1_GPIO_PORT					GPIOA
#define LED1_GPIO_CLK					RCC_APB2Periph_GPIOA

#define LED2_PIN						GPIO_Pin_12
#define LED2_GPIO_PORT					GPIOC
#define LED2_GPIO_CLK					RCC_APB2Periph_GPIOC
#else
#define LED1_PIN						GPIO_Pin_0
#define LED1_GPIO_PORT					GPIOA
#define LED1_GPIO_CLK					RCC_APB2Periph_GPIOA

#define LED2_PIN						GPIO_Pin_1
#define LED2_GPIO_PORT					GPIOA
#define LED2_GPIO_CLK					RCC_APB2Periph_GPIOA
#endif
#endif

#if (WIZ550SR_ENABLE == 1)
//#define BOOT_PIN						GPIO_Pin_9
//#define BOOT_GPIO_PORT				GPIOB
//#define BOOT_GPIO_CLK					RCC_APB2Periph_GPIOB

#define FACT_PIN						GPIO_Pin_8
#define FACT_GPIO_PORT					GPIOB
#define FACT_GPIO_CLK					RCC_APB2Periph_GPIOB

#define STAT_PIN						GPIO_Pin_8
#define STAT_GPIO_PORT					GPIOA
#define STAT_GPIO_CLK					RCC_APB2Periph_GPIOA
#else
//#define BOOT_PIN						GPIO_Pin_8
//#define BOOT_GPIO_PORT				GPIOC
//#define BOOT_GPIO_CLK					RCC_APB2Periph_GPIOC

#define FACT_PIN						GPIO_Pin_12
#define FACT_GPIO_PORT					GPIOC
#define FACT_GPIO_CLK					RCC_APB2Periph_GPIOC
#endif

typedef enum
{
  LED1 = 0,
  LED2 = 1
} Led_TypeDef;

void LED_Init(Led_TypeDef Led);
void LED_On(Led_TypeDef Led);
void LED_Off(Led_TypeDef Led);
void LED_Toggle(Led_TypeDef Led);
uint8_t get_LED_Status(Led_TypeDef Led);
//void BOOT_Pin_Init();
//uint8_t get_bootpin_Status();
void GPIO_Configuration(void);
bool Board_factory_get(void);
void Board_factory_Init(void);
#if (WIZ550SR_ENABLE == 1)
void STAT_Init(void);
void STAT_On(void);
void STAT_Off(void);
#endif

#endif
