#include "oled.h"
#include "stdlib.h" 	 
#include "main.h"
#include "Type.h"
#include "menu.h"


/* =========================================
 * 全局变量
 * ========================================= */
int16_t Num = 1;
uint8_t icon = 0;
extern ElectricData_t displayData;
UI_State currentUI = MAIN_INTERFACE;
TaskHandle_t MenuTaskHandle = NULL;

/* 闪烁标志 */
uint8_t blink = 0;

static void ShowVolt(uint8_t x,uint8_t y,float value)
{
    uint16_t integer = (uint16_t)value;
    uint16_t decimal = (uint16_t)(value * 100) % 100;

    OLED_ShowNum(x,y,integer,2,16,1);

    OLED_ShowChar(x+16,y,'.',16,1);

    OLED_ShowNum(x+24,y,decimal,2,16,1);

    OLED_ShowChar(x+40,y,'V',16,1);
}

/* =========================================
 * 通用电流显示
 * ========================================= */
static void ShowCurrent(uint8_t x,uint8_t y,float value)
{
    uint16_t integer = (uint16_t)value;
    uint16_t decimal = (uint16_t)(value * 100) % 100;

    OLED_ShowNum(x,y,integer,2,16,1);

    OLED_ShowChar(x+16,y,'.',16,1);

    OLED_ShowNum(x+24,y,decimal,2,16,1);

    OLED_ShowChar(x+40,y,'A',16,1);
}

static void Menu_Draw(void)
{
    /* 四个图标 */

    OLED_ShowPicture(0, 32, 32, 32, BMP1,1);
    OLED_ShowPicture(32, 32, 32,32,BMP2, 1);
    OLED_ShowPicture(64, 32,32,32,BMP3,1);
    OLED_ShowPicture(96, 32, 32, 32, BMP4,1);

    /* 闪烁选中 */

    if(Num != 1)
    {
        if(blink == 0)
        {
            switch(icon)
            {
                case 1:

                    OLED_ShowPicture(0, 32, 32, 32,BMP5,1);

                    break;

                case 2:

                    OLED_ShowPicture(32,32,32,32,BMP5,1);

                    break;

                case 3:

                    OLED_ShowPicture(64,32,32, 32,BMP5, 1);

                    break;

                case 4:

                    OLED_ShowPicture(96,32,32, 32,BMP5,1);

                    break;
            }
        }
    }

    /* 显示编号 */

    OLED_ShowNum(88, 0,Num, 2, 16, 1);
}

/*=========================================================
 * 子界面UI
 * 只画一次固定文字
 =========================================================*/

static void Draw_Interface1_UI(void)
{
    OLED_Clear();

    OLED_ShowString(0,32, (uint8_t *)"PV:",  16, 1);

}

static void Draw_Interface2_UI(void)
{
    OLED_Clear();

    OLED_ShowString(0, 32, (uint8_t *)"FAN:", 16, 1);

}

static void Draw_Interface3_UI(void)
{
    OLED_Clear();

    OLED_ShowString(0, 32, (uint8_t *)"BAT V:", 16,  1);

    OLED_ShowString(0, 48,(uint8_t *)"BAT I:", 16, 1);

    OLED_ShowString(0, 16, (uint8_t *)"SOC:", 16, 1);

}

static void Draw_Interface4_UI(void)
{
    OLED_Clear();

    OLED_ShowString(0, 32, (uint8_t *)"LOAD V:", 16,1);

    OLED_ShowString(0, 48, (uint8_t *)"LOAD I:", 16, 1);

}

/*=========================================================
 * 子界面数值刷新
 * 这里只更新数字
 =========================================================*/

static void Update_Interface1_Value(void)
{
    ShowVolt(32, 32,displayData.gf_volt);

}

static void Update_Interface2_Value(void)
{
    ShowVolt(40, 32, displayData.fj_volt);

}

static void Update_Interface3_Value(void)
{
    ShowVolt(56, 32, displayData.batteryVolt);

    ShowCurrent(56, 48,displayData.batteryVolt);

    OLED_ShowNum(56, 16, displayData.soc , 2, 16, 1);

}

static void Update_Interface4_Value(void)
{
    ShowVolt(64, 32,  displayData.ydtd_Volt);

    ShowCurrent(64, 48, displayData.ydtd_Current);

}

/*=========================================================
 * 编码器处理
 =========================================================*/
 void UI_EncoderProcess(void)
{
    EncoderEvent_t event;

    while(xQueueReceive(EncoderQueueHandle,&event,0) == pdPASS)
    {
        if(event == ENC_CW)
        {
            Num++;
        }
        else if(event == ENC_CCW)
        {
            Num--;
        }

        /* 限幅 */
        if(Num < 1)
        {
            Num = 13;
        }

        if(Num > 13)
        {
            Num = 1;
        }

        /* 图标映射 */
        if(Num == 1)
        {
            icon = 0;
        }
        else if(Num <= 4)
        {
            icon = 1;
        }
        else if(Num <= 7)
        {
            icon = 2;
        }
        else if(Num <= 10)
        {
            icon = 3;
        }
        else
        {
            icon = 4;
        }
    }
}

void UI_KeyProcess(void)
{
    switch(currentUI)
    {
        case MAIN_INTERFACE:

            if(Num > 1 && Num <= 4)
                currentUI = INTERFACE1;

            else if(Num > 4 && Num <= 7)
                currentUI = INTERFACE2;

            else if(Num > 7 && Num <= 10)
                currentUI = INTERFACE3;

            else if(Num > 10 && Num <= 13)
                currentUI = INTERFACE4;

            break;

        default:

            Num = 1;

            currentUI = MAIN_INTERFACE;

            break;
    }
}


/*=========================================================
 * 页面切换
 =========================================================*/
void UI_SwitchProcess(void)
{
    static UI_State lastUI = MAIN_INTERFACE;

    if(lastUI != currentUI)
    {
        switch(currentUI)
        {
            case MAIN_INTERFACE:

                OLED_Clear();

                Menu_Draw();


                break;

            case INTERFACE1:

                Draw_Interface1_UI();

                break;

            case INTERFACE2:

                Draw_Interface2_UI();

                break;

            case INTERFACE3:

                Draw_Interface3_UI();

                break;

            case INTERFACE4:

                Draw_Interface4_UI();

                break;
        }

        lastUI = currentUI;
    }
}

/*=========================================================
 * OLED显示刷新
 =========================================================*/
void UI_DisplayProcess(void)
{
    if(currentUI == MAIN_INTERFACE)
    {
        UI_MainRefresh();
    }
    else
    {
        UI_SubRefresh();
    }
    OLED_Refresh();
}

/*=========================================================
 * 主界面刷新
 =========================================================*/
static void UI_MainRefresh(void)
{
    Menu_Draw();

}

/*=========================================================
 * 子界面刷新
 =========================================================*/
static void UI_SubRefresh(void)
{
    switch(currentUI)
    {
        case INTERFACE1:

            Update_Interface1_Value();

            break;

        case INTERFACE2:

            Update_Interface2_Value();

            break;

        case INTERFACE3:

            Update_Interface3_Value();

            break;

        case INTERFACE4:

            Update_Interface4_Value();

            break;

        default:

            break;
    }
}