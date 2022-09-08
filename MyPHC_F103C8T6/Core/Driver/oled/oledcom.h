#ifndef __H_OLEDCOM_H__
#define __H_OLEDCOM_H__

#include "stdint.h"
#define OLED_ADD 0X78    // oled的地址
#define COMMAND_ADD 0X00 //命令寄存器地址
#define DATA_ADD 0X40    //数据寄存器地址

void SetContrast(uint8_t contrast);
void EntireDisplay(uint8_t isOn);
void InverseDisplay(uint8_t isOn);
void OpenOled(uint8_t isOn);
void SetColStartAddPage(uint8_t col);
void SetpageStartAddPage(uint8_t page);
void SetMemAddMode(uint8_t mode);
void SetColStartEndAdd(uint8_t start, uint8_t end);
void SetPageStartEndAdd(uint8_t start, uint8_t end);
void SetStartLine(uint8_t start);
void SetSegmentRemap(uint8_t isOn);
void SetMultiplexRatio(uint8_t ratio);
void SetScanDirection(uint8_t isOn);
void SetDisplayOffset(uint8_t Offset, uint8_t dir);
void HorizontalScroll();
void ScrollStart(uint8_t isOn);
void SetComPins(uint8_t a5, uint8_t a4);
void OLED_Write_Byte(uint8_t address, uint8_t data);
void OpenChargePump(uint8_t isOn);
void SetClockDiv_OscFre();
void SetVcomLevel();
void SetPreCharge();


#endif
