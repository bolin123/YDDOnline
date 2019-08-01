#include "HalSPI.h"
#include "HalGPIO.h"

#define W25Q64_PAGE_SIZE  4096
#define W25Q64_CHIP_ID 0xEF16
#define W25Q64_CS_PIN 0x1c
#define W25Q64_CS_ENABLE() HalGPIOSetLevel(W25Q64_CS_PIN, 0)
#define W25Q64_CS_DISABLE() HalGPIOSetLevel(W25Q64_CS_PIN, 1)

#define W25X_WriteEnable       0x06 
#define W25X_WriteDisable      0x04 
#define W25X_ReadStatusReg     0x05 
#define W25X_WriteStatusReg    0x01 
#define W25X_ReadData          0x03 
#define W25X_FastReadData      0x0B 
#define W25X_FastReadDual      0x3B 
#define W25X_PageProgram       0x02 
#define W25X_BlockErase        0xD8 
#define W25X_SectorErase       0x20 
#define W25X_ChipErase         0xC7 
#define W25X_PowerDown         0xB9 
#define W25X_ReleasePowerDown  0xAB 
#define W25X_DeviceID          0xAB 
#define W25X_ManufactDeviceID  0x90 
#define W25X_JedecDeviceID     0x9F 

extern uint8_t g_commonBuff[4096];

/************************************************/
//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
/************************************************/
static uint8_t readSR(void)   
{  
    uint8_t byte=0;   
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_ReadStatusReg);    //发送读取状态寄存器命令    
    byte=HalSPIReadWriteByte(0Xff);             //读取一个字节  
    W25Q64_CS_DISABLE();                            //取消片选     
    return byte;   
} 


//等待空闲
static void waitBusy(void)   
{   
    while ((readSR()&0x01)==0x01);   // 等待BUSY位清空
}  


/************************************************/
//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
/***********************************************
void SPI_FLASH_Write_SR(uint8_t sr)   
{   
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_WriteStatusReg);   //发送写取状态寄存器命令    
    HalSPIReadWriteByte(sr);               //写入一个字节  
    W25Q64_CS_DISABLE();                            //取消片选             
}   
*/

/************************************************/
//SPI_FLASH写使能  
//将WEL置位   
/************************************************/
void W25Q64WriteEnable(void)   
{
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_WriteEnable);      //发送写使能  
    W25Q64_CS_DISABLE();                            //取消片选             
} 

/************************************************/
//SPI_FLASH写禁止  
//将WEL清零  
/************************************************/
void W25Q64WriteDisable(void)   
{  
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_WriteDisable);     //发送写禁止指令    
    W25Q64_CS_DISABLE();                            //取消片选             
}

/************************************************/
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
/************************************************/
void W25Q64Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)   
{ 
    uint16_t i;
    
    W25Q64_CS_ENABLE(); //使能器件   
    HalSPIReadWriteByte(W25X_ReadData); //发送读取命令   
    HalSPIReadWriteByte((uint8_t)((ReadAddr)>>16));  //发送24bit地址    
    HalSPIReadWriteByte((uint8_t)((ReadAddr)>>8));
    HalSPIReadWriteByte((uint8_t)ReadAddr);
    for(i = 0; i < NumByteToRead; i++)
    { 
        pBuffer[i] = HalSPIReadWriteByte(0XFF); //循环读数  
    }
    W25Q64_CS_DISABLE(); //取消片选             
}  

/************************************************/
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!  
/************************************************/ 
static void writePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t i;  
    W25Q64WriteEnable(); //SET WEL 
    W25Q64_CS_ENABLE();  //使能器件   
    HalSPIReadWriteByte(W25X_PageProgram); //发送写页命令   
    HalSPIReadWriteByte((uint8_t)((WriteAddr)>>16)); //发送24bit地址    
    HalSPIReadWriteByte((uint8_t)((WriteAddr)>>8));   
    HalSPIReadWriteByte((uint8_t)WriteAddr);   
    for(i = 0; i < NumByteToWrite; i++)
    {
        HalSPIReadWriteByte(pBuffer[i]);//循环写数  
    }
    W25Q64_CS_DISABLE();                            //取消片选 
    waitBusy();                     //等待写入结束
} 

/************************************************/
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
/************************************************/
static void writeNoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{                    
    uint16_t pageremain;    
    pageremain = 256 - WriteAddr % 256; //单页剩余的字节数                
    if(NumByteToWrite <= pageremain)
    {
        pageremain = NumByteToWrite;//不大于256个字节
    }
    while(1)
    {      
        writePage(pBuffer, WriteAddr, pageremain);
        if(NumByteToWrite == pageremain)
        {
            break;//写入结束了
        }
        else //NumByteToWrite>pageremain
        {
            pBuffer+=pageremain;
            WriteAddr+=pageremain;  

            NumByteToWrite-=pageremain;           //减去已经写入了的字节数
            if(NumByteToWrite>256)
                pageremain=256; //一次可以写入256个字节
            else pageremain=NumByteToWrite;       //不够256个字节了
        }
    };      
} 

/************************************************/
//擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个山区的最少时间:150ms
static void eraseSector(uint32_t Dst_Addr)   
{   
    Dst_Addr *= W25Q64_PAGE_SIZE;
    W25Q64WriteEnable();                  //SET WEL     
    waitBusy();   
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_SectorErase);      //发送扇区擦除指令 
    HalSPIReadWriteByte((uint8_t)((Dst_Addr)>>16));  //发送24bit地址    
    HalSPIReadWriteByte((uint8_t)((Dst_Addr)>>8));   
    HalSPIReadWriteByte((uint8_t)Dst_Addr);  
    W25Q64_CS_DISABLE();                            //取消片选             
    waitBusy();                     //等待擦除完成
}  

void W25Q64PageWrite(uint8_t *pBuffer, uint32_t page)
{
    eraseSector(page); //擦除这个扇区
    writeNoCheck(pBuffer, page * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE);//写入整个扇区 
}
/************************************************/
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)      
/************************************************/
void W25Q64Write(uint8_t *pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{ 
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;     
    uint16_t i;    
    uint8_t *writeBuff = g_commonBuff;//一个扇区大小

    secpos = WriteAddr / W25Q64_PAGE_SIZE; //扇区地址 0~511 for w25x16
    secoff = WriteAddr % W25Q64_PAGE_SIZE; //在扇区内的偏移
    secremain = W25Q64_PAGE_SIZE - secoff; //扇区剩余空间大小   

    if(NumByteToWrite <= secremain)
        secremain = NumByteToWrite; //不大于4096个字节
    while(1) 
    {   
        W25Q64Read(writeBuff, secpos * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE); //读出整个扇区的内容
        for(i = 0; i < secremain; i++) //校验数据
        {
            if(writeBuff[secoff + i] != 0XFF)
            {
                break; //需要擦除     
            }
        }
        
        if(i < secremain) //需要擦除
        {
            eraseSector(secpos); //擦除这个扇区
            for(i = 0; i < secremain; i++)       //复制
            {
                writeBuff[i + secoff] = pBuffer[i];   
            }
            writeNoCheck(writeBuff, secpos * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE);//写入整个扇区  
        }
        else 
        {
            writeNoCheck(pBuffer, WriteAddr, secremain);//写已经擦除了的,直接写入扇区剩余区间. 
        }
        
        if(NumByteToWrite == secremain)
        {
            break;//写入结束了
        }
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff = 0;//偏移位置为0    

            pBuffer += secremain;  //指针偏移
            WriteAddr += secremain;//写地址偏移       
            NumByteToWrite -= secremain;              //字节数递减
            if(NumByteToWrite > W25Q64_PAGE_SIZE)
            {
                secremain = W25Q64_PAGE_SIZE;  //下一个扇区还是写不完
            }
            else 
            {
                secremain = NumByteToWrite;          //下一个扇区可以写完了
            }
        }    
    };       
}

/************************************************/
//擦除整个芯片
//整片擦除时间:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//等待时间超长...
/************************************************/
void W25Q64EraseChip(void)   
{                                             
    W25Q64WriteEnable();                  //SET WEL 
    waitBusy();   
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_ChipErase);        //发送片擦除命令  
    W25Q64_CS_DISABLE();                            //取消片选             
    waitBusy();                     //等待芯片擦除结束
}   

//进入掉电模式
void W25Q64PowerDown(void)   
{ 
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_PowerDown);        //发送掉电命令  
    W25Q64_CS_DISABLE();                            //取消片选             
    //delayUs(3);                               //等待TPD  
}   

//唤醒
void W25Q64Wakeup(void)   
{  
    W25Q64_CS_ENABLE();                            //使能器件   
    HalSPIReadWriteByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
    W25Q64_CS_DISABLE();                            //取消片选               
    //delayUs(3);                               //等待TRES1
}   

static uint16_t readID(void)
{
    uint16_t id = 0;     
    W25Q64_CS_ENABLE();                 
    HalSPIReadWriteByte(0x90);//发送读取ID命令     
    HalSPIReadWriteByte(0x00);       
    HalSPIReadWriteByte(0x00);       
    HalSPIReadWriteByte(0x00);                  
    id = HalSPIReadWriteByte(0xFF)<<8;  
    id += HalSPIReadWriteByte(0xFF);  
    W25Q64_CS_DISABLE();                 
    return id;
}

uint16_t W25Q64Initialize(void)
{
    HalGPIOConfig(W25Q64_CS_PIN, HAL_IO_OUTPUT);

    W25Q64Wakeup();
    if(W25Q64_CHIP_ID == readID())
    {
        return 0;
    }
    return HAL_EXCEPTION_ID_EXFLASH;
}

void W25Q64Poll(void)
{
}

