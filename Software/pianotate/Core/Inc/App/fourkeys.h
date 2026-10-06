#pragma once
#include "animation.h"
#include "music.h"
extern int32_t ZOffset;
extern int32_t Speed;
extern bool StartFlag;
extern void FourKeys_Init(int32_t speed,SongName songName);
extern void FourKeys_UpdateState();