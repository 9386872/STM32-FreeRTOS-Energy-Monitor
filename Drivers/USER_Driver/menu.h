#ifndef __MENU_H__
#define __MENU_H__


static void ShowVolt(uint8_t x,uint8_t y,float value);
static void ShowCurrent(uint8_t x,uint8_t y,float value);
static void Menu_Draw(void);
static void Draw_Interface1_UI(void);
static void Draw_Interface2_UI(void);
static void Draw_Interface3_UI(void);
static void Draw_Interface4_UI(void);
static void Update_Interface1_Value(void);
static void Update_Interface2_Value(void);
static void Update_Interface3_Value(void);
static void Update_Interface4_Value(void);
void UI_KeyProcess(void);
void UI_EncoderProcess(void);
void UI_SwitchProcess(void);
void UI_DisplayProcess(void);
static void UI_MainRefresh(void);
static void UI_SubRefresh(void);

#endif 
