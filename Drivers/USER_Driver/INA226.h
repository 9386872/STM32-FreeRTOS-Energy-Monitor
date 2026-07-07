#ifndef __INA226_H__
#define __INA226_H__


void INA226_init(void);
//仅使用了获取电压电流功率3个功能
float INA226_GetBusV1(void);
float INA226_GetCurrent1(void);
float INA226_GetPower1(void);
float INA226_GetBusV2(void);
float INA226_GetCurrent2(void);
float INA226_GetPower2(void);
#endif /* INC_IIC_INA226_H_ */



