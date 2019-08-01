#include "Temperature.h"
#include "Sys.h"

#define TEMP_18B20_DQ_PIN 0x3b //pd11
#define TEMP_18B20_DQ_INPUT() HalGPIOConfig(TEMP_18B20_DQ_PIN, HAL_IO_INPUT)
#define TEMP_18B20_DQ_OUTPUT() HalGPIOConfig(TEMP_18B20_DQ_PIN, HAL_IO_OUTPUT)
#define TEMP_18B20_DQ_SET_LEVEL(x) HalGPIOSetLevel(TEMP_18B20_DQ_PIN, x)
#define TEMP_18B20_DQ_GET_LEVEL() HalGPIOGetLevel(TEMP_18B20_DQ_PIN)


static int tempReset(void)
{
    uint8_t retry = 0;
    TEMP_18B20_DQ_OUTPUT();         //SET PG11 OUTPUT
    //TEMP_18B20_DQ_SET_LEVEL(1);         //DQ=1 
    //HalWaitUs(700);
    TEMP_18B20_DQ_SET_LEVEL(0);         //拉低DQ
    HalWaitUs(500);            //拉低480us
    TEMP_18B20_DQ_SET_LEVEL(1);         //DQ=1 
    HalWaitUs(1);             //15US

    TEMP_18B20_DQ_INPUT();        //SET PG11 INPUT         
    while (TEMP_18B20_DQ_GET_LEVEL() && retry < 200)
    {
        retry++;
        HalWaitUs(1);
    }       
    if(retry >= 200)
    {
        return -1;
    }

    retry=0;
    while (!TEMP_18B20_DQ_GET_LEVEL() && retry < 240)
    {
        retry++;
        HalWaitUs(1);
    }
    if(retry >= 240)
    {
        return -1;
    }            
    return 0;
}
/*
static uint8_t readBit(void)
{
    uint8_t data;
    TEMP_18B20_DQ_OUTPUT();        //SET PG11 OUTPUT
    TEMP_18B20_DQ_SET_LEVEL(0); 
    HalWaitUs(2);
    //TEMP_18B20_DQ_SET_LEVEL(1); 
    TEMP_18B20_DQ_INPUT();        //SET PG11 INPUT
    HalWaitUs(3); 
    data = TEMP_18B20_DQ_GET_LEVEL();
    HalWaitUs(56);           
    return data;
}
*/
static uint8_t tempReadByte(void)
{
    uint8_t mask;
    uint8_t data = 0x00;

    for(mask = 0x01; mask != 0; mask <<= 1) 
    {
        TEMP_18B20_DQ_OUTPUT();
        TEMP_18B20_DQ_SET_LEVEL(0); 
        HalWaitUs(2);
        TEMP_18B20_DQ_INPUT();
        HalWaitUs(3); 
        if (TEMP_18B20_DQ_GET_LEVEL())
			data |= mask;
		else	
			data  &= ~mask;
		HalWaitUs(56);
    }
    return data;
}

static void tempWriteByte(uint8_t cmd)
{             
    uint8_t i;
    TEMP_18B20_DQ_OUTPUT();        //SET PG11 OUTPUT;
    for (i = 0; i < 8; i++) 
    {
        TEMP_18B20_DQ_SET_LEVEL(1);
        HalWaitUs(1);   
        TEMP_18B20_DQ_SET_LEVEL(0);        // Write 1
        HalWaitUs(2);                            
        TEMP_18B20_DQ_SET_LEVEL(cmd & 0x01);
        HalWaitUs(58); 
        cmd = cmd >> 1;
    }
    TEMP_18B20_DQ_SET_LEVEL(1);
}

uint16_t TemperatureGetValue(void)
{
    int ret1, ret2;
    uint8_t tl;
    uint16_t value = 0;
    
    ret1 = tempReset();
    tempWriteByte(0xcc);        // skip rom
    tempWriteByte(0x44);        // convert
    HalWaitUs(CONVERT_T);
    ret2 = tempReset();
    tempWriteByte(0xcc);        // skip rom
    tempWriteByte(0xbe);        // convert            
    tl = tempReadByte();         // LSB   
    value = tempReadByte();         // MSB  
    value = (value << 8) + tl;
    //Syslog("value = %d", value);
    //printf("ret1=%d, ret2 = %d\n", ret1, ret2);
    return value;
}

float TemperatureValueExchange(uint16_t temp)
{
	float dat;
	
	// 获取温度的实际数值，不包含符号位
	dat = (temp >> 4) & 0x7F;                         //提取整数部分
	dat += (float)(temp&0x0F) / 16;                 //提取小数部分
	// 判断温度的符号
	if (0 != (temp & 0xF800))   //判断符号为，全为1表示零下温度值
	{                      
		return -dat;
	} 
	
	return dat;
}

void TemperatureInit(void)
{
    HalGPIOConfig(TEMP_18B20_DQ_PIN, HAL_IO_OUTPUT);
    HalGPIOSetLevel(TEMP_18B20_DQ_PIN, 0);
    tempReset();
    tempWriteByte(0xCC);                        //跳过ROM
    // 设置配置寄存器，精确到9Bit即0.5C'
    tempWriteByte(0x4E);                        //设置暂存器指令
    tempWriteByte(0xFF);                        //TH
    tempWriteByte(0xFF);                        //TL
    tempWriteByte(ACCURACY);                        //config寄存器

    tempWriteByte(0x44);                        //启动一次温度转换
}

void TemperaturePoll(void)
{
    static uint32_t oldtime = 0;

    if(SysTimeHasPast(oldtime, 1000))
    {
        Syslog("value = %.1f", TemperatureValueExchange(TemperatureGetValue()));
        oldtime = SysTime();
    }
}

