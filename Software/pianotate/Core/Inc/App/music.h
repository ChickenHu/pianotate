#pragma once
#include "beeper.h"
#include "timer.h"
#include <stdint.h>

#define PSC 7

typedef struct {
  bool isPlaying;
  uint8_t time;
  TIM_HandleTypeDef *beeper;
} SoundChannel;

typedef union {
  uint8_t bytes[12 * 7 * sizeof(uint32_t)];
  uint32_t words[12 * 7];
} ToneData_t;

extern const ToneData_t ToneData;

extern SoundChannel Channel1, Channel2;

extern void Music_PlayNote(SoundChannel *channel, uint8_t note, uint8_t time);
extern void Music_StopNote(SoundChannel *channel);
extern void Music_UpdateState();