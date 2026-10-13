#pragma once

#include "music.h"
#include <stdbool.h>
#include <stdint.h>

#define LEADERBOARD_ENTRIES 3

void Leaderboard_Init(void);
uint32_t Leaderboard_GetScore(SongName songName, uint8_t rank);
bool Leaderboard_RecordScore(SongName songName, uint32_t score);