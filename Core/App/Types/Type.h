#ifndef BUTTONTYPE_H
#define BUTTONTYPE_H

#include <stdint.h>
typedef enum
{
    ENC_NONE = 0,
    ENC_CW,     // 顺时针 +1
    ENC_CCW     // 逆时针 -1
} EncoderEvent_t;

typedef enum
{
    MAIN_INTERFACE = 0,
    INTERFACE1,
    INTERFACE2,
    INTERFACE3,
    INTERFACE4
} UI_State;

typedef enum
{
    KEY_EVENT_NONE = 0,

    KEY_EVENT_UI,
    KEY_EVENT_JDQ1,
    KEY_EVENT_JDQ2,


}KeyEvent_t;

typedef struct
{
     /* ADC采集 */
    float gf_volt;          // 光伏电压
    float fj_volt;          // 风机电压

    /* INA226采集 */
    float batteryVolt;      // 电池电压
    float batteryCurrent;   // 电池电流

    float ydtd_Volt;        // 负载电压
    float ydtd_Current;     // 负载电流

    /* SOC */
    float soc;              // 电池剩余电量(%)


} ElectricData_t;



#endif /* BUTTONTYPE_H */