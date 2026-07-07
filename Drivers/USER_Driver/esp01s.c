#include "esp01s.h"
#include "main.h"
#include "Type.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"

char R_data[1024];//用于保存ESP8266返回的数据。
extern ElectricData_t electricData;
uint16_t R_length = 0;//用于记录返回数据的长度

uint8_t at_start_flag = 0;

uint8_t aRxBuffer1;


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
uint8_t AT_Wait(char *target, uint32_t timeout)
{
    uint32_t start = HAL_GetTick();

    while (HAL_GetTick() - start < timeout)
    {
        if (strstr(R_data, target) != NULL)
        {
            memset(R_data, 0, sizeof(R_data));
            R_length = 0;
            return 1;
        }

        osDelay(10);
    }

    return 0;
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
