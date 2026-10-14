#pragma once
#include "music.h"
#include <stdint.h>

typedef struct PianoRecordedNote PianoRecordedNote_t;

typedef enum{
    HOLD,NOTE,ROTATE
}NoteType_t;
typedef struct {
  NoteType_t noteType;
  bool isChecked;
  uint16_t startId;
  uint16_t endId;
  bool rotateDirect; // 0-left,1-right;仅当noteType=ROTATE时有意义
} Note;

typedef struct{
    uint16_t combo;
    uint16_t good;
    uint16_t perfect;
    uint16_t miss;
    uint16_t maxCombo;
    uint32_t totalScore;
}Score_t ;

extern int32_t ZOffset;
extern int32_t Speed;
extern uint8_t border[128][2];// [0] 上边界 [1] 下边界
extern bool StartFlag;
extern void FourKeys_Init(int32_t speed,SongName songName);
extern void FourKeys_InitFromPiano(int32_t speed,
                                   const PianoRecordedNote_t *notes,
                                   uint8_t noteCount,
                                   uint16_t durationMs);
extern void FourKeys_JudgeInput(void);
extern void FourKeys_UpdateState();
extern void FourKeys_DrawRotate(int32_t startZ, int32_t endZ, uint8_t lane);
extern Score_t Score;
