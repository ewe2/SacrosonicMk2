#ifndef WAVETABLE_H_INCLUDED
#define WAVETABLE_H_INCLUDED

#include <stdint.h>

#define WT_SIZE 4096 // size of the table in actual samples
#define WT_EFFECTIVE_SIZE (WT_SIZE * 4)
#define WT_INDEX_MAX (WT_EFFECTIVE_SIZE - 1)
#define WT_PHASE_RANGE 0.25 // amount of the waveform the table actually contains, current options: 0.25. 0.5 and 1 probably on the way (maybe)

#define PI 3.14159265

float wt_sine[WT_SIZE];
float wt_tri[WT_SIZE];
float wt_square[WT_SIZE];

void wt_init();
void wt_initSine();
void wt_initTri();
void wt_initSquare();

float wt_getSampleFromTable(float * table, float index, float offset);

float wt_mixSamples(float sample1, float sample2, float mix);

#endif /* WAVETABLE_H_INCLUDED */
