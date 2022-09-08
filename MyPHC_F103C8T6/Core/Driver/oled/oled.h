#ifndef __H_OLED_H__
#define __H_OLED_H__

#include "mini-printf.h"
#include "stdint.h"
#include "oledcom.h"
#include "string.h"
#include "math.h"
#include "stdio.h"

#define LENGTH 128
#define WIDTH 8

typedef struct OLED_T
{
    char string[16];
    uint8_t num[16];
    uint8_t update1; //第一行
    uint8_t update2; //第一行
    uint8_t update3; //第一行
    uint8_t update4; //第一行
} OLED;

void Oled_Init(OLED *obj);
void Oled_Clear();
void Oled_ClearPage(uint8_t page);
void Oled_ShowAll();
void Oled_ShowPoint(uint8_t x, uint8_t y);
void Oled_ShowChar(uint8_t x, uint8_t y, uint8_t height, uint8_t width, uint8_t *data);
void String2Order(char *string, uint8_t len, uint8_t *data);
void Float2String(float fl, char *str, uint8_t d); // fl:浮点数    d:小数位数

#endif