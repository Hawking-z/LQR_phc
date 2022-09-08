#include "oledapp.h"
#include "robot.h"

extern uint8_t MyCharacter[67][16];
extern robot PHC;

void OledAppLoop(OLED *obj)
{
    if (obj->update1)
    {
        obj->update1 = 0;
        memset(obj->num, 0, sizeof(obj->num));
        memset(obj->string, 0, sizeof(obj->string));

        //处理开始
        Float2String(PHC.voltage,obj->string,4);

        //处理结束

        String2Order(obj->string, strlen(obj->string), obj->num);
        for (uint8_t i = 0; i < 16; i++)
        {
            Oled_ShowChar(i * 8, 0, 2, 8, MyCharacter[obj->num[i]]);
        }
    }
    if (obj->update2)
    {
        obj->update2 = 0;
        memset(obj->num, 0, sizeof(obj->num));
        memset(obj->string, 0, sizeof(obj->string));

        //处理开始
        switch (PHC.mode)
        {
        case 1:
            strcpy(obj->string,"moving mode");
            break;
        case 0:
            strcpy(obj->string,"stop mode");
            break;
        default:
            break;
        }
        //处理结束
        String2Order(obj->string, strlen(obj->string), obj->num);
        for (uint8_t i = 0; i < 16; i++)
        {
            Oled_ShowChar(i * 8, 2, 2, 8, MyCharacter[obj->num[i]]);
        }
    }
    if (obj->update3)
    {
        obj->update3 = 0;
        memset(obj->num, 0, sizeof(obj->num));
        memset(obj->string, 0, sizeof(obj->string));

        //处理开始
        switch (PHC.imu.bias_init_success)
        {
        case 1:
            strcpy(obj->string,"init success");
            obj->update3 = 0;
            break;
        case 0:
            strcpy(obj->string,"initializing..");
            obj->update3 = 1;
            break;
        default:
            break;
        }
        //处理结束

        String2Order(obj->string, strlen(obj->string), obj->num);
        for (uint8_t i = 0; i < 16; i++)
        {
            Oled_ShowChar(i * 8, 4, 2, 8, MyCharacter[obj->num[i]]);
        }
    }
    if (obj->update4)
    {
        obj->update4 = 0;
        memset(obj->num, 0, sizeof(obj->num));
        memset(obj->string, 0, sizeof(obj->string));

        //处理开始
        switch (PHC.bluetooth.isConnect)
        {
        case 0:
            strcpy(obj->string,"BT lost.");
            break;
        case 1:
            strcpy(obj->string,"BT connected.");
            break;
        default:
            break;
        }
        //处理结束

        String2Order(obj->string, strlen(obj->string), obj->num);
        for (uint8_t i = 0; i < 16; i++)
        {
            Oled_ShowChar(i * 8, 6, 2, 8, MyCharacter[obj->num[i]]);
        }
    }
}