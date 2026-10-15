#include "music.h"
#include "beeper.h"
#include <stdint.h>

SoundChannel Channel1 = {.songName = NONE, .beeper = &BEEPER1},
             Channel2 = {.songName = NONE, .beeper = &BEEPER2};
static bool MusicPaused;
static uint32_t MusicPauseTick;

#define TONE_COUNT (12 * 7)

const ToneData_t ToneData = {{
#embed "../../Inc/Data/tone.bin"
}};
const uint8_t BadAppleData[] = {
#embed "../../Inc/Data/bad_apple.bin"
};
const uint8_t BigFishData[] = {
#embed "../../Inc/Data/big_fish.bin"
};
const uint8_t BadAppleCover[] = {
#embed "../../Inc/Data/bad_apple_pic.bin"
};
const uint8_t BigFishCover[] = {
#embed "../../Inc/Data/big_fish_pic.bin"
};
const uint8_t IRememberCover[] = {
#embed "../../Inc/Data/I_remember_pic.bin"
};
#define Tone (ToneData.words)

inline uint8_t Music_GetToneIndex(uint8_t note) {
  // 高四位表示音名，依次从C到A,对应0到11
  // 低四位表示八度，有错位，例如0b00000000对应C1
  // 打包乐谱时为了节省空间，应当以这种方式包装成bin
  return (note >> 4) + ((note & 0b00001111) - 1) * 12;
}
inline uint32_t Music_GetToneFreq(uint8_t note) {
  return Tone[Music_GetToneIndex(note)];
}
inline uint8_t Music_GetSongNote(SongName songName, uint16_t index,
                                 uint8_t channel) {
  if (channel > 1u || index >= Music_GetSongSize(songName)) {
    return 0;
  }

  switch (songName) {
  case BADAPPLE:
    return BadAppleData[index * 2u + channel];
  case BIG_FISH:
    return BigFishData[index * 2u + channel];
  default:
    return 0;
  }
}
inline uint16_t Music_GetSongSize(SongName songName) {
  switch (songName) {
  case BADAPPLE:
    return sizeof(BadAppleData) / 2u;
  case BIG_FISH:
    return sizeof(BigFishData) / 2u;
  default:
    return 0;
  }
}
inline uint16_t Music_GetSongBeat(SongName songName) {
  switch (songName) {
  case BADAPPLE:
    return 60 * 1000 / 140 / 2;
  case BIG_FISH:
    return 60 * 1000 / 140 / 2;
  default:
    return 0;
  }
}
void Music_PlayNote(SoundChannel *channel, uint8_t note) {
  if (note == 0) { // 休止符
    Beeper_Stop(channel->beeper);
    channel->isPWMStart = false;
    return;
  }
  Beeper_Set(channel->beeper, PSC, Music_GetToneFreq(note));
  if (!channel->isPWMStart) {
    Beeper_Start(channel->beeper);
    channel->isPWMStart = true;
  }
}
inline void Music_StopNote(SoundChannel *channel) {
  Beeper_Stop(channel->beeper);
  channel->isPWMStart = false;
}

static void Music_ResetChannel(SoundChannel *channel) {
  Music_StopNote(channel);
  channel->isPlaying = false;
  channel->noteIndex = 0;
  channel->songName = NONE;
  channel->songSize = 0;
  channel->nextNoteTime = 0;
}

static void Music_ResumeChannel(SoundChannel *channel, uint8_t voice,
                                uint16_t rewindMs) {
  if (!channel->isPlaying) {
    return;
  }

  uint32_t beat = Music_GetSongBeat(channel->songName);
  if (beat == 0 || channel->songSize == 0) {
    Music_ResetChannel(channel);
    return;
  }

  int32_t remaining = (int32_t)(channel->nextNoteTime - MusicPauseTick);
  if (remaining < 0) {
    remaining = 0;
  } else if ((uint32_t)remaining > beat) {
    remaining = (int32_t)beat;
  }

  uint32_t elapsed =
      (uint32_t)channel->noteIndex * beat + beat - (uint32_t)remaining;
  elapsed = elapsed > rewindMs ? elapsed - rewindMs : 0;
  uint32_t noteIndex = elapsed / beat;
  if (noteIndex >= channel->songSize) {
    Music_ResetChannel(channel);
    return;
  }

  uint32_t notePhase = elapsed % beat;
  channel->noteIndex = (uint16_t)noteIndex;
  channel->nextNoteTime = Timer + beat - notePhase;
  uint8_t note =
      Music_GetSongNote(channel->songName, channel->noteIndex, voice);
  Music_PlayNote(channel, note);
}

inline void Music_UpdateState() {
  if (MusicPaused) {
    return;
  }

  if (Channel1.isPlaying) {
    if ((int32_t)(Timer - Channel1.nextNoteTime) >= 0) {
      if (Channel1.noteIndex == Channel1.songSize - 1) {
        Music_StopNote(&Channel1);
        Channel1.noteIndex = 0;
        Channel1.isPlaying = false;
        Channel1.songName = NONE;
        Channel1.nextNoteTime = 0;
        Channel1.songSize = 0;
      } else {
        Channel1.noteIndex++;
        Music_PlayNote(&Channel1, Music_GetSongNote(
                    Channel1.songName, Channel1.noteIndex, 0));
        Channel1.nextNoteTime += Music_GetSongBeat(Channel1.songName);
      }
    }
  }
  if (Channel2.isPlaying) {
    if ((int32_t)(Timer - Channel2.nextNoteTime) >= 0) {
      if (Channel2.noteIndex == Channel2.songSize - 1) {
        Music_StopNote(&Channel2);
        Channel2.noteIndex = 0;
        Channel2.isPlaying = false;
        Channel2.songName = NONE;
        Channel2.nextNoteTime = 0;
        Channel2.songSize = 0;
      } else {
        Channel2.noteIndex++;
        Music_StopNote(&Channel2);
        Music_PlayNote(&Channel2, Music_GetSongNote(
                    Channel2.songName, Channel2.noteIndex, 1));
        Channel2.nextNoteTime += Music_GetSongBeat(Channel2.songName);
      }
    }
  }
}
void Music_Play(SongName songName) {
  MusicPaused = false;
  uint16_t songSize = Music_GetSongSize(songName);
  uint16_t beat = Music_GetSongBeat(songName);
  if (songSize == 0 || beat == 0) {
    Music_Stop();
    return;
  }

  Channel1.songName = songName;
  Channel1.songSize = songSize;
  Channel1.isPlaying = true;
  Channel1.noteIndex = 0;
  Channel1.nextNoteTime = Timer + beat;
  Music_PlayNote(&Channel1,
                 Music_GetSongNote(Channel1.songName, Channel1.noteIndex, 0));

  Music_StopNote(&Channel2);
  Channel2.songName = songName;
  Channel2.songSize = songSize;
  Channel2.isPlaying = true;
  Channel2.noteIndex = 0;
  Channel2.nextNoteTime = Timer + beat;
  Music_PlayNote(&Channel2,
                 Music_GetSongNote(songName, Channel2.noteIndex, 1));
  //   Beeper_Start(Channel1.beeper);
}

void Music_Stop() {
  MusicPaused = false;
  Music_ResetChannel(&Channel1);
  Music_ResetChannel(&Channel2);
}

void Music_Pause() {
  if (MusicPaused) {
    return;
  }
  MusicPaused = true;
  MusicPauseTick = Timer;
  Music_StopNote(&Channel1);
  Music_StopNote(&Channel2);
}

void Music_Resume(uint16_t rewindMs) {
  if (!MusicPaused) {
    return;
  }
  MusicPaused = false;
  Music_ResumeChannel(&Channel1, 0, rewindMs);
  Music_ResumeChannel(&Channel2, 1, rewindMs);
}