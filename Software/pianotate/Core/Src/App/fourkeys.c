#include "fourkeys.h"
#include "animation.h"
#include "music.h"
#include "oled.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
// 先z 再x 最后 y
// z先大后小，x先0再127
int32_t Speed = 4;                  // 每个frame刷新时新增的Z-offset
int32_t ZOffset = 6400;             // 当前的Z-offset
uint8_t KeyFront[4] = {0, 0, 0, 0}, // 每条轨道上当前最靠近低Z端的候选音符
    KeySize[4] = {0, 0, 0, 0};
uint16_t Beat;
uint8_t NoteSize = 8;
uint16_t Size = 0;

bool StartFlag = false;
bool MusicStartFlag = false;

SongName PlayingSongName;

typedef struct {
  NoteType_t noteType;
  bool isChecked;
  int32_t startZ;    // 音符的起始Z坐标
  int32_t endZ;      // 音符的结束Z坐标
  bool rotateDirect; // 0-left,1-right;仅当noteType=ROTATE时有意义
} Note;
Note NoteBuffer[4][150] = {}; // 每条轨道上最多150个音符

static uint32_t rng_state = 0x114514u;

static inline uint8_t PRNG4(void) {
  rng_state = rng_state * 1664525u + 1013904223u;
  return (uint8_t)(rng_state >> 30);
}

void FourKeys_Init(int32_t speed, SongName songName) {
  Speed = speed;
  Beat = Music_GetSongBeat(songName);
  PlayingSongName = songName;
  Animation_SetUI();
  uint8_t lane = 0;
  Size = Music_GetSongSize(songName);
  ZOffset = Beat * Size * Speed / FreshPeriod / 2;
  for (int i = 0; i < Size; i++) {
    // i增加，离Zoffset越来越远，越后被显示
    NoteBuffer[lane][KeySize[lane]].startZ =
        ZOffset - Beat * i * Speed / FreshPeriod / 2;
    NoteBuffer[lane][KeySize[lane]].noteType = ROTATE;
    while (Music_GetSongNote(songName, i + 1) ==
           Music_GetSongNote(songName, i)) {
      NoteBuffer[lane][KeySize[lane]].noteType = HOLD;
      i++;
    }
    NoteBuffer[lane][KeySize[lane]].endZ =
        ZOffset - Beat * i * Speed / FreshPeriod / 2 - NoteSize;
    /*ROTATE生成逻辑暂未实现*/
    KeySize[lane]++;
    lane = PRNG4();
  }
  // KeyFront[lane] = FindKeyFront(lane, ZOffset);
  // memcpy(KeySize, KeyFront, 4);
}

inline void FourKeys_DrawNote(int16_t startZ, int16_t endZ, uint8_t lane) {
  for (int16_t z = startZ; z >= endZ; z--) {
    int16_t index = z - ZOffset;
    if (index >= 512) {
      continue;
    }
    if (index < 0) {
      break;
    }

    OLED_DrawLine(OLED1.FrameBufferPtr, projection_table[index][lane].x,
                  projection_table[index][lane].y,
                  projection_table[index][lane].x, 32);
  }
}
inline void FourKeys_DrawRotate(int16_t startZ, int16_t endZ, uint8_t lane) {

}

void FourKeys_UpdateState() {
  // 判定逻辑暂时未实现
  // 显示刷新部分
  if (!StartFlag) {
    return;
  }

  /*识别是否结束*/
  if (ZOffset <= 0) {
    StartFlag = false;
    MusicStartFlag = false;
    OLED_Clear(&OLED1);
    OLED_AddLayer(OLED1.FrameBufferPtr, UILayer);
    return;
  }
  // ZOffset是显示区域最远处的Z轴坐标
  // 判定线处为ZOffset+512
  ZOffset -= Speed;
  if (!MusicStartFlag &&
      ZOffset <= Beat * Size * Speed / FreshPeriod / 2 - 512) {
    MusicStartFlag = true;
    Music_Play(PlayingSongName);
  }
  OLED_Clear(&OLED1);
  for (int lane = 0; lane < 4; lane++) {
    while (KeyFront[lane] + 1 < KeySize[lane] &&
           NoteBuffer[lane][KeyFront[lane] + 1].startZ >= ZOffset) {
      KeyFront[lane]++;
    }
    for (int16_t i = KeyFront[lane]; i >= 0; i--) {
      if (NoteBuffer[lane][i].endZ > ZOffset + 512) {
        break;
      }
      switch (NoteBuffer[lane][i].noteType) {
      case HOLD:
      case NOTE: {
        FourKeys_DrawNote(NoteBuffer[lane][i].startZ, NoteBuffer[lane][i].endZ,
                          lane);
        break;
      }
      case ROTATE: {
        FourKeys_DrawRotate(NoteBuffer[lane][i].startZ,
                            NoteBuffer[lane][i].endZ, lane);
        break;
      }
      }
    }
  }
  OLED_AddLayer(OLED1.FrameBufferPtr, UILayer);
}