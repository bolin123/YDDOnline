#include "HalRTC.h"
#include "HalWait.h"
#include "PowerManager.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_exti.h"

static void rtcIrqConfig(void)
{
    
    NVIC_InitTypeDef NVIC_InitStructure;        
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;      //RTCȫ���жϵ��ж�����
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;   
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     
    NVIC_Init(&NVIC_InitStructure); 
    
    EXTI_InitTypeDef EXTI_InitStructure;

    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    RTC_ITConfig(RTC_FLAG_ALR, ENABLE);
    RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������

}

uint16_t HalRTCInit(void)
{
    uint32_t errcount = 0;
    
    PWR_BackupAccessCmd(ENABLE);                //ʹ�ܺ󱸼Ĵ������� 

    if(BKP_ReadBackupRegister(BKP_DR1) != 0x5050) //��ָ���ĺ󱸼Ĵ����ж�������:��������д���ָ�����ݲ����
    {
        BKP_DeInit();                               //��λ��������	
        RCC_LSEConfig(RCC_LSE_ON);                  //�����ⲿ���پ���(LSE),ʹ��������پ���

        while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (errcount < 250)) // �ȴ��ⲿʱ�� LSE ����
        {
            errcount++;
            HalWaitMs(10);
        }

        if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) //�Ƿ�ɹ�
        {
            return HAL_EXCEPTION_ID_RTC;
        }

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);     //����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��	  
        RCC_RTCCLKCmd(ENABLE);                      //ʹ��RTCʱ��  
        RTC_WaitForLastTask();                      //�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_WaitForSynchro();                       //�ȴ�RTC�Ĵ���ͬ��	

        //RTC_ITConfig(RTC_IT_ALR, ENABLE);		//ʹ��RTC���ж�
        //RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_EnterConfigMode();                      /// ��������	
        RTC_SetPrescaler(32767);                    //����RTCԤ��Ƶ��ֵ
        RTC_WaitForLastTask();                      //�ȴ����һ�ζ�RTC�Ĵ�����д�������

        RTC_ExitConfigMode();                       //�˳�����ģʽ  
        BKP_WriteBackupRegister(BKP_DR1, 0X5050);   //��ָ���ĺ󱸼Ĵ�����д���û���������
    }
    else //ϵͳ������ʱ
    {
        //RTC_WaitForSynchro();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC->CRL &= (uint16_t)~RTC_FLAG_RSF;

        // Loop until RSF flag is set 
        while(((RTC->CRL & RTC_FLAG_RSF) == (uint16_t) RESET) && (errcount < 250)) // �ȴ��ⲿʱ�� LSE ����
        {
            errcount++;
            HalWaitMs(10);
        }

        if((RTC->CRL & RTC_FLAG_RSF) == (uint16_t)RESET) //�Ƿ�ɹ�
        {
            return HAL_EXCEPTION_ID_RTC;
        }
    }
    rtcIrqConfig();
    return 0;
}

void HalRTCAlarmSet(uint16_t seconds)
{
    printf("RTC alarm time = %d\n", seconds);
    
    RTC_SetAlarm(RTC_GetCounter() + seconds);
    RTC_WaitForLastTask();
#if 0
    RCC->APB1ENR|=1<<28;
    RCC->APB1ENR|=1<<27;
    PWR->CR|=1<<8;   

    RTC->CRL|=1<<4;   
    RTC->CRH|=1<<1;  
    RTC->ALRL=seconds&0xffff;
    RTC->ALRH=seconds>>16;
    RTC->CRL&=~(1<<4);
#endif
}


void RTCAlarm_IRQHandler(void)
{
    PMWakeup(PM_WAKEUP_TYPE_RTC);
    EXTI_ClearITPendingBit(EXTI_Line17);
}

void RTC_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)//�����ж�
    {
        //PMWakeup(PM_WAKEUP_TYPE_RTC);
        RTC_ClearITPendingBit(RTC_IT_ALR);  //�������ж�
    }
}


static uint8_t isLeapYear(uint16_t year)
{
    if(year % 4 == 0)
    {
        if(year % 100 == 0)
        {
            if(year % 400 == 0)
            {
                return 1;

            }

            else return 0;
        }
        else return 1;
    }
    else return 0;
}


const uint8_t mon_table[12] =
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};


int HalRTCSetTime(HalRTCTime_t *time)
{
    uint16_t t;
    uint16_t errcount = 0;
    uint32_t seccount = 0;

    if(time->year < HAL_RTC_YEAR_START || time->year > 2099) //2099, 0 = 2019
    {
        return -1;
    }

    for(t = HAL_RTC_YEAR_START; t < time->year; t++)
    {
        if(isLeapYear(t))
        {
            seccount += 31622400;
        }
        else seccount += 31536000;
    }

    time->month -= 1;

    for(t = 0; t < time->month; t++)
    {
        seccount += (uint32_t)mon_table[t] * 86400;

        if(isLeapYear(time->year) && t == 1)
        {
            seccount += 86400;
        }
    }

    seccount += (uint32_t)(time->day - 1) * 86400;
    seccount += (uint32_t)time->hour * 3600;
    seccount += (uint32_t)time->minute * 60;
    seccount += time->second;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    RTC_SetCounter(seccount);

    while(((RTC->CRL & RTC_FLAG_RTOFF) == (uint16_t) RESET) && (errcount < 200))
    {
        errcount++;
        HalWaitMs(10);
    }
    if((RTC->CRL & RTC_FLAG_RTOFF) == (uint16_t) RESET)
    {
        return -1;
    }
    return 0;
}

int HalRTCSetUtc(uint32_t utc)
{
    uint16_t errcount = 0;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    RTC_SetCounter(utc);

    while(((RTC->CRL & RTC_FLAG_RTOFF) == (uint16_t) RESET) && (errcount < 200))
    {
        errcount++;
        HalWaitMs(10);
    }
    if((RTC->CRL & RTC_FLAG_RTOFF) == (uint16_t) RESET)
    {
        return -1;
    }
    return 0;
}

uint32_t HalRTCGetUtc(void)
{
    return RTC_GetCounter();
}

HalRTCTime_t * HalRTCGetTime(void)
{
    static HalRTCTime_t dataTime;
	uint32_t timecount = 0; 
	uint32_t temp = 0;
	uint16_t temp1 = 0;	  
	
    timecount = RTC_GetCounter();
 	temp = timecount / 86400; 

	temp1 = HAL_RTC_YEAR_START;	
	while(temp >= 365)
	{				 
		if(isLeapYear(temp1))
		{
			if(temp >= 366)
			{
			    temp -= 366;
			}
			else if(temp == 365) 
			{
			    break;
			}
			else 
			{
			    temp1++;
			    break;
			}  
		}
		else 
		{
		    temp -= 365;
		}
		temp1++;  
	}   
	dataTime.year = temp1;
	temp1=0;
	while(temp>=28)
	{
		if(isLeapYear(dataTime.year) && temp1 == 1)
		{
			if(temp >= 29)
			{
                temp -= 29;
			}
			else 
			{
			    break; 
			}
		}
		else 
		{
			if(temp >= mon_table[temp1])
			    temp -= mon_table[temp1];
			else 
			    break;
		}
		temp1++;  
	}
	dataTime.month = temp1 + 1;
	dataTime.day = temp + 1;

	temp = timecount % 86400;     		   
	dataTime.hour = temp / 3600;     
	dataTime.minute = (temp % 3600) / 60; 
	dataTime.second = (temp % 3600) % 60;   
	return &dataTime;
}	 


