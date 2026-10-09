#include "piano.h"
#include <stdint.h>

static const uint8_t Piano_NoteMap[7] = {
    0x40,  // C4 
    0x42,  // D4
    0x44,  // E4
    0x45,  // F4
    0x47,  // G4
    0x49,  // A4
    0x4B   // B4
};
/*按照之前的代码，应该是这样写？*/

static bool Piano_Active = false;

void Piano_Init(void)
{
    Music_StopNote(&Channel1);
    Music_StopNote(&Channel2);
    Channel1.isPlaying = false;
    Channel2.isPlaying = false;
    Piano_Active = true;
}
//初始化电子琴,蜂鸣器状态改为关，并改Piano_Active为True
void Piano_Update(void)
{
    if (Piano_Active==true)
    {
        return;
    }
//检验电子琴是否初始化，初始化过后才开始运行下面的代码
    
    Key_Update();
//刷新按键状态
    
    for (uint8_t keyIdx = 0; keyIdx < 7; keyIdx++)
    {
        if (KeyList[keyIdx].keyFlag == KEY_PRESSED)
        {
            Music_PlayNote(&Channel1, Piano_NoteMap[keyIdx]);
        }
    }
    //轮询各个按键状态，按下即发声

    
    bool Key_AllReleased = true;
    for (uint8_t keyIdx = 0; keyIdx < 7; keyIdx++)
    {
        if (KeyList[keyIdx].keyFlag == KEY_PRESSED)
        {
            Key_AllReleased = false;
            break;
        }
    }
    if (Key_AllReleased==true)
    {
        Music_StopNote(&Channel1);
    }//先默认所有按键松开，如果有没有松开的，改Key_AllReleased为False
}
