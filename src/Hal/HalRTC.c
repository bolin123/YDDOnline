#include "HalRTC.h"
#include "HalWait.h"

uint16_t HalRTCInit(void)
{
    uint32_t errcount = 0;

    if(BKP_ReadBackupRegister(BKP_DR1) != 0x5050) //从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE); //使能PWR和BKP外设时钟	 
        PWR_BackupAccessCmd(ENABLE);                //使能后备寄存器访问 
        BKP_DeInit();                               //复位备份区域	
        RCC_LSEConfig(RCC_LSE_ON);                  //设置外部低速晶振(LSE),使用外设低速晶振

        while((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (errcount < 250)) // 等待外部时钟 LSE 就绪
        {
            errcount++;
            HalWaitMs(10);
        }

        if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) //是否成功
        {
            return HAL_EXCEPTION_ID_RTC;
        }

        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);     //设置RTC时钟(RTCCLK),选择LSE作为RTC时钟	  
        RCC_RTCCLKCmd(ENABLE);                      //使能RTC时钟  
        RTC_WaitForLastTask();                      //等待最近一次对RTC寄存器的写操作完成
        RTC_WaitForSynchro();                       //等待RTC寄存器同步	

        //RTC_ITConfig(RTC_IT_SEC, ENABLE);		//使能RTC秒中断
        //RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
        RTC_EnterConfigMode();                      /// 允许配置	
        RTC_SetPrescaler(32767);                    //设置RTC预分频的值
        RTC_WaitForLastTask();                      //等待最近一次对RTC寄存器的写操作完成

        RTC_ExitConfigMode();                       //退出配置模式  
        BKP_WriteBackupRegister(BKP_DR1, 0X5050);   //向指定的后备寄存器中写入用户程序数据
    }
    else //系统继续计时
    {
        //RTC_WaitForSynchro();	//等待最近一次对RTC寄存器的写操作完成
        RTC->CRL &= (uint16_t)~RTC_FLAG_RSF;

        // Loop until RSF flag is set 
        while(((RTC->CRL & RTC_FLAG_RSF) == (uint16_t) RESET) && (errcount < 250)) // 等待外部时钟 LSE 就绪
        {
            errcount++;
            HalWaitMs(10);
        }

        if((RTC->CRL & RTC_FLAG_RSF) == (uint16_t)RESET) //是否成功
        {
            return HAL_EXCEPTION_ID_RTC;
        }
    }

    return 0;
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


