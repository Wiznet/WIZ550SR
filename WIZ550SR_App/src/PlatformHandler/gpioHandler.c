
#include "stm32f10x.h"
#include "gpioHandler.h"
#include "spiHandler.h"

uint8_t factory_flag = 0;

GPIO_TypeDef* GPIO_PORT[LEDn] = {LED1_GPIO_PORT, LED2_GPIO_PORT};
const uint16_t GPIO_PIN[LEDn] = {LED1_PIN, LED2_PIN};
const uint32_t GPIO_CLK[LEDn] = {LED1_GPIO_CLK, LED2_GPIO_CLK};

/**
  * @brief  Configures LED GPIO.
  * @param  Led: Specifies the Led to be configured.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_Init(Led_TypeDef Led)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(GPIO_CLK[Led], ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = GPIO_PIN[Led];
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(GPIO_PORT[Led], &GPIO_InitStructure);

}

/**
  * @brief  Turns selected LED On.
  * @param  Led: Specifies the Led to be set on.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_On(Led_TypeDef Led)
{
  //GPIO_PORT[Led]->BSRR = GPIO_PIN[Led];
  GPIO_ResetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}

/**
  * @brief  Turns selected LED Off.
  * @param  Led: Specifies the Led to be set off.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_Off(Led_TypeDef Led)
{
  //GPIO_PORT[Led]->BRR = GPIO_PIN[Led];
  GPIO_SetBits(GPIO_PORT[Led], GPIO_PIN[Led]);
}

/**
  * @brief  Toggles the selected LED.
  * @param  Led: Specifies the Led to be toggled.
  *   This parameter can be one of following parameters:
  *     @arg LED1
  *     @arg LED2
  * @retval None
  */
void LED_Toggle(Led_TypeDef Led)
{
  GPIO_PORT[Led]->ODR ^= GPIO_PIN[Led];
}

uint8_t get_LED_Status(Led_TypeDef Led)
{
	return GPIO_ReadOutputDataBit(GPIO_PORT[Led], GPIO_PIN[Led]);
}

#if 0
/**
  * @brief	Configures GPIO for D0~D15
  * @param	io Specifies the GPIO to be configured.
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return None
  */
void BOOT_Pin_Init()
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	
  	/* Configure the Boot trigger pin */
  	GPIO_InitStructure.GPIO_Pin = BOOT_PIN;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//GPIO_Mode_IN_FLOATING;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
  	GPIO_Init(BOOT_GPIO_PORT, &GPIO_InitStructure);
}

/**
  * @brief	To get the status of GPIO.
  * @param	io Specifies the GPIO to get a status
  *   This parameter can be one of following parameters:
  * 	@arg D0 @arg D1 @arg D2 @arg D3 @arg D4
  * 	@arg D5 @arg D6 @arg D7 @arg D8 @arg D9
  * 	@arg D10 @arg D11 @arg D12 @arg D13 @arg D14 @arg D15
  * @return The status of GPIO
  */
uint8_t get_bootpin_Status()
{
	return GPIO_ReadInputDataBit(BOOT_GPIO_PORT, BOOT_PIN);
}
#endif

/**
  * @brief  Configures the GPIO
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  // Port C Output
  GPIO_InitStructure.GPIO_Pin = W5500_RESET_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(W5500_RESET_PORT, &GPIO_InitStructure);

  GPIO_SetBits(W5500_RESET_PORT, W5500_RESET_PIN);

  // Port C input
  GPIO_InitStructure.GPIO_Pin = INT_W5500_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(INT_W5500_PORT, &GPIO_InitStructure);
}

bool Board_factory_get(void)
{
	return GPIO_ReadInputDataBit(FACT_GPIO_PORT, FACT_PIN);
}

void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line12) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line12);

    	factory_flag = 1;

    	NVIC_DisableIRQ(EXTI15_10_IRQn);
    }
}

void Board_factory_Init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // Define GPIO Init structure
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Configure the GPIOC ports for input */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Connect EXTI */
    // External Interrupt configuration register1 (AFIO_EXTICR2)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource12);

    /* Configure EXTI12  to generate an interrupt on falling edge */
    EXTI_InitStructure.EXTI_Line = EXTI_Line12;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    // 2 bit for preemption priority, 2 bits for sub priority
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Clear EXTI Line Pending Bit */
    EXTI_ClearITPendingBit(EXTI_Line12);
    /* Enable the Key EXTI line Interrupt */
    NVIC_ClearPendingIRQ(EXTI15_10_IRQn);
}

#if (WIZ550SR_ENABLE == 1)
void STAT_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Enable the GPIO_LED Clock */
  RCC_APB2PeriphClockCmd(STAT_GPIO_CLK, ENABLE);

  /* Configure the GPIO_LED pin */
  GPIO_InitStructure.GPIO_Pin = STAT_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_Init(STAT_GPIO_PORT, &GPIO_InitStructure);
}

void STAT_On(void)
{
  GPIO_ResetBits(STAT_GPIO_PORT, STAT_PIN);
}

void STAT_Off(void)
{
  GPIO_SetBits(STAT_GPIO_PORT, STAT_PIN);
}
#endif
