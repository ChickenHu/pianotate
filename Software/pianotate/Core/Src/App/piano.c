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

static bool Piano_Active = false;
static PianoState_t Piano_State;
static PianoRecordedNote_t Piano_Notes[PIANO_RECORD_CAPACITY];
static uint8_t Piano_NoteCount;
static uint16_t Piano_DurationMs;
static uint32_t Piano_RecordStartTick;
static int8_t Piano_VoiceKey[2];
static uint8_t Piano_VoiceNoteIndex[2];
static uint8_t Piano_IgnoredKeys;
static uint16_t Piano_ExitMotionMs;
static uint8_t Piano_ReviewSelection;

static uint16_t Piano_GetElapsedMs(void)
{
    uint32_t elapsed = (uint32_t)(Timer - Piano_RecordStartTick);
    return elapsed > PIANO_RECORD_MAX_DURATION_MS
               ? PIANO_RECORD_MAX_DURATION_MS
               : (uint16_t)elapsed;
}

static void Piano_StopVoice(uint8_t voice, uint16_t endMs)
{
    if (Piano_VoiceKey[voice] < 0)
    {
        return;
    }

    PianoRecordedNote_t *note = &Piano_Notes[Piano_VoiceNoteIndex[voice]];
    note->endMs = endMs > note->startMs ? endMs : note->startMs + 1u;
    Piano_VoiceKey[voice] = -1;
    Music_StopNote(voice == 0 ? &Channel1 : &Channel2);
}

static void Piano_CompleteRecording(void)
{
    Piano_DurationMs = Piano_GetElapsedMs();
    Piano_StopVoice(0, Piano_DurationMs);
    Piano_StopVoice(1, Piano_DurationMs);
    Music_Stop();
    Piano_IgnoredKeys = 0;
    Piano_ExitMotionMs = 0;
    Piano_ReviewSelection = 0;
    Piano_State = PIANO_STATE_REVIEW;
    for (uint8_t key = 0; key < 7; key++)
    {
        KeyList[key].keyFlag = false;
    }
}

static void Piano_StartNote(uint8_t key, uint16_t elapsed)
{
    int8_t voice = Piano_VoiceKey[0] < 0 ? 0
                   : Piano_VoiceKey[1] < 0 ? 1
                                           : -1;
    if (voice < 0 || Piano_NoteCount >= PIANO_RECORD_CAPACITY)
    {
        Piano_IgnoredKeys |= (uint8_t)(1u << key);
        return;
    }

    uint8_t noteIndex = Piano_NoteCount++;
    Piano_Notes[noteIndex] = (PianoRecordedNote_t){
        .startMs = elapsed,
        .endMs = elapsed + 1u,
        .note = Piano_NoteMap[key],
        .voice = (uint8_t)voice,
    };
    Piano_VoiceKey[(uint8_t)voice] = (int8_t)key;
    Piano_VoiceNoteIndex[(uint8_t)voice] = noteIndex;
    Music_PlayNote(voice == 0 ? &Channel1 : &Channel2, Piano_NoteMap[key]);
}

static void Piano_UpdateRecording(void)
{
    uint16_t elapsed = Piano_GetElapsedMs();
    Piano_DurationMs = elapsed;

    int32_t gyroY = MPU6050_Device.gy;
    if (MPU6050_Ready && (gyroY > 10000 || gyroY < -10000))
    {
        if (Piano_ExitMotionMs < 1000u)
        {
            Piano_ExitMotionMs++;
        }
        if (Piano_ExitMotionMs >= 150u)
        {
            Piano_CompleteRecording();
            return;
        }
    }
    else
    {
        Piano_ExitMotionMs = 0;
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
        if (Piano_IgnoredKeys & keyMask)
        {
            Piano_IgnoredKeys &= (uint8_t)~keyMask;
        }
        else
        {
            for (uint8_t voice = 0; voice < 2; voice++)
            {
                if (Piano_VoiceKey[voice] == (int8_t)key)
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
        Piano_ReviewSelection = 0;
    }
    if (KeyList[3].keyFlag && KeyList[3].state == KEY_PRESSED)
    {
        Piano_ReviewSelection = 1;
    }
    if (KeyList[4].keyFlag && KeyList[4].state == KEY_PRESSED)
    {
        if (Piano_ReviewSelection == 0)
        {
            Piano_Active = false;
            Menu_ReturnToMode();
        }
        else if (Piano_NoteCount > 0)
        {
            Piano_Active = false;
            FourKeys_InitFromPiano(16, Piano_Notes, Piano_NoteCount,
                                   Piano_DurationMs);
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
    Piano_NoteCount = 0;
    Piano_DurationMs = 0;
    Piano_RecordStartTick = Timer;
    Piano_VoiceKey[0] = -1;
    Piano_VoiceKey[1] = -1;
    Piano_IgnoredKeys = 0;
    Piano_ExitMotionMs = 0;
    Piano_ReviewSelection = 0;
    Piano_State = PIANO_STATE_RECORDING;
    Piano_Active = true;
}

void Piano_RecordUpdate(void)
{
    if (!Piano_Active)
    {
        return;
    }

    if (Piano_State == PIANO_STATE_RECORDING)
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
    if (!Piano_Active)
    {
        return;
    }

    if (Piano_State == PIANO_STATE_REVIEW)
    {
        OLED_Clear(&OLED1);
        Menu_DrawArrow(Piano_ReviewSelection == 0 ? 8 : 40, 4, &OLED1);
        Font_DrawText(&OLED1, 32, 8, "Exit");
        Font_DrawText(&OLED1, 32, 40, "Play");

        char countText[6] = {
            (char)('0' + Piano_NoteCount / 10u),
            (char)('0' + Piano_NoteCount % 10u),
            '/', '6', '4', '\0',
        };
        OLED_Clear(&OLED2);
        Font_DrawText(&OLED2, 0, 0, "NOTES");
        Font_DrawText(&OLED2, 42, 0, countText);
        Font_DrawText(&OLED2, 0, 24,
                      Piano_NoteCount == 0 ? "NO NOTES" : "READY");
        return;
    }

    OLED_Clear(&OLED1);
    Font_DrawText(&OLED1, 32, 28, "PIANO REC");
    char countText[6] = {
        (char)('0' + Piano_NoteCount / 10u),
        (char)('0' + Piano_NoteCount % 10u),
        '/', '6', '4', '\0',
    };
    OLED_Clear(&OLED2);
    Font_DrawText(&OLED2, 0, 0, "REC");
    Font_DrawText(&OLED2, 24, 0, countText);
    for (uint8_t voice = 0; voice < 2; voice++)
    {
        if (Piano_VoiceKey[voice] >= 0)
        {
            uint8_t key = (uint8_t)Piano_VoiceKey[voice];
            Font_DrawText(&OLED2, voice == 0 ? 0 : 64, 28,
                          Piano_NoteName[key]);
        }
    }
}
