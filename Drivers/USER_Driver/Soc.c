#include "soc.h"
#include <math.h>
#include "main.h"
/* =========================
 * OCV-SOC查表
 * ========================= */
static const OCV_SOC_Entry ocv_soc_lut[] =
{
    {25.16f, 0.0f},   {25.744f, 10.0f},
    {26.048f, 20.0f}, {26.28f, 30.0f},
    {26.312f, 40.0f}, {26.328f, 50.0f},
    {26.424f, 60.0f}, {26.616f, 70.0f},
    {26.624f, 80.0f}, {26.632f, 90.0f},
    {26.96f, 100.0f}
};

#define LUT_SIZE (sizeof(ocv_soc_lut)/sizeof(OCV_SOC_Entry))

/* 电池参数 */
#define BATTERY_CAPACITY_AH     30.0f

/* 静置判断电流（mA） */
#define REST_CURRENT_MA         100.0f

/* 电压保护 */
#define FULL_VOLTAGE            26.9f
#define EMPTY_VOLTAGE           25.2f

/* =========================
 * 内部状态
 * ========================= */
static float fRemainingCapacity_Ah = 0.0f;
static float fSocDynamic = 0.0f;
static uint8_t soc_initialized = 0;

/* =========================
 * OCV估算
 * ========================= */
static float estimate_soc_from_ocv(float ocv)
{
    if (ocv <= ocv_soc_lut[0].ocv)
        return 0.0f;

    if (ocv >= ocv_soc_lut[LUT_SIZE - 1].ocv)
        return 100.0f;

    for (uint8_t i = 0; i < LUT_SIZE - 1; i++)
    {
        if (ocv >= ocv_soc_lut[i].ocv &&
            ocv < ocv_soc_lut[i + 1].ocv)
        {
            float dv = ocv_soc_lut[i + 1].ocv - ocv_soc_lut[i].ocv;
            float ds = ocv_soc_lut[i + 1].soc - ocv_soc_lut[i].soc;

            return ocv_soc_lut[i].soc +
                   (ocv - ocv_soc_lut[i].ocv) * ds / dv;
        }
    }

    return 0.0f;
}

/* =========================
 * 初始化
 * ========================= */
void soc_init(float battery_voltage)
{
    float soc = estimate_soc_from_ocv(battery_voltage);

    if (soc < 0.0f) soc = 0.0f;
    if (soc > 100.0f) soc = 100.0f;

    fSocDynamic = soc;

    fRemainingCapacity_Ah =
        BATTERY_CAPACITY_AH * soc / 100.0f;

    soc_initialized = 1;
}

/* =========================
 * 更新SOC
 * ========================= */
void update_soc(float battery_voltage,float current_ma,float sample_ms)
{
    if (!soc_initialized)
    {
        soc_init(battery_voltage);
        return;
    }

    float current_A = current_ma / 1000.0f;
    float delta_h = sample_ms / 3600000.0f;

    /* =========================
     * 安时积分
     * 放电为正 → SOC下降
     * ========================= */
    fRemainingCapacity_Ah -= current_A * delta_h;

    /* 限幅 */
    if (fRemainingCapacity_Ah < 0.0f)
        fRemainingCapacity_Ah = 0.0f;

    if (fRemainingCapacity_Ah > BATTERY_CAPACITY_AH)
        fRemainingCapacity_Ah = BATTERY_CAPACITY_AH;

    /* SOC计算 */
    fSocDynamic =
        (fRemainingCapacity_Ah / BATTERY_CAPACITY_AH) * 100.0f;

    /* =========================
     * 满电校准
     * ========================= */
    if (battery_voltage >= FULL_VOLTAGE)
    {
        fRemainingCapacity_Ah = BATTERY_CAPACITY_AH;
        fSocDynamic = 100.0f;
    }

    /* =========================
     * 低电压校准
     * ========================= */
    if (battery_voltage <= EMPTY_VOLTAGE)
    {
        fRemainingCapacity_Ah = 0.0f;
        fSocDynamic = 0.0f;
    }

    /* =========================
     * 静置OCV修正
     * ========================= */
    if (fabs(current_ma) < REST_CURRENT_MA)
    {
        float ocv_soc = estimate_soc_from_ocv(battery_voltage);

        /* 小幅修正（避免跳变） */
        fSocDynamic =
            0.9f * fSocDynamic +
            0.1f * ocv_soc;

        if (fSocDynamic > 100.0f) fSocDynamic = 100.0f;
        if (fSocDynamic < 0.0f)   fSocDynamic = 0.0f;

        fRemainingCapacity_Ah =
            BATTERY_CAPACITY_AH * fSocDynamic / 100.0f;
    }
}

/* =========================
 * 获取SOC
 * ========================= */
float get_current_soc(void)
{
    return fSocDynamic;
}