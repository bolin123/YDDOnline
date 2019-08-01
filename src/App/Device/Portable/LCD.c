#include "LCD.h"

#define LCD_HIGH_BOARDRATE_PIN 0x11
#define LCD_BUSY_PIN 0x12

#define LCD_PROTOCOL_HEAD 0xAA

#define LCD_CMD_CLEAR  0x01 //清屏
#define LCD_CMD_PONIT  0x02 //画点
#define LCD_CMD_LINE   0x03 //画线
#define LCD_CMD_RECTANGLE 0x05 //画矩形
#define LCD_CMD_MOVE   0x09 //区域移动
#define LCD_CMD_STRING 0x11 //显示字符串
#define LCD_CMD_NUMBER 0x14 //显示数值
#define LCD_CMD_JPEG   0x22 //显示图片
#define LCD_CMD_BRIGHTNESS 0x30 //亮度设置

static volatile bool g_lcdConneted = false;

static void lcdDataSend(uint8_t cmd, uint8_t *data, uint16_t len)
{
    uint8_t i = 0;
    uint8_t buff[256];

    if(len < 250)
    {
        buff[i++] = LCD_PROTOCOL_HEAD;
        buff[i++] = cmd;
        memcpy(&buff[i], data, len);
        i += len;
        buff[i++] = 0xCC;
        buff[i++] = 0x33;
        buff[i++] = 0xC3;
        buff[i++] = 0x3C;

        HalUartWrite(HAL_UART_LCD_PORT, buff, i);
    }
}

static void shortNumToBuff(uint16_t num, uint8_t *buff)
{
    buff[0] = (uint8_t)(num >> 8);
    buff[1] = (uint8_t)(num & 0x00ff);
}

void LCDStringsPrint(char *data, uint8_t len, SysDisplayPosition_t *pos, bool hasBgColor, 
                                LCDCharSize_t size, uint16_t color, uint16_t bgcolor)
{
    uint8_t buff[250] = {0};
    
    if(len + 9 < sizeof(buff))
    {
        buff[0] = (uint8_t)size;
        if(hasBgColor)
        {
            buff[0] += 0x40;
        }
        shortNumToBuff(color, &buff[1]);
        shortNumToBuff(bgcolor, &buff[3]);
        shortNumToBuff(pos->x, &buff[5]);
        shortNumToBuff(pos->y, &buff[7]);
        memcpy(&buff[9], data, len);
        lcdDataSend(LCD_CMD_STRING, buff, len + 9);
    }
    else
    {
        Syslog("Err: out of range!");
    }
}

void LCDPictureShow(SysPictureID_t id)
{
    uint8_t cmd[2] = {0x00, 0x00};

    cmd[1] = (uint8_t)id;
    lcdDataSend(LCD_CMD_JPEG, cmd, sizeof(cmd));
}

/*
Mode：
0x00=Color 颜色显示矩形框。
0x01=Color 颜色填充矩形区域。
0x02=Color XOR 矩形区域数据，多用于菜单选中/不选中着色

top:左上角，bottom:右下角

*/
void LCDRectangle(uint8_t mode, uint16_t color, SysDisplayPosition_t *top, SysDisplayPosition_t *bottom)
{
    uint8_t buff[11] = {0};

    buff[0] = mode;
    shortNumToBuff(color, &buff[1]);
    shortNumToBuff(top->x, &buff[3]);
    shortNumToBuff(top->y, &buff[5]);
    shortNumToBuff(bottom->x, &buff[7]);
    shortNumToBuff(bottom->y, &buff[9]);
    lcdDataSend(LCD_CMD_RECTANGLE, buff, sizeof(buff));
}

//移动方向，0x00=向左。0x01=向右。0x02=向上。0x03=向下。
//Color：填充颜色
void LCDRectMove(uint8_t dir, uint8_t distance, uint16_t color, SysDisplayPosition_t *top, SysDisplayPosition_t *bottom)
{
    uint8_t cmd[13] = {0};

    cmd[0] = 0x80 + dir; //bit7: 0=环移. 1=平移，空出区域用颜色填充
    cmd[2] = distance;
    shortNumToBuff(color, &cmd[3]);
    shortNumToBuff(top->x, &cmd[5]);
    shortNumToBuff(top->y, &cmd[7]);
    shortNumToBuff(bottom->x, &cmd[9]);
    shortNumToBuff(bottom->y, &cmd[11]);
    lcdDataSend(LCD_CMD_MOVE, cmd, sizeof(cmd));
}

void LCDClear(void)
{
    uint8_t blue[2] = {0x00, 0x1F};

    Syslog("");
    lcdDataSend(LCD_CMD_CLEAR, blue, sizeof(blue));
}

static void lcdRecvBytes(uint8_t *data, uint16_t len)
{
    uint16_t i;
    static uint8_t buff[8];
    static uint8_t count = 0;

    for(i = 0; i < len; i++)
    {
        buff[count++] = data[i];

        if(count == 1)
        {
            if(data[i] != 0xAA)//head
            {
                count = 0;
            }
        }
        else if(count == 2)
        {
            if(data[i] != 0x00)//cmd
            {
                count = 0;
            }
        }
        else 
        {
            if(count == 8)
            {
                if(buff[2] == 0x4f && buff[3] == 0x4b)
                {
                    g_lcdConneted = true;
                    count = 0;
                }
            }
        }
    }
}

void LCDBrightnessSet(uint8_t value)
{
    uint8_t bright;
    if(value <= 100)
    {
        bright = (uint8_t)((uint16_t)value * 255 / 100);
        lcdDataSend(LCD_CMD_BRIGHTNESS, &bright, 1);
    }
}

static void uartInit(void)
{
    HalUartConfig_t config;
    
    config.baudrate = 115200;
    config.flowControl = 0;
    config.parity = 0;
    config.wordLength = USART_WordLength_8b;
    config.recvCb = lcdRecvBytes;
    HalUartConfig(HAL_UART_LCD_PORT, &config);
}

void LCDInitialize(void)
{
    //HalGPIOConfig(LCD_HIGH_BOARDRATE_PIN, HAL_IO_OUTPUT);
    //HalGPIOConfig(LCD_BUSY_PIN, HAL_IO_INPUT);
    //HalGPIOSetLevel(LCD_HIGH_BOARDRATE_PIN, 0);
    uint8_t count = 0;
    uartInit();
    do
    {
        lcdDataSend(0x00, NULL, 0);
        HalWaitMs(500);
        count++;
        if(count >= 8)
        {
            SysReboot();
        }
    }while(!g_lcdConneted);
    
}

void LCDPoll(void)
{
}

