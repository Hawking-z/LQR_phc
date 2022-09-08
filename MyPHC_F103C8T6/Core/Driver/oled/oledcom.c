#include "oledcom.h"
#include "i2c.h"

void OLED_Write_Byte(uint8_t address, uint8_t data)
{
    uint8_t Byte = data;
    HAL_I2C_Mem_Write(&hi2c2, OLED_ADD, address, I2C_MEMADD_SIZE_8BIT, &Byte, sizeof(uint8_t), 1000);
}

void OLED_Write_Len(uint8_t address, uint8_t *data, uint16_t len)
{
    HAL_I2C_Mem_Write(&hi2c2, OLED_ADD, address, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

/*---------------------------------------------基础设置-----------------------------------------------------*/
//设置对比度 0-255
void SetContrast(uint8_t contrast)
{

    OLED_Write_Byte(COMMAND_ADD, 0x81);
    OLED_Write_Byte(COMMAND_ADD, contrast);
}

//是否点亮全屏 0为否 1为是
void EntireDisplay(uint8_t isOn)
{
    if (isOn)
        isOn = 1;
    OLED_Write_Byte(COMMAND_ADD, 0xA4 + isOn);
}

//像素是否翻转---1为亮变为1为灭
void InverseDisplay(uint8_t isOn)
{
    if (isOn)
        isOn = 1;
    OLED_Write_Byte(COMMAND_ADD, 0xA6 + isOn);
}

//开启屏幕 0为关闭
void OpenOled(uint8_t isOn)
{
    if (isOn)
        isOn = 1;
    OLED_Write_Byte(COMMAND_ADD, 0xAE + isOn);
}

//开启电荷泵,0为关闭
void OpenChargePump(uint8_t isOn)
{
    OLED_Write_Byte(COMMAND_ADD, 0x8D);
    OLED_Write_Byte(COMMAND_ADD, 0x10 | (isOn << 2));
}

/*---------------------------------------------地址设置-----------------------------------------------------*/
//页地址模式下设置列起始地址      0-127
void SetColStartAddPage(uint8_t col)
{
    uint8_t lower = (0x00) | (col & 0x0F);
    uint8_t higher = (0x10) | ((col & 0xF0)>>4);
    OLED_Write_Byte(COMMAND_ADD, lower);
    OLED_Write_Byte(COMMAND_ADD, higher);
}

//页地址模式下设置页起始地址      0-7
void SetpageStartAddPage(uint8_t page)
{
    if (page >= 7)
        page = 7;
    OLED_Write_Byte(COMMAND_ADD, 0xB0 + page);
}

//设置内存地址模式 ---- 00H-水平；01H-垂直；02H页
void SetMemAddMode(uint8_t mode)
{
    if (mode >= 2)
        mode = 2;
    OLED_Write_Byte(COMMAND_ADD, 0x20);
    OLED_Write_Byte(COMMAND_ADD, mode);
}

//设置列的结束和开始地址，并使指针指向开始地址 0-127
void SetColStartEndAdd(uint8_t start, uint8_t end)
{
    OLED_Write_Byte(COMMAND_ADD, 0x21);
    OLED_Write_Byte(COMMAND_ADD, start & 0x7F);
    OLED_Write_Byte(COMMAND_ADD, end & 0x7F);
}

//设置页的结束和开始地址，并使指针指向开始地址 0-7
void SetPageStartEndAdd(uint8_t start, uint8_t end)
{
    OLED_Write_Byte(COMMAND_ADD, 0x22);
    OLED_Write_Byte(COMMAND_ADD, start & 0x07);
    OLED_Write_Byte(COMMAND_ADD, end & 0x07);
}

/*---------------------------------------------硬件设置-----------------------------------------------------*/
//设置屏幕起始行 此命令用于设置屏幕起始行寄存器以设置GDDRAM起始地址, 取值范围为[0, 63]。若值为0, 则RAM第0行映射至COM0, 若值为1, 则RAM第1行映射到COM0, 以此类推。
void SetStartLine(uint8_t start) // 0-63
{
    if (start >= 63)
        start = 63;
    OLED_Write_Byte(COMMAND_ADD, 0x40 + start);
}

//此指令用于改变屏幕数据列地址和段驱动器间的映射关系, 这增强和OLED模块设计的可伸缩性。此命令只影响其后的数据输入, 已存储在GDDRAM中的数据将保持不变。A0:左右反置 A1:正常
// 0为否 1为是
void SetSegmentRemap(uint8_t isOn)
{
    OLED_Write_Byte(COMMAND_ADD, 0xA1 - isOn);
}

//设置复用率,此命令用于将默认的63复用率更改至任何范围为[16, 63]的值, COM0~COM64输出将切换到正确的输出信号
void SetMultiplexRatio(uint8_t ratio)
{
    OLED_Write_Byte(COMMAND_ADD, 0xA8);
    if (ratio <= 15)
        ratio = 16;
    if (ratio >= 63)
        ratio = 63;
    OLED_Write_Byte(COMMAND_ADD, ratio);
}
//设置列输出扫描方向,此指令用于设置列输出的扫描方向, 增强了OLED模块设计的布局的伸缩性。注意, 此指令会立即生效。例如当屏幕正常显示时调用此指令, 屏幕将会立刻垂直翻转。
void SetScanDirection(uint8_t isOn)
{
    if (isOn)
        isOn = 1;
    OLED_Write_Byte(COMMAND_ADD, 0xC0 + (isOn << 3));
}
//设置显示偏移 此命令为双字节命令, 第二个字节用于设置屏幕起始行为COM0~COM63之一
void SetDisplayOffset(uint8_t Offset, uint8_t dir) // dir为1时，表示反向
{
    if (Offset >= 63)
        Offset = 63;
    if (dir)
        Offset = 64 - Offset;
    OLED_Write_Byte(COMMAND_ADD, 0xD3);
    OLED_Write_Byte(COMMAND_ADD, Offset);
}
//设置COM硬件配置 （DAH+A[5:4]）SSD1306的COMn引脚一共有左边 COM32~COM63 和右边 COM0~COM31 共64个（金手指面朝上方）。
//通过设置A[5]可以让左右COM引脚的输出互换。A[5]=0时禁止左右反置，A[5]=1时启用左右反置。
// COM引脚的排列有序列和奇偶间隔（备选）两种，通过A[4]进行设置。A[4]=0时使用序列COM引脚配置，A[4]=1时使用奇偶间隔（备选）COM引脚配置。
void SetComPins(uint8_t a5, uint8_t a4)
{
    OLED_Write_Byte(COMMAND_ADD, 0xDA);
    OLED_Write_Byte(COMMAND_ADD, 0x02 | (a5 << 5) | (a4 << 4));
}

/*---------------------------------------------时序和指令设置-----------------------------------------------------*/
//设置显示时钟分频值/震荡频率     震荡频率 = A[7:4] 默认为1000b  分频 = A[3:0] 默认为0000b
void SetClockDiv_OscFre()
{
    //直接使用默认的
    OLED_Write_Byte(COMMAND_ADD, 0xD5);
    OLED_Write_Byte(COMMAND_ADD, 0x80);
}

//设置VCOMH电压倍率;0x00-0.65*vcc;0x20-0.77*vcc;0x30-0.83*vcc
void SetVcomLevel()
{
    //直接设置默认
    OLED_Write_Byte(COMMAND_ADD, 0xDB);
    OLED_Write_Byte(COMMAND_ADD, 0x20);
}

//设置预充电周期
void SetPreCharge()
{
    //直接设置默认
    OLED_Write_Byte(COMMAND_ADD, 0xD9);
    OLED_Write_Byte(COMMAND_ADD, 0x22);
}

/*---------------------------------------------滚动设置-----------------------------------------------------*/
//滚动设置
void HorizontalScroll()
{
}

//开启滚动
void ScrollStart(uint8_t isOn)
{
    if (isOn)
        isOn = 1;
    OLED_Write_Byte(COMMAND_ADD, 0x2E + isOn);
}
