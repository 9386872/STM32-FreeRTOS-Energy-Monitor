#include "esp01s.h"
#include "main.h"
#include "Type.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"

char R_data[1024];//用于保存ESP8266返回的数据。

uint16_t R_length = 0;//用于记录返回数据的长度

uint8_t at_start_flag = 0;

uint8_t aRxBuffer1;

char request_id[64];

extern ElectricData_t electricData;

/* ================================
 * AT发送
 * ================================ */
void AT_Send(char *cmd)
{
    HAL_UART_Transmit(&huart1,(uint8_t *)cmd, strlen(cmd),100);
}


/* ================================
 * AT等待（带超时）
 * ================================ */
uint8_t AT_Wait(char *target,uint32_t timeout)
{
    uint32_t start = HAL_GetTick();


    while(HAL_GetTick()-start < timeout)
    {

        if(strstr(R_data,target)!=NULL)
        {

            return 1;
        }


        osDelay(10);
    }


    return 0;
}

/*=========================================================
 * 回复华为云命令
 *========================================================*/
void HuaweiIot_Response(char *req_id)
{
    char cmd[256];

    sprintf(cmd,"AT+MQTTPUB=0,""\"$oc/devices/<DeviceID>/sys/commands/response/request_id=%s\",""\"\",0,0\r\n",req_id);

    AT_Send(cmd);
}

/*=========================================================
 * MQTT命令解析
 *=========================================================*/
static void ParseMqttCommand(void)
{
    char *p;
    int value;

    /***************************************************
     * 提取 request_id
     ***************************************************/
    p = strstr(R_data,"request_id=");

    if(p == NULL)
    {    
         printf("no request_id\r\n");
         return;
    }
    p += strlen("request_id=");

    char *end = strchr(p,'"');

    if(end == NULL)
        return;

    memset(request_id,0,sizeof(request_id));

    memcpy(request_id,p,end-p);

    printf("request_id=%s\r\n",request_id);


    /***************************************************
     * 风机继电器 fj_jdq
     ***************************************************/
    p = strstr(R_data,"fj_jdq");

    if(p != NULL)
    {
        p = strchr(p,':');

        if(p != NULL)
        {
            value = atoi(p+1);

            printf("fj_jdq=%d\r\n",value);

            if(value == 0)
            {
                fj_zt = 0;

                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4,GPIO_PIN_RESET);

                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_RESET);

                HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
            }
            else
            {
                fj_zt = 1;

                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,GPIO_PIN_SET);

                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
            }

            HuaweiIot_Response(request_id);
        }

        return;
    }


    /***************************************************
     * 光伏继电器 gf_jdq
     ***************************************************/
    p = strstr(R_data,"gf_jdq");

    if(p != NULL)
    {
        p = strchr(p,':');

        if(p != NULL)
        {
            value = atoi(p+1);

            printf("gf_jdq=%d\r\n",value);

            if(value == 0)
            {
                gf_zt = 0;

                HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);
            }
            else
            {
                gf_zt = 1;

                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5,GPIO_PIN_SET);
            }

            HuaweiIot_Response(request_id);
        }

        return;
    }
}

/*=========================================================
 * 一帧ESP数据解析
 *=========================================================*/
void Esp_ParseFrame(void)
{
    /* 调试输出，可根据需要关闭 */
    printf("ESP<< %s\r\n", R_data);

    /*************************************************
     * ESP启动完成
     *************************************************/
    if(strstr(R_data,"ready") != NULL)
    {
        at_start_flag = 1;

        printf("ESP Ready\r\n");

        return;
    }

    /*************************************************
     * MQTT下发命令
     *************************************************/
    if(strstr(R_data,"+MQTTSUBRECV") != NULL)
    {
        ParseMqttCommand(); 

        return;
    }

    /*************************************************
     * MQTT连接成功
     *************************************************/
    if(strstr(R_data,"+MQTTCONNECTED") != NULL)
    {
        printf("MQTT Connected\r\n");

        return;
    }

    /*************************************************
     * WiFi连接成功
     *************************************************/
    if(strstr(R_data,"WIFI GOT IP") != NULL)
    {
        printf("WiFi Got IP\r\n");

        return;
    }

    /*************************************************
     * OK
     *************************************************/
    if(strstr(R_data,"OK") != NULL)
    {
        return;
    }

    /*************************************************
     * ERROR
     *************************************************/
    if(strstr(R_data,"ERROR") != NULL)
    {
        printf("ESP ERROR : %s\r\n",R_data);

        return;
    }
}

/* ================================
 * ESP初始化（状态式）
 * ================================ */
uint8_t HuaweiIot_init(void)
{
    printf("Start ESP8266 Init...\r\n");

    /*-------------------------------
     * 测试AT
     *------------------------------*/
    AT_Send("AT\r\n");

    if(!AT_Wait("OK",2000))
    {
        printf("AT Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * 复位ESP8266
     *------------------------------*/
    AT_Send("AT+RST\r\n");

    osDelay(3000);

    if(!AT_Wait("ready",5000))
    {
        printf("ESP Reset Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * STA模式
     *------------------------------*/
    AT_Send("AT+CWMODE=1\r\n");

    if(!AT_Wait("OK",3000))
    {
        printf("CWMODE Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * 连接WiFi
     *------------------------------*/
    AT_Send("AT+CWJAP=\"<WIFI_SSID>\",\"<WIFI_PASSWORD>\"\r\n");

    if(!AT_Wait("WIFI GOT IP",15000))
    {
        printf("WiFi Connect Failed\r\n");
        return 0;
    }

    printf("WiFi Connected\r\n");

    /*-------------------------------
     * MQTT用户配置
     *------------------------------*/
    AT_Send(
        "AT+MQTTUSERCFG=0,1,""\"NULL\",""\"<Username>\","
        "\"<Password>\","
        "0,0,\"\"\r\n");

    if(!AT_Wait("OK",3000))
    {
        printf("MQTTUSERCFG Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * ClientID
     *------------------------------*/
    AT_Send(
        "AT+MQTTCLIENTID=0," "\"<ClientID>\"\r\n");

    if(!AT_Wait("OK",3000))
    {
        printf("MQTTCLIENTID Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * MQTT连接
     *------------------------------*/
    AT_Send(
        "AT+MQTTCONN=0,"
        "\"<MQTT_BROKER_ADDRESS>\","
        "1883,1\r\n");

    if(!AT_Wait("+MQTTCONNECTED",10000))
    {
        printf("MQTT Connect Failed\r\n");
        return 0;
    }

    printf("MQTT Connected\r\n");

    /*-------------------------------
     * 订阅属性
     *------------------------------*/
    AT_Send(
        "AT+MQTTSUB=0,"
        "\"$oc/devices/<DeviceID>/sys/properties/report\",1\r\n");

    if(!AT_Wait("OK",3000))
    {
        printf("SUB1 Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * 订阅命令
     *------------------------------*/
    AT_Send(
        "AT+MQTTSUB=0,"
        "\"$oc/devices/<DeviceID>/sys/commands/#\",1\r\n");

    if(!AT_Wait("OK",3000))
    {
        printf("SUB2 Failed\r\n");
        return 0;
    }

    /*-------------------------------
     * 订阅响应
     *------------------------------*/
    AT_Send(
        "AT+MQTTSUB=0,"
        "\"$oc/devices/<DeviceID>/sys/commands/response/#\",1\r\n");

    if(!AT_Wait("OK",3000))
    {
        printf("SUB3 Failed\r\n");
        return 0;
    }

    printf("Huawei IoT Init Success\r\n");

    return 1;
}

void HuaweiIot_publish(void)
{
    ElectricData_t mqttData;

    char pubtemp[512];

    xSemaphoreTake(ElectricDataMutexHandle, portMAX_DELAY);

    mqttData = electricData;

    xSemaphoreGive(ElectricDataMutexHandle);

       snprintf(pubtemp,
         sizeof(pubtemp),
         "AT+MQTTPUB=0,"
         "\"$oc/devices/<DeviceID>/sys/properties/report\","
         "\"{\\\"services\\\":[{"
         "\\\"service_id\\\":\\\"Power_parameters\\\","
         "\\\"properties\\\":{"
         "\\\"gf_channel_voltage\\\":%.2f,"
         "\\\"Fan_channel_voltage\\\":%.2f,"
         "\\\"batteryVolt\\\":%.2f,"
         "\\\"batteryCurrent\\\":%.2f,"
         "\\\"ydtd_Volt\\\":%.2f,"
         "\\\"ydtd_Current\\\":%.2f,"
         "\\\"battery_capacity\\\":%.2f"
         "}}]}\",0,0\r\n",

         mqttData.gf_volt,
         mqttData.fj_volt,  
         mqttData.batteryVolt,
         mqttData.batteryCurrent,
         mqttData.ydtd_Volt,
         mqttData.ydtd_Current,
         mqttData.soc);
    AT_Send(pubtemp);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if(huart->Instance == USART1)
    {
        xQueueSendFromISR(EspRxQueueHandle, &aRxBuffer1,&xHigherPriorityTaskWoken);

        HAL_UART_Receive_IT(&huart1, &aRxBuffer1,1);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}
