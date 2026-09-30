#include "music.h"

#define C1 275201
#define CS1 259756
#define D1 245177
#define DS1 231416
#define E1 218427
#define F1 206168
#define FS1 194597
#define G1 183675
#define GS1 173366
#define A1 163635
#define AS1 154451
#define B1 145782

#define C2 137600
#define CS2 129877
#define D2 122588
#define DS2 115707
#define E2 109213
#define F2 103083
#define FS2 97298
#define G2 91837
#define GS2 86682
#define A2 81817
#define AS2 77225
#define B2 72891

#define C3 68800
#define CS3 64938
#define D3 61293
#define DS3 57853
#define E3 54606
#define F3 51541
#define FS3 48648
#define G3 45918
#define GS3 43341
#define A3 40908
#define AS3 38612
#define B3 36445

#define C4 34399
#define CS4 32469
#define D4 30646
#define DS4 28926
#define E4 27303
#define F4 25770
#define FS4 24324
#define G4 22958
#define GS4 21670
#define A4 20454
#define AS4 19306
#define B4 18222

#define C5 17199
#define CS5 16234
#define D5 15323
#define DS5 14463
#define E5 13651
#define F5 12885
#define FS5 12161
#define G5 11479
#define GS5 10834
#define A5 10226
#define AS5 9652
#define B5 9110

#define C6 8599
#define CS6 8116
#define D6 7661
#define DS6 7231
#define E6 6825
#define F6 6442
#define FS6 6080
#define G6 5739
#define GS6 5417
#define A6 5113
#define AS6 4826
#define B6 4555

#define C7 4299
#define CS7 4058
#define D7 3830
#define DS7 3615
#define E7 3412
#define F7 3220
#define FS7 3040
#define G7 2869
#define GS7 2708
#define A7 2556
#define AS7 2412
#define B7 2277

const uint32_t Tone[12];

const uint32_t bad_apple[456] = {
    A4, B4, C5, D5, E5, E5, A5,  G5, // 3
    E5, E5, A4, A4, E5, D5, C5,  B4, // 4
    A4, B4, C5, D5, E5, E5, D5,  C5, // 5
    B4, A4, B4, C5, A4, B4, GS4, B4, // 6
    A4, B4, C5, D5, E5, E5, A5,  G5, // 7
    E5, E5, A4, A4, E5, D5, C5,  B4, // 8
    A4, B4, C5, D5, E5, E5, D5,  C5, // 9
    B4, B4, C5, C5, D5, D5, E5,  E5, // 10
    A5, B5, C6, D6, E6, E6, A6,  G6, // 11UP
    E6, E6, A5, A5, E6, D6, C6,  B5, // 12UP
    A5, B5, C6, D6, E6, E6, D6,  C6, // 13UP
    B5, A5, B5, C6, A5, B5, GS5, B5, // 14UP
    A5, B5, C6, D6, E6, E6, A6,  G6, // 15 UP
    E6, E6, A5, A5, E6, D6, C6,  B5, // 16UP
    A5, B5, C6, D6, E6, E6, D6,  C6, // 17UP
    B5, B5, C6, C6, D6, D6, E6,  E6, // 18UP
    A4, B4, C5, D5, E5, E5, A5,  G5, // 11DOWN
    E5, E5, A4, A4, E5, D5, C5,  B4, // 12DOWN
    A4, B4, C5, D5, E5, E5, D5,  C5, // 13DOWN
    B4, A4, B4, C5, A4, B4, GS4, B4, // 14DOWN
    A4, B4, C5, D5, E5, E5, A5,  G5, // 15 DOWN
    E5, E5, A4, A4, E5, D5, C5,  B4, // 16DOWN
    A4, B4, C5, D5, E5, E5, D5,  C5, // 17DOWN
    B4, B4, C5, C5, D5, D5, E5,  E5, // 18DOWN
    G5, A5, E5, D5, E5, E5, D5,  E5, // 19
    G5, A5, E5, D5, E5, E5, D5,  E5, // 20
    D5, C5, B4, G4, A4, A4, G4,  A4, // 21
    B4, C5, D5, E5, A4, A4, E5,  G5, // 22
    G5, A5, E5, D5, E5, E5, D5,  E5, // 23
    G5, A5, E5, D5, E5, E5, D5,  E5, // 24
    D5, C5, B4, G4, A4, A4, G4,  A4, // 25
    B4, C5, D5, E5, A4, A4, E5,  G5, // 26UP
    G6, A6, E6, D6, E6, E6, D6,  E6, // 27UP
    G6, A6, E6, D6, E6, E6, D6,  E6, // 28UP
    D6, C6, B5, G5, A5, A5, G5,  A5, // 29UP
    B5, C6, D6, E6, A5, A5, E6,  G6, // 30UP
    G6, A6, E6, D6, E6, E6, D6,  E6, // 31UP
    G6, A6, E6, D6, E6, E6, A6,  B6, // 32UP
    C7, B6, A6, G6, E6, E6, D6,  E6, // 33UP
    D6, C6, A5, G5, A5, A5, E6,  G6, // 34UP
    B4, C5, D5, E5, A4, A4, E6,  G6, // 26DOWM
    G5, A5, E5, D5, E5, E5, D5,  E5, // 27DOWN
    G5, A5, E5, D5, E5, E5, D5,  E5, // 28DOWN
    D5, C5, B4, G4, A4, A4, G4,  A4, // 29DOWN
    B4, C5, D5, E5, A4, A4, E5,  G5, // 30DOWN
    G5, A5, E5, D5, E5, E5, D5,  E5, // 31DOWN
    G5, A5, E5, D5, E5, E5, A5,  B5, // 32DOWN
    C6, B5, A5, G5, E5, E5, D5,  E5, // 33DOWN
    D5, C5, A4, G4, A4, A4, E6,  G6, // 34DOWN
    G6, A6, E6, D6, E6, E6, D6,  E6, // 35
    G6, A6, E6, D6, E6, E6, A6,  B6, // 36
    D6, C6, B5, G5, A5, A5, G5,  A5, // 37
    B5, C6, D6, E6, A5, A5, E6,  G6, // 38
    G6, A6, E6, D6, E6, E6, D6,  E6, // 39
    G6, A6, E6, D6, E6, E6, A6,  B6, // 40
    C7, B6, A6, G6, E6, E6, D6,  E6, // 41
    D6, C6, A5, G5, A5, A5, A5,  A5  // 42

};
