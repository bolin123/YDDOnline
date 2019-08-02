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
    TEMP_18B20_DQ_SET_LEVEL(0);         //����DQ
    HalWaitUs(500);                     //���� > 480us 
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
    * ���еĶ�ʱ϶����������60us�ĳ���ʱ��
    * ����������ʱ϶����Ҫ������1us�Ļָ�ʱ��
    * ���еĶ�ʱ϶�����������ߣ���������1us�����ͷ�����
    */
#if 1
    for(mask = 0x01; mask != 0; mask <<= 1) 
    {
        TEMP_18B20_DQ_OUTPUT();
        TEMP_18B20_DQ_SET_LEVEL(0); 
        HalWaitUs(1);  //t1 > 1us, ����1us�����ͷ�����(9.6)
        TEMP_18B20_DQ_INPUT();
        //master sample init, do nothing
        HalWaitUs(10);  //t2, t1 + t2 <= 15us (18.6)

        //�ȴ������ȶ�
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
    * дʱ϶����������60us�ĳ���ʱ��(60~120)
    * д1ʱ϶�����������ߺ�����������15��s���ͷ�����|_<15_|
    * д0ʱ϶�����������ߺ��������������������������ʱ϶����ʱ���Ҫ��(����60��s)
    * ��������дʱ϶����Ҫ������1us�Ļָ�ʱ��
    */
    TEMP_18B20_DQ_OUTPUT();
    for (i = 0; i < 8; i++) 
    {
        TEMP_18B20_DQ_SET_LEVEL(1); //idle
        HalWaitUs(1);               //����1us�Ļָ�ʱ��
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
	
	// ��ȡ�¶ȵ�ʵ����ֵ������������λ
	dat = (temp >> 4) & 0x7F;                         //��ȡ��������
	dat += (float)(temp&0x0F) / 16;                 //��ȡС������
	// �ж��¶ȵķ���
	if (0 != (temp & 0xF800))   //�жϷ���Ϊ��ȫΪ1��ʾ�����¶�ֵ
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
    tempWriteByte(0xCC);                        //����ROM
    // �������üĴ�������ȷ��9Bit��0.5C'
    tempWriteByte(0x4E);                        //�����ݴ���ָ��
    tempWriteByte(0xFF);                        //TH
    tempWriteByte(0xFF);                        //TL
    tempWriteByte(ACCURACY);                        //config�Ĵ���

    //tempWriteByte(0x44);                        //����һ���¶�ת��
    TemperatureGetValue();
    TemperatureGetValue();
    TemperatureGetValue();
}



