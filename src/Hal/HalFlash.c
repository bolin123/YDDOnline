#include "HalFlash.h"
//#include "stm32f10x_flash.h"

#define FLASH_PAGE_SIZE  2048

#define SECTOR_START_ADDR(addr) ((addr) & (~(FLASH_PAGE_SIZE - 1)))
#define SPI_ADDR_TO_SECTOR(addr) ((addr) / FLASH_PAGE_SIZE)
#define ADDR_TO_SECTOR_OFFSET(addr) ((addr) - SECTOR_START_ADDR(addr))

extern uint8_t g_commonBuff[4096];

static int flashRead(uint32_t addr, uint8_t *buf, uint16_t len);
static void flashWrite(uint32_t addr, uint8_t *buf, uint16_t len);
static void flashPageErase(uint32_t addr);


void HalFlashInitialize(void)
{

}


void HalFlashPoll(void)
{

}


void HalFlashWrite(uint32_t addr, const uint8_t *data, uint32_t len)
{
    //uint32_t pageAddress;
    uint8_t *writeBuff = g_commonBuff;
    uint32_t pageAddress = (addr / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
    uint32_t pageOffset = addr % FLASH_PAGE_SIZE;
    uint8_t count = (len + pageOffset) / FLASH_PAGE_SIZE;
    uint32_t lastnum = len;
    uint32_t cpynum = 0;

    do
    {
        flashRead(pageAddress, writeBuff, FLASH_PAGE_SIZE); //backup first
        flashPageErase(pageAddress);
        if(lastnum <= FLASH_PAGE_SIZE - pageOffset)
        {
            cpynum = lastnum;
        }
        else
        {
            cpynum = FLASH_PAGE_SIZE - pageOffset;
        }
        
        memcpy(writeBuff + pageOffset, &data[len - lastnum], cpynum);
        flashWrite(pageAddress, writeBuff, FLASH_PAGE_SIZE);
        lastnum -= cpynum;
        pageAddress += FLASH_PAGE_SIZE;
        pageOffset = 0;
    }while(count--);
    
    
    
#if 0
	unsigned char secBuf[FLASH_PAGE_SIZE];
    unsigned char count=0, i=0;
    uint16_t tailLen=0;
    uint32_t startAddr = addr;
	uint8_t * dataByte = (uint8_t *)data;

	count = len / FLASH_PAGE_SIZE;
    tailLen = len % FLASH_PAGE_SIZE; 

    for(i = 0; i < count; i++)
    {
        memcpy(secBuf, dataByte+i*FLASH_PAGE_SIZE, FLASH_PAGE_SIZE);
        flashWrite(startAddr+i*FLASH_PAGE_SIZE, secBuf, FLASH_PAGE_SIZE);
    }
    
    //last data
    if(tailLen)
    {
        memset(secBuf, 0xff, FLASH_PAGE_SIZE);
        memcpy(secBuf, dataByte+i*FLASH_PAGE_SIZE, tailLen);
        flashWrite(startAddr+i*FLASH_PAGE_SIZE, secBuf, FLASH_PAGE_SIZE);
    }
#endif
}

void HalFlashErase(uint32_t addr)
{    
    flashPageErase(addr);
}

void HalFlashRead(uint32_t addr, uint8_t *buf, uint32_t bufSize)
{
    flashRead(addr, (uint8_t *)buf, bufSize);
}

static void flashPageErase(uint32_t addr)
{   
    uint8_t i;
    FLASH_Unlock();  
    for(i=0; i<10; i++)
    {
        FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
        if(FLASH_ErasePage(addr) == FLASH_COMPLETE)
        {
            break;
        }
    }
    
    FLASH_Lock();
}


static void flashWrite(uint32_t addr, uint8_t *buf, uint16_t len)
{
	uint32_t g_sun = 0x00000000;
	uint32_t Address = addr;
	uint8_t num = 0;
	uint8_t *str = buf;
	int i,j;
	
	FLASH_Unlock();	
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 

	if(buf != 0)
	{
		for(j=len/4; j>0; j--)
		{
			g_sun = 0;
			if(j==0)
			{
				num = len%4;
				switch(num)
				{
					case 1:
					case 2:
					case 3:		
					for(i=num-1 ;i >=0 ;i--)
					{
						g_sun |= str[i];
						if(i > 1)
						{
							g_sun = (g_sun << 8);
						}
						else
						{
							g_sun = (g_sun << 8*i);
						}
					}		
						break;
				}
			}
			else 
			{
					for(i = 3; i >= 1; i--)
					{
						g_sun |= str[i];
						g_sun = (g_sun << 8); 
					}		
					g_sun |= str[0];
			}
			str =str+4;
			if (FLASH_ProgramWord(Address, g_sun) == FLASH_COMPLETE)
			{
				Address = Address + 4;
			}
			else
			{ 
			}
		}
	}
	
	FLASH_Lock();
}

static int flashRead(uint32_t addr, uint8_t *buf, uint16_t len) 
{
	int i = 0;
	memcpy(buf, (const void *)addr, len);
	return i;

}

void FLASH_IRQHandler(void)
{
	printf("%s Entry!!!\n", __func__);
	//while(1);
}


