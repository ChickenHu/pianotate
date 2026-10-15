#pragma once

#include "music.h"
#include <stdbool.h>
#include <stdint.h>

#define RANK_ENTRIES 3

void Rank_Init(void);
uint32_t Rank_GetScore(SongName songName, uint8_t rank);
bool Rank_RecordScore(SongName songName, uint32_t score);