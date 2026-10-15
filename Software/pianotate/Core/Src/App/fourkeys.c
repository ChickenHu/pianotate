#include "fourkeys.h"
#include "font.h"
#include "key.h"
#include "menu.h"
#include "mpu6050.h"
#include "music.h"
#include "oled.h"
#include "piano.h"
#include "rank.h"
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <sys/_intsup.h>

// 先z 再x 最后 y
// z先大后小，x先0再127
int32_t Speed = 4;      // 每个frame刷新时新增的Z-offset
int32_t ZOffset = 6400; // 当前的Z-offset
static int32_t InitialZOffset;
static uint32_t GameStartTick;
static uint32_t PauseTick;
static uint32_t GameOverBlinkTick;
static bool GamePaused;
static bool GameOver;
static bool GameOverTextVisible;
static bool PianoChartActive;
static uint16_t PianoChartDurationMs;
static uint8_t PianoChartNoteCount;
static const PianoRecordedNote_t *PianoChartNotes;
static uint8_t PianoRecordTone[2];
uint16_t KeyFront[4] = {0, 0, 0, 0}, // 每条轨道上当前最靠近低Z端的候选音符
    KeySize[4] = {0, 0, 0, 0};
static uint16_t JudgeNext[4] = {0, 0, 0, 0};
uint16_t Beat;
uint8_t NoteSize = 8;
uint16_t Size = 0;
uint8_t border[128][2] = {
#embed "../../Inc/Data/border.bin"
};
typedef struct {
  uint8_t x;
  uint8_t y;
} Point;
static const Point ProjectionTable[256 * 2][4] = {
#embed "../../Inc/Data/projection_table.bin"
};
static uint8_t UILayer[129 * 8] = {0};

bool StartFlag = false;
bool MusicStartFlag = false;

SongName PlayingSongName;

Note NoteBuffer[4][200] = {}; // 每条轨道上最多150个音符

Score_t Score;

static uint32_t rng_state = 0x114514u;

static void FourKeys_SetUI(void) {
  for (uint8_t i = 0; i < 8; i++) {
    UILayer[i * 129] = 0x40;
  }
  OLED_DrawLine(UILayer, 0, 0, 0, 63);
  OLED_DrawLine(UILayer, 127, 0, 127, 63);
  for (uint16_t i = 0; i < 4; i++) {
    OLED_DrawLine(
        UILayer, ProjectionTable[0][i % 4].x, ProjectionTable[0][i % 4].y,
        ProjectionTable[0][(i + 1) % 4].x, ProjectionTable[0][(i + 1) % 4].y);
  }
  OLED_DrawLine(UILayer, 0, 32, ProjectionTable[0][1].x, 32);
  OLED_DrawLine(UILayer, 127, 32, ProjectionTable[0][3].x, 32);
  for (uint16_t i = 0; i < 512; i++) {
    OLED_DrawPixel(UILayer, ProjectionTable[i][0].x, ProjectionTable[i][0].y);
    OLED_DrawPixel(UILayer, ProjectionTable[i][1].x, ProjectionTable[i][1].y);
    OLED_DrawPixel(UILayer, ProjectionTable[i][2].x, ProjectionTable[i][2].y);
    OLED_DrawPixel(UILayer, ProjectionTable[i][3].x, ProjectionTable[i][3].y);
  }
  OLED_DrawLine(UILayer, ProjectionTable[511 - 8][1].x,
                ProjectionTable[511 - 8][1].y, ProjectionTable[511 - 8][2].x,
                ProjectionTable[511 - 8][2].y);
  OLED_DrawLine(UILayer, ProjectionTable[511 - 8][0].x,
                ProjectionTable[511 - 8][0].y, ProjectionTable[511 - 8][3].x,
                ProjectionTable[511 - 8][3].y);
}

static inline uint8_t PRNG4(void) {
  rng_state = rng_state * 1664525u + 1013904223u;
  return (uint8_t)(rng_state >> 30);
}

static inline int32_t FourKeys_GetStartZ(int lane, int index) {
  uint16_t id = NoteBuffer[lane][index].startId;
  if (PianoChartActive) {
    uint32_t framePeriod = (uint32_t)FreshPeriod * 2u;
    uint32_t distance =
        (uint32_t)((uint64_t)id * (uint32_t)Speed / framePeriod);
    return InitialZOffset - (int32_t)distance;
  }
  return Beat * Size * Speed / FreshPeriod / 2 -
         Beat * id * Speed / FreshPeriod / 2;
}

static inline int32_t FourKeys_GetJudgeDiff(int lane, int index) {
  return FourKeys_GetStartZ(lane, index) - (ZOffset + 512);
}

static inline int32_t FourKeys_GetEndZ(int lane, int index) {
  uint16_t id = NoteBuffer[lane][index].endId;
  if (PianoChartActive) {
    uint32_t framePeriod = (uint32_t)FreshPeriod * 2u;
    uint32_t distance =
        (uint32_t)((uint64_t)id * (uint32_t)Speed / framePeriod);
    return InitialZOffset - (int32_t)distance - NoteSize;
  }
  return Beat * Size * Speed / FreshPeriod / 2 -
         Beat * id * Speed / FreshPeriod / 2 - NoteSize;
}

static uint32_t FourKeys_GetLeadInMs(void) {
  uint32_t framePeriod = (uint32_t)FreshPeriod * 2u;
  return (512u * framePeriod + (uint32_t)Speed - 1u) / (uint32_t)Speed;
}

static void FourKeys_UpdateTimeline(void) {
  if (!StartFlag || Speed <= 0) {
    return;
  }

  uint32_t elapsed = (uint32_t)(Timer - GameStartTick);
  uint32_t framePeriod = (uint32_t)FreshPeriod * 2u;
  uint32_t distance =
      (uint32_t)(((uint64_t)elapsed * (uint32_t)Speed) / framePeriod);
  ZOffset = InitialZOffset - (int32_t)distance;

  uint32_t musicStartDelay =
      (512u * framePeriod + (uint32_t)Speed - 1u) / (uint32_t)Speed;
  if (!MusicStartFlag && elapsed >= musicStartDelay) {
    MusicStartFlag = true;
    if (PlayingSongName != NONE) {
      Music_Play(PlayingSongName);
    }
  }
}

static void FourKeys_FormatNumber(uint32_t value, char output[11]) {
  char reversedDigits[10];
  uint8_t digitCount = 0;

  do {
    reversedDigits[digitCount++] = (char)('0' + value % 10);
    value /= 10;
  } while (value != 0 && digitCount < sizeof(reversedDigits));

  for (uint8_t digit = 0; digit < digitCount; digit++) {
    output[digit] = reversedDigits[digitCount - digit - 1];
  }
  output[digitCount] = '\0';
}

static void FourKeys_RecordPerfect(void) {
  Score.perfect++;
  Score.combo++;
  Score.maxCombo = Score.maxCombo > Score.combo ? Score.maxCombo : Score.combo;
  Score.totalScore += 200 + (Score.combo >> 2);
}

static void FourKeys_DrawScore(void) {
  char scoreText[11];
  char comboText[11];
  FourKeys_FormatNumber(Score.totalScore, scoreText);
  FourKeys_FormatNumber(Score.combo, comboText);

  OLED_Clear(&OLED2);
  Font_DrawText(&OLED2, 64, 0, "SCORE");
  Font_DrawText(&OLED2, 64 + 36, 0, scoreText);
  Font_DrawText(&OLED2, 64, 16, "COMBO");
  Font_DrawText(&OLED2, 64 + 36, 16, comboText);
  if (PianoChartActive) {
    Font_DrawText(&OLED2, 0, 48, "Improvisation");
  }
}

static const char *FourKeys_GetResultText(void) {
  if (Score.perfect == Size) {
    return "ALL PERFECT";
  }
  if (Score.combo == Size) {
    return "FULL COMBO";
  }
  return "Clear";
}

static void FourKeys_DrawResult(void) {
  FourKeys_DrawScore();
  if (GameOverTextVisible) {
    Font_DrawText(&OLED2, 64, 32, FourKeys_GetResultText());
  }
}

static void FourKeys_ClearKeyFlags(void) {
  for (uint8_t key = 0; key < 7; key++) {
    KeyList[key].keyFlag = false;
  }
}

static void FourKeys_Resume(void) {
  uint32_t elapsed = PauseTick - GameStartTick;
  uint32_t resumedElapsed = elapsed > 3000u ? elapsed - 3000u : 0u;
  GameStartTick = Timer - resumedElapsed;
  GamePaused = false;
  memset(KeyFront, 0, sizeof(KeyFront));
  PianoRecordTone[0] = 0;
  PianoRecordTone[1] = 0;

  uint32_t framePeriod = (uint32_t)FreshPeriod * 2u;
  uint32_t musicStartDelay =
      (512u * framePeriod + (uint32_t)Speed - 1u) / (uint32_t)Speed;
  if (resumedElapsed < musicStartDelay) {
    Music_Stop();
    MusicStartFlag = false;
  } else if (MusicStartFlag) {
    Music_Resume(3000);
  } else {
    Music_Resume(0);
  }
  FourKeys_UpdateTimeline();
}

void FourKeys_Init(int32_t speed, SongName songName) {
  Music_Stop();
  PianoChartActive = false;
  PianoChartNotes = NULL;
  PianoChartNoteCount = 0;
  PianoChartDurationMs = 0;
  PianoRecordTone[0] = 0;
  PianoRecordTone[1] = 0;
  GamePaused = false;
  GameOver = false;
  GameOverTextVisible = false;
  Speed = speed;
  Beat = Music_GetSongBeat(songName == NONE ? FISH : songName);
  PlayingSongName = songName;
  FourKeys_SetUI();
  uint8_t lane = 0;
  Size = songName == NONE ? 8 : Music_GetSongSize(songName);
  ZOffset = Beat * Size * Speed / FreshPeriod / 2;
  InitialZOffset = ZOffset;
  GameStartTick = Timer;
  memset(KeyFront, 0, sizeof(KeyFront));
  memset(KeySize, 0, sizeof(KeySize));
  memset(JudgeNext, 0, sizeof(JudgeNext));
  memset(NoteBuffer, 0, sizeof(NoteBuffer));
  Score = (Score_t){0};
  MusicStartFlag = false;
  if (songName == NONE) {
    NoteBuffer[0][0] = (Note){
        .noteType = ROTATE,
        .isChecked = false,
        .startId = 4,
        .endId = 4,
        .rotateDirect = false,
    };
    KeySize[0] = 1;
  } else {
    for (int i = 0; i < Size; i++) {
      // i增加，离Zoffset越来越远，越后被显示
      NoteBuffer[lane][KeySize[lane]].startId = i;
      NoteType_t noteType = NOTE;
      while (i % 8 != 7 && i + 1 < Size &&
             Music_GetSongNote(songName, i + 1) ==
                 Music_GetSongNote(songName, i)) {
        noteType = HOLD;
        i++;
      }
      NoteBuffer[lane][KeySize[lane]].noteType = noteType;
      NoteBuffer[lane][KeySize[lane]].endId = i;
      NoteBuffer[lane][KeySize[lane]].rotateDirect = false;
      KeySize[lane]++;
      if (i % 8 == 7 && PRNG4() == 0) {
        uint8_t rotateLane = PRNG4();
        uint16_t rotateIndex = KeySize[rotateLane]++;
        NoteBuffer[rotateLane][rotateIndex].noteType = ROTATE;
        NoteBuffer[rotateLane][rotateIndex].isChecked = false;
        NoteBuffer[rotateLane][rotateIndex].startId = i;
        NoteBuffer[rotateLane][rotateIndex].endId = i;
        NoteBuffer[rotateLane][rotateIndex].rotateDirect = false;
      }
      lane = PRNG4();
    }
  }
  StartFlag = true;
}

void FourKeys_InitFromPiano(int32_t speed, const PianoRecordedNote_t *notes,
                            uint8_t noteCount, uint16_t durationMs) {
  if (notes == NULL || noteCount == 0 || noteCount > PIANO_RECORD_CAPACITY ||
      speed <= 0) {
    return;
  }

  FourKeys_Init(speed, NONE);
  PianoChartActive = true;
  PianoChartNotes = notes;
  PianoChartNoteCount = noteCount;
  PianoChartDurationMs = durationMs;
  memset(KeyFront, 0, sizeof(KeyFront));
  memset(KeySize, 0, sizeof(KeySize));
  memset(JudgeNext, 0, sizeof(JudgeNext));
  memset(NoteBuffer, 0, sizeof(NoteBuffer));
  Size = noteCount;
  Score = (Score_t){0};
  MusicStartFlag = false;
  PianoRecordTone[0] = 0;
  PianoRecordTone[1] = 0;

  for (uint8_t index = 0; index < noteCount; index++) {
    uint8_t lane = notes[index].voice == 0 ? 1 : 2;
    uint16_t laneIndex = KeySize[lane]++;
    NoteBuffer[lane][laneIndex] = (Note){
        .noteType = HOLD,
        .isChecked = false,
        .startId = notes[index].startMs,
        .endId = notes[index].endMs,
        .rotateDirect = false,
    };
  }

  uint32_t framePeriod = (uint32_t)FreshPeriod * 2u;
  InitialZOffset =
      (int32_t)(((uint64_t)durationMs * (uint32_t)Speed + framePeriod - 1u) /
                    framePeriod +
                512u);
  ZOffset = InitialZOffset;
  GameStartTick = Timer;
}

static void FourKeys_UpdatePianoPlayback(void) {
  if (!PianoChartActive) {
    return;
  }

  uint32_t elapsed = (uint32_t)(Timer - GameStartTick);
  uint32_t leadInMs = FourKeys_GetLeadInMs();
  if (elapsed < leadInMs) {
    return;
  }
  uint32_t playbackMs = elapsed - leadInMs;
  uint8_t targetTone[2] = {0, 0};
  for (uint8_t index = 0; index < PianoChartNoteCount; index++) {
    const PianoRecordedNote_t *note = &PianoChartNotes[index];
    if (playbackMs >= note->startMs && playbackMs < note->endMs) {
      targetTone[note->voice] = note->note;
    }
  }

  SoundChannel *channels[2] = {&Channel1, &Channel2};
  for (uint8_t voice = 0; voice < 2; voice++) {
    if (targetTone[voice] == PianoRecordTone[voice]) {
      continue;
    }
    if (targetTone[voice] == 0) {
      Music_StopNote(channels[voice]);
    } else {
      Music_PlayNote(channels[voice], targetTone[voice]);
    }
    PianoRecordTone[voice] = targetTone[voice];
  }
}

void FourKeys_DrawNote(int32_t startZ, int32_t endZ, uint8_t lane) {
  if (startZ < endZ) {
    return;
  }

  int32_t visibleMin = ZOffset;
  int32_t visibleMax = ZOffset + 511;

  // 求音符区间与可见区间的交集
  int32_t zBegin = startZ < visibleMax ? startZ : visibleMax;

  int32_t zEnd = endZ > visibleMin ? endZ : visibleMin;

  // 两个区间没有交集
  if (zBegin < zEnd) {
    return;
  }
  int32_t indexBegin = ProjectionTable[zBegin - ZOffset][lane].x;
  int32_t indexEnd = ProjectionTable[zEnd - ZOffset][lane].x;
  if (indexEnd < indexBegin) {
    int32_t temp = indexBegin;
    indexBegin = indexEnd;
    indexEnd = temp;
  }
  for (int32_t z = indexBegin; z <= indexEnd; z++) {

    OLED_DrawLine(OLED1.FrameBufferPtr, z, border[z][lane < 2 ? 0 : 1], z, 32);
  }
}
inline void FourKeys_DrawRotate(int32_t startZ, int32_t endZ, uint8_t lane) {
  FourKeys_DrawNote(startZ, endZ, 1);
  FourKeys_DrawNote(startZ, endZ, 2);
  FourKeys_DrawNote(startZ, endZ, 3);
  FourKeys_DrawNote(startZ, endZ, 0);
}

static bool FourKeys_AllNotesJudged(void) {
  for (uint8_t lane = 0; lane < 4; lane++) {
    if (JudgeNext[lane] < KeySize[lane]) {
      return false;
    }
  }
  return true;
}

void FourKeys_JudgeInput(void) {
  if (!StartFlag) {
    if (GameOver) {
      for (uint8_t key = 0; key < 7; key++) {
        if (KeyList[key].keyFlag && KeyList[key].state == KEY_PRESSED) {
          GameOver = false;
          Menu_ReturnToMode();
          FourKeys_ClearKeyFlags();
          return;
        }
      }
    }
    return;
  }

  if (GamePaused) {
    for (uint8_t key = 2; key <= 4; key++) {
      if (!KeyList[key].keyFlag || KeyList[key].state != KEY_PRESSED) {
        continue;
      }

      if (key == 2) {
        StartFlag = false;
        MusicStartFlag = false;
        GamePaused = false;
        Music_Stop();
        Menu_ReturnToSong();
      } else if (key == 3) {
        FourKeys_Resume();
      } else {
        FourKeys_Init(Speed, PlayingSongName);
      }
      FourKeys_ClearKeyFlags();
      return;
    }
    FourKeys_ClearKeyFlags();
    return;
  }

  FourKeys_UpdateTimeline();
  FourKeys_UpdatePianoPlayback();

  for (uint8_t key = 2; key <= 4; key++) {
    if (KeyList[key].keyFlag && KeyList[key].state == KEY_PRESSED) {
      GamePaused = true;
      PauseTick = Timer;
      Music_Pause();
      Font_DrawText(&OLED1, 44, 28, "PAUSE");
      FourKeys_ClearKeyFlags();
      return;
    }
  }

  for (uint8_t lane = 0; lane < 4; lane++) {
    while (JudgeNext[lane] < KeySize[lane] &&
           NoteBuffer[lane][JudgeNext[lane]].isChecked) {
      JudgeNext[lane]++;
    }
  }

  if (FourKeys_AllNotesJudged()) {
    for (uint8_t key = 0; key < 7; key++) {
      KeyList[key].keyFlag = false;
    }
    return;
  }

  for (uint8_t key = 0; key < 7; key++) {
    uint8_t lane;
    if (key == 0) {
      lane = 1;
    } else if (key == 1) {
      lane = 2;
    } else if (key == 5) {
      lane = 3;
    } else if (key == 6) {
      lane = 0;
    } else {
      continue;
    }

    if (!KeyList[key].keyFlag || KeyList[key].state != KEY_PRESSED) {
      continue;
    }

    if (JudgeNext[lane] < KeySize[lane]) {
      uint16_t targetNote = JudgeNext[lane];
      if (NoteBuffer[lane][targetNote].noteType == ROTATE) {
        continue;
      }
      int32_t diff = FourKeys_GetJudgeDiff(lane, targetNote);
      int32_t distance = diff < 0 ? -diff : diff;
      if (distance <= Speed * 2) {
        if (distance <= Speed) {
          FourKeys_RecordPerfect();
        } else {
          Score.good++;
          Score.combo++;
          Score.maxCombo =
              Score.maxCombo > Score.combo ? Score.maxCombo : Score.combo;
          Score.totalScore += 100 + (Score.combo >> 2);
        }
        NoteBuffer[lane][targetNote].isChecked = true;
        JudgeNext[lane]++;
      } else if (diff > Speed * 2) {
        Score.miss++;
        Score.combo = 0;
        NoteBuffer[lane][targetNote].isChecked = true;
        JudgeNext[lane]++;
      } else {
        Score.miss++;
        Score.combo = 0;
      }
    } else {
      Score.miss++;
      Score.combo = 0;
    }
  }

  for (uint8_t key = 0; key < 7; key++) {
    KeyList[key].keyFlag = false;
  }

  for (uint8_t lane = 0; lane < 4; lane++) {
    while (JudgeNext[lane] < KeySize[lane]) {
      uint16_t note = JudgeNext[lane];
      if (NoteBuffer[lane][note].isChecked) {
        JudgeNext[lane]++;
        continue;
      }

      int32_t diff = FourKeys_GetJudgeDiff(lane, note);
      if (NoteBuffer[lane][note].noteType == ROTATE) {
        int32_t distance = diff < 0 ? -diff : diff;
        if (MPU6050_RotatingFlag && distance <= Speed * 4) {
          FourKeys_RecordPerfect();
          NoteBuffer[lane][note].isChecked = true;
          JudgeNext[lane]++;
          continue;
        }
        if (diff > Speed * 4) {
          Score.miss++;
          Score.combo = 0;
          NoteBuffer[lane][note].isChecked = true;
          JudgeNext[lane]++;
          continue;
        }
        break;
      }
      if (diff <= Speed * 2) {
        break;
      }

      NoteBuffer[lane][note].isChecked = true;
      Score.miss++;
      Score.combo = 0;
      JudgeNext[lane]++;
    }
  }
}

void FourKeys_UpdateState() {
  // Advance and draw the game frame.
  if (!StartFlag) {
    if (GameOver && (uint32_t)(Timer - GameOverBlinkTick) >= 500u) {
      GameOverBlinkTick = Timer;
      GameOverTextVisible = !GameOverTextVisible;
      FourKeys_DrawResult();
    }
    for (uint8_t key = 0; key < 7; key++) {
      if (KeyList[key].checkFlag && KeyList[key].state == KEY_PRESSED) {
        MenuState.isActivate = true;
        MenuState.page = MENU_PAGE_MODE;
        OLED_SendCmd(&OLED1, 0xA4);
        OLED_Clear(&OLED1);
        OLED_FreshFrame(&OLED1);
        for (uint8_t key = 0; key < 7; key++) {
          KeyList[key].checkFlag = false;
        }
        break;
      }
    }
    for (uint8_t key = 0; key < 7; key++) {
      KeyList[key].checkFlag = false;
    }
    return;
  }
  if (GamePaused) {
    return;
  }

  FourKeys_UpdateTimeline();
  FourKeys_DrawScore();

  bool playbackFinished;
  if (PianoChartActive) {
    uint32_t elapsed = (uint32_t)(Timer - GameStartTick);
    playbackFinished = elapsed >= FourKeys_GetLeadInMs() + PianoChartDurationMs;
  } else {
    playbackFinished = !Channel1.isPlaying && !Channel2.isPlaying;
  }
  if (FourKeys_AllNotesJudged() && playbackFinished) {
    if (!PianoChartActive) {
      (void)Rank_RecordScore(PlayingSongName, Score.totalScore);
    }
    StartFlag = false;
    MusicStartFlag = false;
    GameOver = true;
    GameOverTextVisible = true;
    GameOverBlinkTick = Timer;
    FourKeys_DrawResult();
    OLED_Clear(&OLED1);
    OLED_AddLayer(OLED1.FrameBufferPtr, UILayer);
    OLED_SendCmd(&OLED1, 0xA5);
    for (uint8_t key = 0; key < 7; key++) {
      KeyList[key].keyFlag = false;
    }
    return;
  }
  // ZOffset是显示区域最远处的Z轴坐标
  OLED_Clear(&OLED1);
  for (int lane = 0; lane < 4; lane++) {
    while (KeyFront[lane] + 1 < KeySize[lane] &&
           FourKeys_GetStartZ(lane, KeyFront[lane] + 1) >= ZOffset) {
      KeyFront[lane]++;
    }
    for (int16_t i = KeyFront[lane]; i >= 0; i--) {

      int32_t startZ = FourKeys_GetStartZ(lane, i);
      int32_t endZ = FourKeys_GetEndZ(lane, i);
      if (endZ > ZOffset + 512) {
        break;
      }
      switch (NoteBuffer[lane][i].noteType) {
      case HOLD:
      case NOTE: {
        if (NoteBuffer[lane][i].isChecked) {
          break;
        }
        FourKeys_DrawNote(startZ, endZ, lane);
        break;
      }
      case ROTATE: {
        FourKeys_DrawRotate(startZ, endZ, lane);
        break;
      }
      }
    }
  }
  OLED_AddLayer(OLED1.FrameBufferPtr, UILayer);
}