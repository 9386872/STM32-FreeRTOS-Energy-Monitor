#ifndef __SOC_H
#define __SOC_H	 
typedef struct {
    float ocv;      // 开路电压（单位：V）
    float soc;      // 对应荷电状态（单位：%）
} OCV_SOC_Entry;

/* 初始化 */
void soc_init(float battery_voltage);

/* 更新SOC */
void update_soc(float battery_voltage,
                float current_ma,
                float sample_ms);

/* 获取SOC */
float get_current_soc(void);
#endif




