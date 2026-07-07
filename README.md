# 基于 FreeRTOS 的风光储电能状态采集系统

## 项目简介

本项目基于 STM32F103 微控制器设计并实现了一套风光储电能状态采集系统，用于风机、光伏、蓄电池及负载等多能源设备的运行状态监测。

系统采用 FreeRTOS 实现多任务调度，完成电压、电流采集、SOC 电量估算、OLED 本地显示以及物联网平台远程监控等功能，实现新能源系统的数据采集与远程管理。


---

## 系统功能

- 光伏电压实时采集
- 风机整流电压采集
- 蓄电池电压、电流采集
- 负载电压、电流采集
- SOC 电量估算
- OLED 多级菜单显示
- 风机、光伏继电器控制
- MQTT 数据上传物联网平台
- 远程控制风机及光伏接入状态

---

## 🛠 硬件设计

本系统围绕风光储电能监测需求设计了多路信号采集与控制电路，主要包括：

- **光伏电压采集电路**：设计电压分压采样电路，将光伏输出电压转换至 STM32 ADC 可采集范围，实现光伏电压检测。
- **风机电压采集电路**：设计三相整流及滤波电路，将风力发电机交流输出转换为直流电压，并通过电压采样电路完成检

- 
## 软件架构

系统采用 FreeRTOS 多任务设计，各任务职责如下：

| Task | 功能 |
|------|------|
| CollectionTask | 电压、电流数据采集、电池 SOC 电量估算 |
| MenuTask | OLED 菜单显示 |
| ButtonTask | 按键事件处理、编码器事件处理、继电器控制 |
| EspTask | MQTT 通信与远程数据上传 |

各任务之间采用 **Mutex** 完成共享数据同步，保证数据一致性。

---

## 系统设计

### 电能参数采集

系统通过 ADC 采集光伏及风机电压。

采用 INA226 完成蓄电池及负载电压、电流检测，并通过 I2C 完成数据读取。

实现：

- 光伏电压采集
- 风机整流电压采集
- 电池电压采集
- 电池电流采集
- 负载电压采集
- 负载电流采集

---

### SOC 电量估算

系统采用

- 开路电压法（OCV）
- 安时积分法（Coulomb Counting）

融合计算电池 SOC，提高剩余电量估算精度。

---

### 安全保护

系统根据采集数据判断运行状态：

- 风机超速保护
- 蓄电池过充保护
- 光伏接入控制
- 风机接入控制

当出现异常时，通过继电器自动切断对应回路，实现系统保护。

同时支持本地按键控制及远程控制。

---

### 人机交互

采用 OLED 实现多级菜单：

- 光伏参数
- 风机参数
- 电池参数
- 负载参数
- SOC 状态

通过按键完成菜单切换与功能选择。

---

### 物联网通信

系统通过 UART 与通信模块进行 AT 指令通信。

主要功能包括：

- WiFi/4G 网络接入
- MQTT 建立连接
- 周期上传采集数据
- 接收远程控制命令
- 控制继电器状态

实现设备远程监测与控制。

---


## 开发环境

- STM32CubeMX
- VSCode
- CMake
- STM32 HAL Library
- FreeRTOS

---

## 项目亮点

- 基于 FreeRTOS 实现多任务实时调度
- 使用 Mutex 完成多任务数据同步
- 基于 INA226 实现高精度电流检测
- OCV + 安时积分融合 SOC 算法
- MQTT 接入物联网平台实现远程监控
- OLED 多级菜单设计
- 模块化软件架构，便于功能扩展

---

## 运行效果


- 系统实物图
<img width="384" height="636" alt="image" src="https://github.com/user-attachments/assets/945a5460-0359-4720-9a40-6bb0da4541a1" />

- OLED 显示界面
- <img width="309" height="291" alt="image" src="https://github.com/user-attachments/assets/b840ca62-d4db-4f4a-a8cd-2fe776f5ca84" />

- 电压电流采集效果
- <img width="337" height="270" alt="image" src="https://github.com/user-attachments/assets/ac1ce31f-a9e6-4ecf-8c6d-e5fa04aec71e" />

- 物联网平台设备在线截图
- <img width="974" height="413" alt="image" src="https://github.com/user-attachments/assets/cac035fb-1245-43fa-9c2b-d2aae85b48ec" />

- 系统框图
- <img width="717" height="609" alt="image" src="https://github.com/user-attachments/assets/3bb770b7-116f-464b-aa4e-163d9fefbe42" />

---

## 注意事项

由于涉及实际设备及物联网平台，本仓库已删除以下敏感信息：

- WiFi SSID
- WiFi Password
- MQTT Device ID
- MQTT ClientID
- MQTT Password
- 服务器地址

使用时请根据实际平台自行配置。

---

## License

仅供学习与交流使用。
