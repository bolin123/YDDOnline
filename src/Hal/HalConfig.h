#ifndef HAL_CONFIG_H
#define HAL_CONFIG_H


#define HAL_EXCEPTION_ID_RTC     0x0001
#define HAL_EXCEPTION_ID_EXFLASH 0x0002

#define HAL_FLASH_INVALID_ADDR 0xffffffff

//#define HAL_DEVICE_DATA_PACKET_LENGTH (6 + 8)

typedef enum
{
    HAL_SENSOR_ID_GEOMAGNETISM = 0, //地磁
    HAL_SENSOR_ID_NOISE,            //地音
    HAL_SENSOR_ID_PRESS1,           //应力1
    HAL_SENSOR_ID_PRESS2,           //应力2
    HAL_SENSOR_ID_COUNT,
}HalSensorID_t;

typedef enum
{
    HAL_DEVICE_TYPE_MASTER = 0, //主机
    HAL_DEVICE_TYPE_PRESS,      //应力
    HAL_DEVICE_TYPE_GEO,        //地音地磁
}HalDeviceType_t;
    
#define HAL_DEVICE_TYPE HAL_DEVICE_TYPE_PRESS

#pragma pack(1)
typedef struct
{
    uint8_t address;
    uint8_t type;
    uint8_t errcode;
    uint8_t power;
    uint32_t utc;
    struct
    {
        uint16_t temperate;  //温度
        
    #if (HAL_DEVICE_TYPE == 1)
        uint16_t press1;     //应力1值
        uint16_t press2;     //应力2值
    #else
        uint16_t geoAmplitue;   //电磁强度
        uint16_t geoFrequency;  //电磁脉冲数
        uint16_t geoEvent;      //电磁事件数
        uint16_t geoEnergy;     //电磁能量值
        uint16_t noiseAmplitue; //地音强度
        uint16_t noiseFrequency;//地音振铃数
        uint16_t noiseEvent;    //地音事件数
        uint16_t noiseEnergy;   //地音能量值
    #endif
    }data;
}HalDeviceDataStorage_t;
#pragma pack()

#define HAL_DEVICE_DATA_PACKET_LENGTH sizeof(HalDeviceDataStorage_t)


#if defined(HAL_OLD_DEVICE)

#define HAL_IO_UART_PIN  0x3c  //模拟串口IO PD12
#define HAL_STATUS_LED_PIN     0x41  //PE1
#define HAL_STATUS_LED_ENABLE_LEVEL  0
#define HAL_STATUS_LED_DISABLE_LEVEL 1

#define HAL_SENSORS_POWER_PIN 0x2d //PC13
#define HAL_SENSORS_POWER_ENABLE_LEVEL  1
#define HAL_SENSORS_POWER_DISABLE_LEVEL 0

#define HAL_485_POWER_PIN     0x0c //PA12
#define HAL_485_POWER_ENABLE_LEVEL  0
#define HAL_485_POWER_DISABLE_LEVEL 1

#define HAL_IR_POWER_PIN      0x42
#define HAL_IR_POWER_ENABLE_LEVEL  0
#define HAL_IR_POWER_DISABLE_LEVEL 1

#define HAL_IR_INPUT_PIN      0x43  //pe3

#define HAL_LIGHT_IRQ_PIN    0x46 //pe6
#define HAL_LIGHT_ACTIVE_LEVEL 0

#define HAL_TEMP_18B20_DQ_PIN 0x3b //pd11
#define HAL_WIRED_PROTO_485DE_PIN 0x0b //pa11
#define HAL_IR_TX_EN_PIN 0x45  //pe5

#else //new one

#define HAL_IO_UART_PIN  0x21  //模拟串口IO Pc1
#define HAL_STATUS_LED_PIN     0x41  //PE1
#define HAL_STATUS_LED_ENABLE_LEVEL  0
#define HAL_STATUS_LED_DISABLE_LEVEL 1

#define HAL_SENSORS_POWER_PIN 0x02 //PA2
#define HAL_SENSORS_POWER_ENABLE_LEVEL  1
#define HAL_SENSORS_POWER_DISABLE_LEVEL 0

#define HAL_485_POWER_PIN     0x3f //Pd15
#define HAL_485_POWER_ENABLE_LEVEL  0
#define HAL_485_POWER_DISABLE_LEVEL 1

#define HAL_IR_POWER_PIN      0x0c //pa12
#define HAL_IR_POWER_ENABLE_LEVEL  0
#define HAL_IR_POWER_DISABLE_LEVEL 1

#define HAL_IR_INPUT_PIN      0x30  //pd0

#define HAL_LIGHT_IRQ_PIN    0x26 //pc6
#define HAL_LIGHT_ACTIVE_LEVEL 0

#define HAL_TEMP_18B20_DQ_PIN 0x03 //pa3
#define HAL_WIRED_PROTO_485DE_PIN 0x0b //pa11
#define HAL_IR_TX_EN_PIN 0x27  //pc7

#endif

//#define HAL_ADC_CH_NUM 4
#define HAL_DAC_BASE_VALUE 1830 //1.51, 1.515v = 1892
#define HAL_DAC_STEP_VALUE 62   //0.05v
#define HAL_RF_CHANNEL_NUM 29 

#define KB(x) ((x)*1024)
/*flash分区
* 0 ~ 5k     :boot
* 5k ~ 125k  :app
* 125k ~ 245k:ota
* 245k ~ 256k:args
*/
#define HAL_FLASH_SIZE (KB(256))  //256KB 120k(ota), 4k(boot) 4+240+12
#define HAL_FLASH_PAGE_SIZE (KB(2))
#define HAL_FLASH_OTA_SIZE (KB(120))

#define HAL_FLASH_BASE_ADDR  0x8000000
#define HAL_BOOT_FLASH_ADDR  (HAL_FLASH_BASE_ADDR + 0)
#define HAL_APP_FLASH_ADDR   (HAL_FLASH_BASE_ADDR + KB(4))
#define HAL_OTA_FLASH_ADDR   (HAL_FLASH_BASE_ADDR + KB(124))
#define HAL_ARGS_FLASH_ADDR  (HAL_FLASH_BASE_ADDR + KB(230))

#define HAL_DEVICE_ARGS_ADDR (HAL_ARGS_FLASH_ADDR + 0) //设备参数
#define HAL_DATA_RECORD_ADDR (HAL_ARGS_FLASH_ADDR + KB(2)) //总数记录
#define HAL_DATA_POINT_INFO_ADDR (HAL_ARGS_FLASH_ADDR + KB(4)) //点位信息

#define HAL_DATA_POINT_INFO_PAGE 11 //11 page
#endif

