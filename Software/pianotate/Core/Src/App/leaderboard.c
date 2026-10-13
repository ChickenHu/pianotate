#include "leaderboard.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"
#include <string.h>

#define LEADERBOARD_MAGIC 0x504E4C42u
#define LEADERBOARD_VERSION 1u
#define LEADERBOARD_SONGS 2u

extern uint32_t _leaderboard_flash_start;

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint32_t scores[LEADERBOARD_SONGS][LEADERBOARD_ENTRIES];
} LeaderboardRecord_t;

static uint32_t Scores[LEADERBOARD_SONGS][LEADERBOARD_ENTRIES];

static int8_t Leaderboard_GetSongIndex(SongName songName) {
  if (songName == BADAPPLE) {
    return 0;
  }
  if (songName == FISH) {
    return 1;
  }
  return -1;
}

void Leaderboard_Init(void) {
  uint32_t address = (uint32_t)(uintptr_t)&_leaderboard_flash_start;
  LeaderboardRecord_t record;
  memcpy(&record, (const void *)(uintptr_t)address, sizeof(record));

  if (record.magic != LEADERBOARD_MAGIC ||
      record.version != LEADERBOARD_VERSION) {
    memset(Scores, 0, sizeof(Scores));
    return;
  }
  memcpy(Scores, record.scores, sizeof(Scores));
}

uint32_t Leaderboard_GetScore(SongName songName, uint8_t rank) {
  int8_t songIndex = Leaderboard_GetSongIndex(songName);
  if (songIndex < 0 || rank >= LEADERBOARD_ENTRIES) {
    return 0;
  }
  return Scores[(uint8_t)songIndex][rank];
}

bool Leaderboard_RecordScore(SongName songName, uint32_t score) {
  int8_t songIndex = Leaderboard_GetSongIndex(songName);
  if (songIndex < 0) {
    return false;
  }

  uint32_t *scores = Scores[(uint8_t)songIndex];
  uint8_t rank = 0;
  while (rank < LEADERBOARD_ENTRIES && score <= scores[rank]) {
    rank++;
  }
  if (rank >= LEADERBOARD_ENTRIES) {
    return false;
  }

  for (uint8_t index = LEADERBOARD_ENTRIES - 1; index > rank; index--) {
    scores[index] = scores[index - 1];
  }
  scores[rank] = score;

  LeaderboardRecord_t record = {
      .magic = LEADERBOARD_MAGIC,
      .version = LEADERBOARD_VERSION,
  };
  memcpy(record.scores, Scores, sizeof(Scores));

  uint32_t address = (uint32_t)(uintptr_t)&_leaderboard_flash_start;
  FLASH_EraseInitTypeDef erase = {
      .TypeErase = FLASH_TYPEERASE_PAGES,
      .PageAddress = address,
      .NbPages = 1,
  };
  uint32_t pageError = 0;
  HAL_StatusTypeDef status = HAL_FLASH_Unlock();
  if (status == HAL_OK) {
    status = HAL_FLASHEx_Erase(&erase, &pageError);
  }
  if (status == HAL_OK) {
    const uint32_t *words = (const uint32_t *)&record;
    for (uint32_t index = 1; index < sizeof(record) / sizeof(uint32_t);
         index++) {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                                 address + index * sizeof(uint32_t),
                                 words[index]);
      if (status != HAL_OK) {
        break;
      }
    }
  }
  if (status == HAL_OK) {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address,
                               LEADERBOARD_MAGIC);
  }
  HAL_FLASH_Lock();
  return status == HAL_OK;
}