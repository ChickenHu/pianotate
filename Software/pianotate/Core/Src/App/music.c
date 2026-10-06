#include "music.h"
#include "beeper.h"
#include <stdint.h>

SoundChannel Channel1 = {false, false, 0, 0, NONE, 0, &BEEPER1},
             Channel2 = {false, false, 0, 0, NONE, 0, &BEEPER2};

#define TONE_COUNT (12 * 7)

const ToneData_t ToneData = {.bytes = {
#embed "../../Inc/Data/tone.bin"
                             }};
const uint8_t BadApple[456] = {
#embed "../../Inc/Data/bad_apple.bin"
};
const uint8_t Fish[541] = {
#embed "../../Inc/Data/fish.bin"
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
inline uint8_t Music_GetSongNote(SongName songName, uint16_t index) {
  switch (songName) {
  case BADAPPLE:
    return BadApple[index];
  case FISH:
    return Fish[index];
  default:
    return 0;
  }
}
inline uint16_t Music_GetSongSize(SongName songName) {
  switch (songName) {
  case BADAPPLE:
    return sizeof(BadApple);
  case FISH:
    return sizeof(Fish);
  default:
    return 0;
  }
}
inline uint16_t Music_GetSongBeat(SongName songName) {
  switch (songName) {
  case BADAPPLE:
    return 60 * 1000 / 140 / 2;
  case FISH:
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
  channel->isPWMStart=false;
}
inline void Music_UpdateState() {
  if (Channel1.isPlaying) {
    if (Timer >= Channel1.nextNoteTime) {
      if (Channel1.noteIndex == Channel1.songSize - 1) {
        Music_StopNote(&Channel1);
        Channel1.noteIndex = 0;
        Channel1.isPlaying = false;
        Channel1.songName = NONE;
        Channel1.nextNoteTime = 0;
        Channel1.songSize = 0;
      } else {
        Channel1.noteIndex++;
        Music_PlayNote(&Channel1, Music_GetSongNote(Channel1.songName,
                                                    Channel1.noteIndex));
        Channel1.nextNoteTime += Music_GetSongBeat(Channel1.songName);
      }
    }
  }
  if (Channel2.isPlaying) {
    if (Timer >= Channel2.nextNoteTime) {
      if (Channel2.noteIndex == Channel2.songSize - 1) {
        Music_StopNote(&Channel2);
        Channel2.noteIndex = 0;
        Channel2.isPlaying = false;
        Channel2.songName = NONE;
        Channel2.nextNoteTime = 0;
        Channel2.songSize = 0;
      } else {
        Channel2.noteIndex++;
        Music_PlayNote(&Channel2, Music_GetSongNote(Channel2.songName,
                                                    Channel2.noteIndex));
        Channel2.nextNoteTime += Music_GetSongBeat(Channel2.songName);
      }
    }
  }
}
void Music_Play(SongName songName) {
  Channel1.songName = songName;
  Channel1.songSize = Music_GetSongSize(songName);
  Channel1.isPlaying = true;
  Channel1.noteIndex = 0;
  Channel1.nextNoteTime = Timer + Music_GetSongBeat(songName);
  Music_PlayNote(&Channel1,
                 Music_GetSongNote(Channel1.songName, Channel1.noteIndex));
  //   Beeper_Start(Channel1.beeper);
}