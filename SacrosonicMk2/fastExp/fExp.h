#ifndef FEXP_H_INCLUDED
#define FEXP_H_INCLUDED

#define FEXP_SEMITONES_PER_OCTAVE 12
#define FEXP_STEPS_PER_SEMITONE 10
#define FEXP_SEMITONES_TOTAL 128
#define FEXP_ARRAY_SIZE (FEXP_SEMITONES_TOTAL * FEXP_STEPS_PER_SEMITONE)
#define FEXP_ARRAY_CENTER (FEXP_ARRAY_SIZE / 2)

#include <math.h>

float fExp_array[FEXP_ARRAY_SIZE];


void fExp_init();
float fExp(float semitones);

#endif /* FEXP_H_INCLUDED */
