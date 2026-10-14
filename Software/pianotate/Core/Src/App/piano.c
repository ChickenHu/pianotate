#include "piano.h"
#include "fourkeys.h"
#include "font.h"
#include "menu.h"
#include "mpu6050.h"
#include "music.h"
#include "oled.h"
#include <stdint.h>

static const uint8_t Piano_NoteMap[7] = {
    0x04,  // C4
    0x24,  // D4
    0x44,  // E4
    0x54,  // F4
    0x74,  // G4
    0x94,  // A4
    0xB4   // B4
};
static const char *Piano_NoteName[7] = {
    "C4", "D4", "E4", "F4", "G4", "A4", "B4"};

typedef enum {
    PIANO_STATE_RECORDING,
    PIANO_STATE_REVIEW,
} PianoState_t;

typedef struct {
    bool active;
    PianoState_t state;
    PianoRecordedNote_t notes[PIANO_RECORD_CAPACITY];
    uint8_t noteCount;
    uint16_t durationMs;
    uint32_t recordStartTick;
    int8_t voiceKey[2];
    uint8_t voiceNoteIndex[2];
    uint8_t ignoredKeys;
    uint16_t exitMotionMs;
    uint8_t reviewSelection;
    bool reviewAnimating;
    int16_t reviewAnimationProcess;
    int16_t reviewAnimationStartY;
    int16_t reviewAnimationEndY;
    uint8_t reviewAnimationTarget;
} PianoContext_t;

static PianoContext_t Piano = {
    .voiceKey = {-1, -1},
};

static const int16_t Piano_ReviewAnimationDelta = 16;

static void Piano_StartReviewAnimation(uint8_t target)
{
    Piano.reviewAnimating = true;
    Piano.reviewAnimationProcess = 1;
    Piano.reviewAnimationTarget = target;
    Piano.reviewAnimationStartY = Piano.reviewSelection == 0 ? 8 : 40;
    Piano.reviewAnimationEndY = target == 0 ? 8 : 40;
}

static void Piano_ReverseReviewAnimation(void)
{
    Piano.reviewAnimationProcess = 256 - Piano.reviewAnimationProcess;
    int16_t temp = Piano.reviewAnimationStartY;
    Piano.reviewAnimationStartY = Piano.reviewAnimationEndY;
    Piano.reviewAnimationEndY = temp;
    Piano.reviewAnimationTarget = Piano.reviewAnimationEndY == 8 ? 0 : 1;
}

static uint16_t Piano_GetElapsedMs(void)
{
    uint32_t elapsed = (uint32_t)(Timer - Piano.recordStartTick);
    return elapsed > PIANO_RECORD_MAX_DURATION_MS
               ? PIANO_RECORD_MAX_DURATION_MS
               : (uint16_t)elapsed;
}

static void Piano_StopVoice(uint8_t voice, uint16_t endMs)
{
    if (Piano.voiceKey[voice] < 0)
    {
        return;
    }

    PianoRecordedNote_t *note = &Piano.notes[Piano.voiceNoteIndex[voice]];
    note->endMs = endMs > note->startMs ? endMs : note->startMs + 1u;
    Piano.voiceKey[voice] = -1;
    Music_StopNote(voice == 0 ? &Channel1 : &Channel2);
}

static void Piano_CompleteRecording(void)
{
    Piano.durationMs = Piano_GetElapsedMs();
    Piano_StopVoice(0, Piano.durationMs);
    Piano_StopVoice(1, Piano.durationMs);
    Music_Stop();
    Piano.ignoredKeys = 0;
    Piano.exitMotionMs = 0;
    Piano.reviewSelection = 0;
    Piano.reviewAnimating = false;
    Piano.reviewAnimationProcess = 0;
    Piano.state = PIANO_STATE_REVIEW;
    for (uint8_t key = 0; key < 7; key++)
    {
        KeyList[key].keyFlag = false;
    }
}

static void Piano_StartNote(uint8_t key, uint16_t elapsed)
{
    int8_t voice = Piano.voiceKey[0] < 0 ? 0
                   : Piano.voiceKey[1] < 0 ? 1
                                           : -1;
    if (voice < 0 || Piano.noteCount >= PIANO_RECORD_CAPACITY)
    {
        Piano.ignoredKeys |= (uint8_t)(1u << key);
        return;
    }

    uint8_t noteIndex = Piano.noteCount++;
    Piano.notes[noteIndex] = (PianoRecordedNote_t){
        .startMs = elapsed,
        .endMs = elapsed + 1u,
        .note = Piano_NoteMap[key],
        .voice = (uint8_t)voice,
    };
    Piano.voiceKey[(uint8_t)voice] = (int8_t)key;
    Piano.voiceNoteIndex[(uint8_t)voice] = noteIndex;
    Music_PlayNote(voice == 0 ? &Channel1 : &Channel2, Piano_NoteMap[key]);
}

static void Piano_UpdateRecording(void)
{
    uint16_t elapsed = Piano_GetElapsedMs();
    Piano.durationMs = elapsed;

    int32_t gyroY = MPU6050_Device.gy;
    if (MPU6050_Ready && (gyroY > 10000 || gyroY < -10000))
    {
        if (Piano.exitMotionMs < 1000u)
        {
            Piano.exitMotionMs++;
        }
        if (Piano.exitMotionMs >= 150u)
        {
            Piano_CompleteRecording();
            return;
        }
    }
    else
    {
        Piano.exitMotionMs = 0;
    }

    if (elapsed >= PIANO_RECORD_MAX_DURATION_MS)
    {
        Piano_CompleteRecording();
        return;
    }

    for (uint8_t key = 0; key < 7; key++)
    {
        if (!KeyList[key].keyFlag || KeyList[key].state == KEY_PRESSED)
        {
            continue;
        }

        uint8_t keyMask = (uint8_t)(1u << key);
        if (Piano.ignoredKeys & keyMask)
        {
            Piano.ignoredKeys &= (uint8_t)~keyMask;
        }
        else
        {
            for (uint8_t voice = 0; voice < 2; voice++)
            {
                if (Piano.voiceKey[voice] == (int8_t)key)
                {
                    Piano_StopVoice(voice, elapsed);
                    break;
                }
            }
        }
        KeyList[key].keyFlag = false;
    }

    for (uint8_t key = 0; key < 7; key++)
    {
        if (KeyList[key].keyFlag && KeyList[key].state == KEY_PRESSED)
        {
            Piano_StartNote(key, elapsed);
            KeyList[key].keyFlag = false;
        }
    }
}

static void Piano_UpdateReviewInput(void)
{
    if (KeyList[2].keyFlag && KeyList[2].state == KEY_PRESSED)
    {
        if (Piano.reviewAnimating)
        {
            if (Piano.reviewAnimationEndY > Piano.reviewAnimationStartY)
            {
                Piano_ReverseReviewAnimation();
            }
        }
        else if (Piano.reviewSelection != 0)
        {
            Piano_StartReviewAnimation(0);
        }
    }
    if (KeyList[3].keyFlag && KeyList[3].state == KEY_PRESSED)
    {
        if (Piano.reviewAnimating)
        {
            if (Piano.reviewAnimationEndY < Piano.reviewAnimationStartY)
            {
                Piano_ReverseReviewAnimation();
            }
        }
        else if (Piano.reviewSelection != 1)
        {
            Piano_StartReviewAnimation(1);
        }
    }
    if (KeyList[4].keyFlag && KeyList[4].state == KEY_PRESSED)
    {
        if (Piano.reviewSelection == 0)
        {
            Piano.active = false;
            Menu_ReturnToMode();
        }
        else if (Piano.noteCount > 0)
        {
            Piano.active = false;
            FourKeys_InitFromPiano(16, Piano.notes, Piano.noteCount,
                                   Piano.durationMs);
        }
    }
    for (uint8_t key = 0; key < 7; key++)
    {
        KeyList[key].keyFlag = false;
    }
}


void Piano_Init(void)
{
    Music_Stop();
    Piano.noteCount = 0;
    Piano.durationMs = 0;
    Piano.recordStartTick = Timer;
    Piano.voiceKey[0] = -1;
    Piano.voiceKey[1] = -1;
    Piano.ignoredKeys = 0;
    Piano.exitMotionMs = 0;
    Piano.reviewSelection = 0;
    Piano.reviewAnimating = false;
    Piano.reviewAnimationProcess = 0;
    Piano.state = PIANO_STATE_RECORDING;
    Piano.active = true;
}

void Piano_RecordUpdate(void)
{
    if (!Piano.active)
    {
        return;
    }

    if (Piano.state == PIANO_STATE_RECORDING)
    {
        Piano_UpdateRecording();
    }
    else
    {
        Piano_UpdateReviewInput();
    }
}

void Piano_Update(void)
{
    if (!Piano.active)
    {
        return;
    }

    if (Piano.state == PIANO_STATE_REVIEW)
    {
        OLED_Clear(&OLED1);
        if (Piano.reviewAnimating)
        {
            Piano.reviewAnimationProcess += Piano_ReviewAnimationDelta;
            if (Piano.reviewAnimationProcess > 256)
            {
                Piano.reviewAnimating = false;
                Piano.reviewSelection = Piano.reviewAnimationTarget;
                Piano.reviewAnimationProcess = 0;
            }
        }
        if (Piano.reviewAnimating)
        {
            Menu_DrawArrowByProcess(Piano.reviewAnimationProcess,
                                    Piano.reviewAnimationStartY,
                                    Piano.reviewAnimationEndY, 4, &OLED1);
        }
        else
        {
            Menu_DrawArrow(Piano.reviewSelection == 0 ? 8 : 40, 4, &OLED1);
        }
        Font_DrawText(&OLED1, 32, 8, "Exit");
        Font_DrawText(&OLED1, 32, 40, "Play");

        char countText[6] = {
            (char)('0' + Piano.noteCount / 10u),
            (char)('0' + Piano.noteCount % 10u),
            '/', '6', '4', '\0',
        };
        OLED_Clear(&OLED2);
        Font_DrawText(&OLED2, 0, 0, "NOTES");
        Font_DrawText(&OLED2, 42, 0, countText);
        Font_DrawText(&OLED2, 0, 24,
                      Piano.noteCount == 0 ? "NO NOTES" : "READY");
        return;
    }

    OLED_Clear(&OLED1);
    Font_DrawText(&OLED1, 32, 28, "PIANO REC");
    char countText[6] = {
        (char)('0' + Piano.noteCount / 10u),
        (char)('0' + Piano.noteCount % 10u),
        '/', '6', '4', '\0',
    };
    OLED_Clear(&OLED2);
    Font_DrawText(&OLED2, 0, 0, "REC");
    Font_DrawText(&OLED2, 24, 0, countText);
    for (uint8_t voice = 0; voice < 2; voice++)
    {
        if (Piano.voiceKey[voice] >= 0)
        {
            uint8_t key = (uint8_t)Piano.voiceKey[voice];
            Font_DrawText(&OLED2, voice == 0 ? 0 : 64, 28,
                          Piano_NoteName[key]);
        }
    }
}
