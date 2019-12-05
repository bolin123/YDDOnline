#include "DataManager.h"
#include "Sys.h"
#include "W25Q64.h"

#define DATA_MANAGER_INFO_FLAG 'D'
//#define DATA_MANAGER_PACKET_LENGTH 14
#define DATA_MANAGER_FLASH_LENGTH (8*1024*1024) //8M
#define DATA_MANGAER_FLASH_VALID_LENGTH (DATA_MANAGER_FLASH_LENGTH - (DATA_MANAGER_FLASH_LENGTH % HAL_DEVICE_DATA_PACKET_LENGTH))

typedef struct
{

    char flag;
    uint32_t totalLength;
    uint32_t headAddress;
    uint32_t tailAddress;
}DataManagerStorageInfo_t;

static DataManagerStorageInfo_t g_storageInfo;

static bool dmHasMuchSpace(uint32_t size)
{
    return (g_storageInfo.totalLength + size < DATA_MANGAER_FLASH_VALID_LENGTH);
}

uint32_t DataManagerGetStoragedLength(void)
{
    return g_storageInfo.totalLength;
}

void DataManagerStorage(uint8_t *data, uint16_t len)
{
    uint32_t offset = g_storageInfo.tailAddress;
    bool covered = false;
    
    if(!dmHasMuchSpace(len)) //没有剩余空间，覆盖前面的数据
    {
        covered = true;
    }
    g_storageInfo.totalLength += len;

    if(offset + len > DATA_MANGAER_FLASH_VALID_LENGTH) //超过flash有效空间部分重头保存
    {
        uint32_t segmentLens = DATA_MANGAER_FLASH_VALID_LENGTH - offset;
        W25Q64Write(data, offset, segmentLens);
        W25Q64Write(data + segmentLens, 0, len - segmentLens);
    }
    else
    {
        W25Q64Write(data, offset, len);
    }
    g_storageInfo.tailAddress = (g_storageInfo.tailAddress + len) % DATA_MANGAER_FLASH_VALID_LENGTH;

    if(covered) //覆盖后，头地址更新与尾部一致，长度为flash有效长度
    {
        g_storageInfo.headAddress = g_storageInfo.tailAddress;
        g_storageInfo.totalLength = DATA_MANGAER_FLASH_VALID_LENGTH;
    }
    Syslog("head = %d, tail = %d, total = %d", g_storageInfo.headAddress, g_storageInfo.tailAddress, g_storageInfo.totalLength);
    HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)&g_storageInfo, sizeof(DataManagerStorageInfo_t));
}

uint32_t DataManagerLoad(uint8_t *buff, uint32_t size)
{
    //uint32_t size = num * DATA_MANAGER_PACKET_LENGTH;
    uint32_t offset = g_storageInfo.headAddress;

    if(g_storageInfo.totalLength)
    {
        if(size > g_storageInfo.totalLength)
        {
            size = g_storageInfo.totalLength;
        }

        if(offset + size > DATA_MANGAER_FLASH_VALID_LENGTH)
        {
            uint32_t segmentSize = DATA_MANGAER_FLASH_VALID_LENGTH - offset;
            W25Q64Read(buff, offset, segmentSize);
            W25Q64Read(buff + segmentSize, 0, size - segmentSize);
        }
        else
        {
            W25Q64Read(buff, offset, size);
        }
        g_storageInfo.headAddress = (g_storageInfo.headAddress + size) % DATA_MANGAER_FLASH_VALID_LENGTH;
        g_storageInfo.totalLength -= size;
        HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)&g_storageInfo, sizeof(DataManagerStorageInfo_t));
        return size;
    }
    return 0; 
}

void DataManagerInit(void)
{
    HalFlashRead(HAL_DATA_RECORD_ADDR, (uint8_t *)&g_storageInfo, sizeof(DataManagerStorageInfo_t));
    if(g_storageInfo.flag != DATA_MANAGER_INFO_FLAG)
    {
        g_storageInfo.flag = DATA_MANAGER_INFO_FLAG;
        g_storageInfo.headAddress = 0;
        g_storageInfo.tailAddress = 0;
        g_storageInfo.totalLength = 0;
        HalFlashWrite(HAL_DATA_RECORD_ADDR, (const uint8_t *)&g_storageInfo, sizeof(DataManagerStorageInfo_t));
    }
    Syslog("total len = %d", g_storageInfo.totalLength);
}

void DataManagerPoll(void)
{
}

