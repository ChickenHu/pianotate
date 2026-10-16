#include "rank.h"
#include "music.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_flash_ex.h"
#include <string.h>

#define RANK_MAGIC 0x504E4C42u
#define RANK_VERSION 1u
#define RANK_SONGS 3u

extern uint32_t _rank_flash_start;

typedef struct {
  uint32_t magic;
  uint32_t version;
  uint32_t scores[RANK_SONGS][RANK_ENTRIES];
} RankRecord_t;

static uint32_t Scores[RANK_SONGS][RANK_ENTRIES];

static int8_t Rank_GetSongIndex(SongName songName) {
  if (songName == BADAPPLE) {
    return 0;
  }
  if (songName == BIG_FISH) {
    return 1;
  }
  if (songName == WOLRDEXECUTE) {
    return 2;
  }
  return -1;
}

void Rank_Init(void) {
  uint32_t address = (uint32_t)(uintptr_t)&_rank_flash_start;
  RankRecord_t record;
  memcpy(&record, (const void *)(uintptr_t)address, sizeof(record));

  if (record.magic != RANK_MAGIC || record.version != RANK_VERSION) {
    memset(Scores, 0, sizeof(Scores));
    return;
  }
  memcpy(Scores, record.scores, sizeof(Scores));
}

uint32_t Rank_GetScore(SongName songName, uint8_t rank) {
  int8_t songIndex = Rank_GetSongIndex(songName);
  if (songIndex < 0 || rank >= RANK_ENTRIES) {
    return 0;
  }
  return Scores[(uint8_t)songIndex][rank];
}

bool Rank_RecordScore(SongName songName, uint32_t score) {
  int8_t songIndex = Rank_GetSongIndex(songName);
  if (songIndex < 0) {
    return false;
  }

  uint32_t *scores = Scores[(uint8_t)songIndex];
  uint8_t rank = 0;
  while (rank < RANK_ENTRIES && score <= scores[rank]) {
    rank++;
  }
  if (rank >= RANK_ENTRIES) {
    return false;
  }

  for (uint8_t index = RANK_ENTRIES - 1; index > rank; index--) {
    scores[index] = scores[index - 1];
  }
  scores[rank] = score;

  RankRecord_t record = {
      .magic = RANK_MAGIC,
      .version = RANK_VERSION,
  };
  memcpy(record.scores, Scores, sizeof(Scores));

  uint32_t address = (uint32_t)(uintptr_t)&_rank_flash_start;
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
      status =
          HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                            address + index * sizeof(uint32_t), words[index]);
      if (status != HAL_OK) {
        break;
      }
    }
  }
  if (status == HAL_OK) {
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, RANK_MAGIC);
  }
  HAL_FLASH_Lock();
  return status == HAL_OK;
}