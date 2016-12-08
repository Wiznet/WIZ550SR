
#include <stdbool.h>
#include "stm32f10x.h"
#include "gpioHandler.h"
#include "uartHandler.h"
#include "timerHandler.h"
#include "ConfigData.h"
#include "S2E.h"
#include "dhcp.h"
#include "dns.h"

uint8_t nagle_flag = 0;
uint32_t nagle_time = 0;
uint32_t uart_recv_count = 0;

uint8_t reconn_flag = 0;			/* 0 : connect / 1 : NONE */
uint32_t reconn_time = 0;

uint8_t inactive_flag = 0;
uint32_t inactive_time = 0;
uint32_t ether_send_prev;
uint32_t ether_recv_prev;
uint8_t keepsend_flag = 0;
uint32_t keepsend_time = 0;

uint8_t trigger_flag = 0;
uint32_t trigger_time = 0;

extern uint8_t factory_flag;
uint32_t factory_time = 0;

uint8_t auth_flag = 0;
uint32_t auth_time = 0;

extern uint8_t run_dns;

static uint32_t mill_cnt = 0;
static uint32_t sec_cnt = 0;

extern bool Board_factory_get(void);

void TIM2_IRQHandler(void)
{
	struct __network_info *net = (struct __network_info *)get_S2E_Packet_pointer()->network_info;
	struct __options *option = (struct __options *)&(get_S2E_Packet_pointer()->options);
	uint32_t count = 0;

	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearFlag(TIM2, TIM_FLAG_Update);
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // Clear the interrupt flag
	
        mill_cnt++;

        /* UART Packing Timer Process */
        if(!nagle_flag && net->packing_time) {
            count = RingBuffer_GetCount(&rxring);

            if(count != 0 && count == uart_recv_count)
                nagle_time++;
            else {
                nagle_time = 0;
                uart_recv_count = count;
            }

            if(nagle_time >= net->packing_time) {
                nagle_flag = 1;
                nagle_time = 0;
            }
        }

        /* Reconnection Process */
        if(reconn_flag)
            reconn_time++;

        if(net->reconnection <= reconn_time) {
            reconn_flag = 0;
            reconn_time = 0;
        }

        /* Factory Reset Process */
        if(factory_flag) {
            factory_time++;

            if(!Board_factory_get()) {
                factory_time = factory_flag = 0;
                NVIC_EnableIRQ(EXTI15_10_IRQn);
            }

            if(factory_time >= 5000) {
                /* Factory Reset */
                set_S2E_Packet_to_factory_value();
                save_S2E_Packet_to_storage();

                NVIC_SystemReset();
            }
        }

        /* Serial Trigger Timer Process */
        if(trigger_flag == 1)
            trigger_time++;

        if(trigger_time >= 500) {
            trigger_flag = 2;
            trigger_time = 0;
        }

        /* Second Process */
        if((mill_cnt % 1000) == 0) {
            LED_Toggle(LED1);
            mill_cnt = 0;
            sec_cnt++;

            /* DHCP Process */
            if(option->dhcp_use)
                DHCP_time_handler();

            /* DNS Process */
            if(option->dns_use) {
                DNS_time_handler();
            }

            /* Inactive Time Process */
            if(inactive_flag == 1) {
                if((ether_recv_cnt == ether_recv_prev) && (ether_send_cnt == ether_send_prev))
                    inactive_time++;
                else {
                    ether_send_prev = ether_send_cnt;
                    ether_recv_prev = ether_recv_cnt;
                    inactive_time = 0;
                }
            }

            if(net->inactivity && (net->inactivity <= inactive_time)) {
                inactive_flag = 2;
                inactive_time = 0;
            }

            /* Connect Password Process */
            if(auth_flag)
                auth_time++;

            /* Minute Process */
            if((sec_cnt % 60) == 0) {
                sec_cnt = 0;

                /* DNS Process */
                if(option->dns_use) {
                    run_dns = 1;
                }
            }

			/* Keepsend Time Process */
			if(keepsend_flag == 1) {
				keepsend_time++;
				if((keepsend_time % 5) == 0) {
					keepsend_time = 0;
					keepsend_flag = 2;
				}
			}
        }
	}
}

/**
 * @brief  Configures the Timer
 * @param  None
 * @return None
 */
void Timer_Configuration(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM2, 71, TIM_PSCReloadMode_Immediate);

	/* TIM enable counter */
	TIM_Cmd(TIM2, ENABLE);

	/* TIM IT enable */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

void delay_cnt(uint32_t count)
{
	volatile uint32_t tmp = count;

	while(tmp--);
}

void delay_us (const uint32_t usec)
{
	RCC_ClocksTypeDef  RCC_Clocks;

	/* Configure HCLK clock as SysTick clock source */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);

	RCC_GetClocksFreq(&RCC_Clocks);

	// Set SysTick Reload(1us) register and Enable
	// usec * (RCC_Clocks.HCLK_Frequency / 1000000) < 0xFFFFFFUL  -- because of 24bit timer
	SysTick_Config(usec * (RCC_Clocks.HCLK_Frequency / 1000000));
	// 72/72000000 --> 1usec
	// 0.001msec = 1usec
	// 1Hz = 1sec, 10Hz = 100msec, 100Hz = 10msec, 1KHz = 1msec,
	// 10KHz = 0.1msec, 100Khz = 0.01msec, 1MHz = 1usec(0.001msec)
	// 1usec = 1MHz


	// SysTick Interrupt Disable
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk ;

	// Until Tick count is 0
	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}

void delay_ms (const uint32_t msec)
{
	delay_us(1000 * msec);
}