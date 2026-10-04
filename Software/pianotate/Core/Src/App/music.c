#include "music.h"

SoundChannel Channel1 = {false, 0, &BEEPER1}, Channel2 = {false, 0, &BEEPER2};

#define TONE_COUNT  (12 * 7)

const ToneData_t ToneData = {
    .bytes = {
        #embed "../../Inc/Data/tone.bin"
    }
};

#define Tone (ToneData.words)

inline uint8_t Music_GetToneIndex(uint8_t note) {
  // 高四位表示音名，依次从C到A,对应0到11
  // 低四位表示八度，有错位，例如0b00000000对应C1
  // 打包乐谱时为了节省空间，应当以这种方式包装成bin
  return (note >> 4) + (note & 0b00001111) * 12;
}
inline uint32_t Music_GetToneFreq(uint8_t note) {
  return Tone[Music_GetToneIndex(note)];
}
void Music_PlayNote(SoundChannel *channel, uint8_t note, uint8_t time) {
  Beeper_Set(channel->beeper, PSC, Music_GetToneFreq(note));
  Beeper_Start(channel->beeper);
  channel->isPlaying = true;
  channel->time = time;
}
inline void Music_StopNote(SoundChannel *channel) {
  Beeper_Stop(channel->beeper);
  channel->isPlaying = false;
  channel->time = 0;
}
inline void Music_UpdateState() {
  if (Channel1.isPlaying) {
    if (Channel1.time > 0) {
      Channel1.time--;
    } else {
      Music_StopNote(&Channel1);
    }
  }
  if (Channel2.isPlaying) {
    if (Channel2.time > 0) {
      Channel2.time--;
    } else {
      Music_StopNote(&Channel2);
    }
  }
}