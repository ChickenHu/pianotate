#ifndef __PIANO_H
#define __PIANO_H

#include "main.h"
#include "key.h"
#include "music.h"
#include <stdint.h>

#define PIANO_RECORD_CAPACITY 64u
#define PIANO_RECORD_MAX_DURATION_MS 60000u

typedef struct PianoRecordedNote {
	uint16_t startMs;
	uint16_t endMs;
	uint8_t note;
	uint8_t voice;
} PianoRecordedNote_t;

extern void Piano_Init(void);// 电子琴初始化函数
extern void Piano_Update(void);// 电子琴模式状态刷新函数
extern void Piano_RecordUpdate(void);

#endif
