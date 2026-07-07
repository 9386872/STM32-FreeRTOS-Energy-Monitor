#ifndef __ESP01s_H
#define __ESP01s_H	 
#include "main.h"

extern char R_data[1024];
extern uint16_t R_length;

extern uint8_t at_start_flag;
extern uint8_t aRxBuffer1;

uint8_t HuaweiIot_init(void);
void HuaweiIot_publish(void);

void AT_Send(char *cmd);
uint8_t AT_Wait(char *target,uint32_t timeout);

#endif // __ESP01s_H