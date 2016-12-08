
#ifndef __TIMERHANDLER_H__
#define __TIMERHANDLER_H__

extern uint8_t nagle_flag;
extern uint32_t nagle_time;
extern uint32_t uart_recv_count;

extern uint8_t reconn_flag;
extern uint32_t reconn_time;

extern uint8_t inactive_flag;
extern uint32_t inactive_time;
extern uint32_t ether_send_prev;
extern uint32_t ether_recv_prev;
extern uint8_t keepsend_flag;
extern uint32_t keepsend_time;

extern uint8_t trigger_flag;
extern uint32_t trigger_time;

extern uint8_t factory_flag;
extern uint32_t factory_time;

extern uint8_t auth_flag;
extern uint32_t auth_time;

void Timer_Configuration(void);
void delay_cnt(uint32_t count);
void delay_us (const uint32_t usec);
void delay_ms (const uint32_t msec);

#endif
