#pragma once
#include "beeper.h"
#include "timer.h"
#include <stdint.h>

#define PSC 7

typedef enum{
    BADAPPLE,NONE
} SongName;
typedef struct {
  bool isPlaying;
  uint8_t noteIndex;
  SongName songName;
  uint16_t songSize;
  uint16_t nextNoteTime;
  TIM_HandleTypeDef *beeper;
} SoundChannel;
typedef union {
  uint8_t bytes[12 * 7 * sizeof(uint32_t)];
  uint32_t words[12 * 7];
} ToneData_t;

extern const ToneData_t ToneData;

extern SoundChannel Channel1, Channel2;

extern const uint8_t BadApple[456];

extern uint16_t Music_GetSongSize(SongName songName);
extern uint16_t Music_GetSongBeat(SongName songName);
extern void Music_PlayNote(SoundChannel *channel, uint8_t note);
extern void Music_StopNote(SoundChannel *channel);
extern void Music_UpdateState();
extern void Music_Play(SongName songName);