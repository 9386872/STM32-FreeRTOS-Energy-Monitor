#ifndef __ESP01s_H
#define __ESP01s_H	 
#include "main.h"

extern char R_data[1024];
extern uint16_t R_length;

extern uint8_t at_start_flag;
extern uint8_t aRxBuffer1;
extern char request_id[64];

uint8_t HuaweiIot_init(void);
void HuaweiIot_publish(void);

void AT_Send(char *cmd);
uint8_t AT_Wait(char *target,uint32_t timeout);

void HuaweiIot_Response(char *req_id);
void Esp_ParseFrame(void);

#endif // __ESP01s_H