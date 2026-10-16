#pragma once
#include "timer.h"
#include <stdint.h>

#define PSC 35

typedef enum { BADAPPLE = 1, BIG_FISH = 2, WOLRDEXECUTE = 3, NONE = 4 } SongName;
typedef struct {
  bool isPlaying;
  bool isPWMStart;
  uint16_t noteIndex;
  SongName songName;
  uint16_t songSize;
  uint32_t nextNoteTime;
  TIM_HandleTypeDef *beeper;
} SoundChannel;
typedef union {
  uint8_t bytes[12 * 7 * sizeof(uint32_t)];
  uint32_t words[12 * 7];
} ToneData_t;

extern const ToneData_t ToneData;

extern SoundChannel Channel1, Channel2;

extern const uint8_t BadAppleData[];
extern const uint8_t BigFishData[];
extern const uint8_t WorldExecuteData[];
extern const uint8_t BadAppleCover[];
extern const uint8_t BigFishCover[];
extern const uint8_t WorldExecuteCover[];

extern inline uint32_t Music_GetToneFreq(uint8_t note);
extern uint16_t Music_GetSongSize(SongName songName);
extern uint16_t Music_GetSongBeat(SongName songName);
extern uint8_t Music_GetSongNote(SongName songName, uint16_t index,
                                 uint8_t channel);
extern void Music_PlayNote(SoundChannel *channel, uint8_t note);
extern void Music_StopNote(SoundChannel *channel);
extern void Music_UpdateState();
extern void Music_Play(SongName songName);
extern void Music_Stop();
extern void Music_Pause();
extern void Music_Resume(uint16_t rewindMs);