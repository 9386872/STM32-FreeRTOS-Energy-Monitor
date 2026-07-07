#include "oled.h"
#include "main.h"
#include "Type.h"
#include "tim.h"
#include "menu.h"
extern ElectricData_t electricData;
ElectricData_t displayData;
/*=========================================================
 * MenuTask
 =========================================================*/
void MenuTask(void *argument)
{
    /* 保存当前任务句柄 */
    menuHandle = xTaskGetCurrentTaskHandle();

    for(;;)
    {    
       xSemaphoreTake(ElectricDataMutexHandle,portMAX_DELAY);

        displayData = electricData;

        xSemaphoreGive( ElectricDataMutexHandle);
        /* 编码器处理 */
        UI_EncoderProcess();

        /* 页面切换 */
        UI_SwitchProcess();

        /* OLED刷新 */
        UI_DisplayProcess();

        /* 固定刷新周期 */
        vTaskDelay(pdMS_TO_TICKS(50));

    }
}
