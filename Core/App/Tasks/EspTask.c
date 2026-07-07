#include "main.h"
#include "Type.h"
#include "esp01s.h"
#include "usart.h"
#include <string.h>

extern uint8_t aRxBuffer1;


/*
====================================================
 ESP任务
====================================================
*/

void EspTask(void *argument)
{

    uint8_t ch;
    uint32_t last_pub = 0;
    uint8_t mqtt_connected = 0;

    /*
    开启UART接收中断

    一个字节一个字节接收
    */

    HAL_UART_Receive_IT(&huart1,&aRxBuffer1,1);



    for(;;)
    {
        /*
        ==========================================
        处理ESP返回数据
        ==========================================
        */
        while(xQueueReceive( EspRxQueueHandle,&ch,0)==pdPASS)
        {
/*收到换行，认为一帧结束 ESP返回格式：xxx\r\n */

            if(ch!='\n')
            {

                if(R_length < sizeof(R_data)-1)
                {

                    R_data[R_length++]=ch;

                    R_data[R_length]='\0';
                }
            }

            else
            {
                if(R_length>0)
                {

                    /*解析完整一帧*/
                    Esp_ParseFrame();
                    /*清空缓存*/
                    memset(R_data, 0, sizeof(R_data));
                    R_length=0;
                }
            }
        }

        /*
        ==========================================
        ESP初始化
        ==========================================
        */
        if(at_start_flag && mqtt_connected==0)
        {
            mqtt_connected = HuaweiIot_init();
        }

        /*
        ==========================================
        5秒上传数据
        ==========================================
        */
        if(mqtt_connected)
        {
            if(HAL_GetTick()-last_pub >=5000)
            {

                last_pub = HAL_GetTick();
                HuaweiIot_publish();
            }
        }

        /*任务周期 */
        osDelay(100);
    }

}