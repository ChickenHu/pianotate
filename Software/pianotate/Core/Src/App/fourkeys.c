#include "fourkeys.h"
#include "music.h"
#include "oled.h"
#include <stdint.h>
#include <string.h>
int32_t Speed = 4;         // 每个frame刷新时新增的Z-offset
int32_t ZOffset = 6400;    // 当前的Z-offset
uint8_t KeyFront[4] = {0,0,0,0}, // 每条轨道上当前最靠近低Z端的候选音符
    KeySize[4] = {0, 0, 0, 0};
uint8_t Beat;
uint8_t NoteSize = 8;
typedef struct {
  int32_t startZ; // 音符的起始Z坐标
  int32_t endZ;   // 音符的结束Z坐标
} Note;
Note NoteBuffer[4][150] = {}; // 每条轨道上最多64个音符
// uint8_t KeyBuffer[4];
static uint8_t FindKeyFront(uint8_t lane, int32_t offset) {
  uint8_t index = 0;
  while (index < 64 && NoteBuffer[lane][index].startZ < offset) {
    index++;
  }
  return index;
}

void FourKeys_Init(int32_t speed, SongName songName) {
  Speed = speed;
  Beat = Music_GetSongBeat(songName);
  Animation_SetUI();
  uint8_t lane = 0;
  uint16_t size = Music_GetSongSize(songName);
  ZOffset = Beat * size * Speed / FreshPeriod;
  for (int i = 0; i < size; i++) {
    // i增加，离Zoffset越来越远，越后被显示
    NoteBuffer[lane][KeySize[lane]].startZ =
        ZOffset - Beat * i * Speed / FreshPeriod;
    NoteBuffer[lane][KeySize[lane]].endZ =
        NoteBuffer[lane][KeySize[lane]].startZ - NoteSize;
    KeySize[lane]++;
    lane++;
    lane &= 0b11;
  }
  // KeyFront[lane] = FindKeyFront(lane, ZOffset);
  // memcpy(KeySize, KeyFront, 4);
}

void FourKeys_UpdateState() {
  // 判定逻辑暂时未实现
  // 显示刷新部分
  if (ZOffset < Speed) {
    return;
  }
  // ZOffset是显示区域最远处的Z轴坐标
  // 判定线处为ZOffset+512
  ZOffset -= Speed;
  OLED_Clear();
  for (int lane = 0; lane < 4; lane++) {
    while (KeyFront[lane] + 1 < KeySize[lane] &&
           NoteBuffer[lane][KeyFront[lane] + 1].startZ >= ZOffset) {
      KeyFront[lane]++;
    }
    for (int16_t i = KeyFront[lane]; i >= 0; i--) {
      if (NoteBuffer[lane][i].endZ > ZOffset + 512) {
        break;
      }
      for (int z = NoteBuffer[lane][i].startZ;
           z >= (int)NoteBuffer[lane][i].endZ; z--) {
        int16_t index = z - ZOffset;
        if (index >= 512) {
          continue;
        }
        if (index < 0) {
          break;
        }
        OLED_DrawLine(FrameBufferPtr, projection_table[index][lane % 4].x,
                      projection_table[index][lane % 4].y,
                      projection_table[index][(lane + 1) % 4].x, 32);
      }
    }
  }
  OLED_AddLayer(FrameBufferPtr, UILayer);
}