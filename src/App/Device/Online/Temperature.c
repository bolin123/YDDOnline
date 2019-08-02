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
    TEMP_18B20_DQ_OUTPUT();             //SET PG11 OUTPUT
    TEMP_18B20_DQ_SET_LEVEL(0);         //拉低DQ
    HalWaitUs(500);                     //拉低 > 480us 
    TEMP_18B20_DQ_INPUT();              //SET PG11 INPUT    
    HalWaitUs(5);
    while (TEMP_18B20_DQ_GET_LEVEL() && retry < 60)//wait 15~60us
    {
        retry++;
        HalWaitUs(1);
    }       
    if(retry >= 65)
    {
        return -1;
    }

    retry=0;
    while (!TEMP_18B20_DQ_GET_LEVEL() && retry < 250)//wait 60~240
    {
        retry++;
        HalWaitUs(1);
    }
    if(retry >= 250)
    {
        return -1;
    }            
    HalWaitUs(5);
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
    /*
    * 所有的读时隙必须至少有60us的持续时间
    * 相邻两个读时隙必须要有最少1us的恢复时间
    * 所有的读时隙都由拉低总线，持续至少1us后再释放总线
    */
#if 1
    for(mask = 0x01; mask != 0; mask <<= 1) 
    {
        TEMP_18B20_DQ_OUTPUT();
        TEMP_18B20_DQ_SET_LEVEL(0); 
        HalWaitUs(1);  //t1 > 1us, 至少1us后再释放总线(9.6)
        TEMP_18B20_DQ_INPUT();
        //master sample init, do nothing
        HalWaitUs(10);  //t2, t1 + t2 <= 15us (18.6)

        //等待数据稳定
        //HalWaitUs(2); //t3
        if (TEMP_18B20_DQ_GET_LEVEL())
        {
			data |= mask;
        }
		else
		{
			data  &= ~mask;
		}
		HalWaitUs(35); //t4 + t3 >= 45us 
    }
#else
    for(mask = 0x01; mask != 0; mask <<= 1) 
    {
        TEMP_18B20_DQ_OUTPUT();
        TEMP_18B20_DQ_SET_LEVEL(1); 
        HalWaitUs(5);
        TEMP_18B20_DQ_SET_LEVEL(0); 
        HalWaitUs(5);
        TEMP_18B20_DQ_SET_LEVEL(1); 
        HalWaitUs(5);
        TEMP_18B20_DQ_INPUT();
        HalWaitUs(1); 
        if (TEMP_18B20_DQ_GET_LEVEL())
			data |= mask;
		else	
			data  &= ~mask;
		TEMP_18B20_DQ_OUTPUT();
        TEMP_18B20_DQ_SET_LEVEL(1); 
		HalWaitUs(56);
    }
#endif
    return data;
}

static void tempWriteByte(uint8_t cmd)
{             
    uint8_t i;
    /*
    * 写时隙必须有最少60us的持续时间(60~120)
    * 写1时隙，在拉低总线后主机必须在15μs内释放总线|_<15_|
    * 写0时隙，在拉低总线后主机必须继续拉低总线以满足时隙持续时间的要求(至少60μs)
    * 相邻两个写时隙必须要有最少1us的恢复时间
    */
    TEMP_18B20_DQ_OUTPUT();
    for (i = 0; i < 8; i++) 
    {
        TEMP_18B20_DQ_SET_LEVEL(1); //idle
        HalWaitUs(1);               //最少1us的恢复时间
        TEMP_18B20_DQ_SET_LEVEL(0); //start
        HalWaitUs(3);              //t1 < 15us    
        TEMP_18B20_DQ_SET_LEVEL(cmd & 0x01);
        HalWaitUs(55);              //t2, t1+t2 >= 60us,
        cmd = cmd >> 1;
    }
    TEMP_18B20_DQ_SET_LEVEL(1); //idle
}

uint16_t TemperatureGetValue(void)
{
#if 1
    uint8_t tl;
    uint16_t value = 0;
//    int ret1, ret2;
    
    tempReset();
    tempWriteByte(0xcc);        // skip rom
    tempWriteByte(0x44);        // convert
    HalWaitUs(CONVERT_T);
    //HalWaitMs(1);
    tempReset();
    tempWriteByte(0xcc);        // skip rom
    tempWriteByte(0xbe);        // convert            
    tl = tempReadByte();         // LSB   
    value = tempReadByte();         // MSB  
    value = (value << 8) + tl;
    //printf("ret1=%d, ret2=%d\n", ret1, ret2);
    return value;
    #else
    uint8_t tl;
    tempReadByte();
    //tempWriteByte(0xcc);
    //tempWriteByte(0xbe);        // convert            
    //tl = tempReadByte();
    return 0;
    
#endif
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

void TemperaturePowerOn(void)
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

    //tempWriteByte(0x44);                        //启动一次温度转换
    TemperatureGetValue();
    TemperatureGetValue();
    TemperatureGetValue();
}



