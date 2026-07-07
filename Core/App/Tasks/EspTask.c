#include "main.h"
#include "Type.h"
#include "esp01s.h"
#include "usart.h"
#include <string.h>

extern uint8_t aRxBuffer1;
extern uint8_t at_start_flag;
extern uint16_t R_length;


void EspTask(void *argument)
{
    uint8_t ch;//队列里取出的一个字符

    uint32_t last_pub = 0;//记录上一次上传数据的时间。

    uint8_t mqtt_connected = 0;//连接标志位

    HAL_UART_Receive_IT(&huart1,&aRxBuffer1,1);

    for(;;)
    {
        while(xQueueReceive(EspRxQueueHandle,&ch, 0) == pdPASS)
        {   
            //拼接字符串
            if(R_length < sizeof(R_data)-1)
            {
                R_data[R_length++] = ch;

                R_data[R_length] = 0;//字符串结束符
            }

            if(strstr(R_data,"ready"))
            {
                at_start_flag = 1;
            }
        }

        /* 初始化 */
        if(at_start_flag && mqtt_connected == 0)
        {
            mqtt_connected = HuaweiIot_init();
        }

        /* 5秒上传一次 */
        if(mqtt_connected)
        {
            if(HAL_GetTick() - last_pub >= 5000)
            {
                last_pub = HAL_GetTick();

                HuaweiIot_publish();
            }
        }

        osDelay(100);
    }
}