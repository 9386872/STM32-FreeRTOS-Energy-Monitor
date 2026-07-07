#include "Type.h"
#include "main.h"
#include "menu.h"



/*=========================================
 * EXTI回调
 *=========================================*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /*=====================================
     * UI按键
     *=====================================*/
    if(GPIO_Pin == GPIO_PIN_2)
    {
        KeyEvent_t event = KEY_EVENT_UI;

        xQueueSendFromISR(KeyQueueHandle, &event,&xHigherPriorityTaskWoken);
    }

    /*=====================================
     * 继电器1
     *=====================================*/
    else if(GPIO_Pin == GPIO_PIN_4)
    {
        KeyEvent_t event = KEY_EVENT_JDQ1;

        xQueueSendFromISR(KeyQueueHandle,&event,&xHigherPriorityTaskWoken);
    }

    /*=====================================
     * 继电器2
     *=====================================*/
    else if(GPIO_Pin == GPIO_PIN_7)
    {
        KeyEvent_t event = KEY_EVENT_JDQ2;

        xQueueSendFromISR(KeyQueueHandle,&event,&xHigherPriorityTaskWoken);
    }

    /*=====================================
     * 编码器A
     *=====================================*/
    else if(GPIO_Pin == GPIO_PIN_0)
    {
        EncoderEvent_t event;

        if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_1) == GPIO_PIN_RESET)
        {
            event = ENC_CCW;

            xQueueSendFromISR(EncoderQueueHandle,&event,&xHigherPriorityTaskWoken);
        }
    }

    /*=====================================
     * 编码器B
     *=====================================*/
    else if(GPIO_Pin == GPIO_PIN_1)
    {
        EncoderEvent_t event;

        if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            event = ENC_CW;

            xQueueSendFromISR(EncoderQueueHandle,&event,&xHigherPriorityTaskWoken);
        }
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void ButtonTask(void *argument)
{
    KeyEvent_t event;

    static uint32_t lastKeyTick = 0;

    for(;;)
    {
        if(xQueueReceive(KeyQueueHandle, &event,portMAX_DELAY) == pdPASS)
        {
            /*=================================
             * 软件消抖
             *=================================*/
            if(HAL_GetTick() - lastKeyTick < 150)
            {
                continue;
            }

            lastKeyTick = HAL_GetTick();

            switch(event)
            {
                /*=============================
                 * UI按键
                 *=============================*/
                case KEY_EVENT_UI:

                    UI_KeyProcess();

                    break;

                /*=============================
                 * 继电器1
                 *=============================*/
                case KEY_EVENT_JDQ1:

                    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
                    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

                    break;

                /*=============================
                 * 继电器2
                 *=============================*/
                case KEY_EVENT_JDQ2:

                    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_4);

                    HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_3);

                    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);

                    HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);

                    break;

                default:

                    break;
            }
        }
    }
}