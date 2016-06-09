#ifndef __note__
#define __note__

// A4s => 4-th A sharp
#define o   0

// 5-th
#define B5  988
#define A5s 932
#define A5  880
#define G5s 831
#define G5  784
#define F5s 740
#define F5  698
#define E5  659
#define D5s 622
#define D5  587
#define C5s 554
#define C5  523

// 4-th
#define B4  494
#define A4s 466
#define A4  440
#define G4s 415
#define G4  392
#define F4s 370
#define F4  349
#define E4  330
#define D4s 311
#define D4  294
#define C4s 277
#define C4  262

// 3-th
#define B3  247
#define A3s 233
#define A3  220
#define G3s 208
#define G3  196
#define F3s 185
#define F3  175
#define E3  165
#define D3s 156
#define D3  147
#define C3s 139
#define C3  131

typedef struct {
  int size;
  char name[17];
  int noteSpeedPairs[];
} Song;

extern Song MARY_HAS_A_LITTLE_LIMB;
extern Song MARIO_THEME;
extern Song BIRTHDAY_SONG;
extern Song FUR_ELISE;
extern Song RIVER_FLOWS_IN_YOU;

#endif