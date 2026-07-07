#include "main.h"
#include "Type.h"
#include "adc.h"
#include "INA226.h"
#include "soc.h"
/* ADC缓存 */
static uint16_t Power_parameters[2];
ElectricData_t electricData;
// void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
// {
//     if(hadc->Instance == ADC1)
//     {
//         electricData.gf_volt = (float)Power_parameters[0] * 3.3f / 4095.0f;
//         electricData.fj_volt = (float)Power_parameters[1] * 3.3f / 4095.0f;

//         BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//         vTaskNotifyGiveFromISR(menuHandle, &xHigherPriorityTaskWoken);
//         portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//     }
// }
uint16_t Read_ADC_Channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);

    HAL_ADC_PollForConversion(&hadc1, 10);

    uint16_t value = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);

    return value;
}

void CollectionTask(void *argument)
{
      ElectricData_t tempData;
      /* SOC初始化（可选：防止开机跳变） */
    soc_init(INA226_GetBusV1());
    for(;;)
    {
      /* ===== ADC采集 ===== */
        Power_parameters[0] = Read_ADC_Channel(ADC_CHANNEL_0);
        Power_parameters[1] = Read_ADC_Channel(ADC_CHANNEL_1);

        tempData.gf_volt =(float)Power_parameters[0] * 3.3f / 4095.0f;

        tempData.fj_volt =(float)Power_parameters[1] * 3.3f / 4095.0f;

        /* ===== INA226采集 ===== */
        tempData.batteryVolt =INA226_GetBusV1();

        tempData.batteryCurrent =INA226_GetCurrent1();

        tempData.ydtd_Volt =INA226_GetBusV2();

        tempData.ydtd_Current =INA226_GetCurrent2();

        // /* ===== SOC计算 ===== */
        update_soc(tempData.batteryVolt,tempData.batteryCurrent,100.0f );   // 采样周期100ms

        tempData.soc = get_current_soc();
          /* 加锁 */
        xSemaphoreTake(ElectricDataMutexHandle,portMAX_DELAY);

        electricData = tempData;

        xSemaphoreGive(ElectricDataMutexHandle);
        osDelay(100);
    }
}