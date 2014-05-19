#include "fExp.h"

void fExp_init() {

    int i;

    for(i = 0; i < FEXP_ARRAY_SIZE; i++){
        fExp_array[i] = exp2f((float)(i - FEXP_ARRAY_CENTER) / (FEXP_SEMITONES_PER_OCTAVE *  FEXP_STEPS_PER_SEMITONE));
    }
}

float fExp(float semitones) {
    return fExp_array[(int)(semitones * FEXP_STEPS_PER_SEMITONE) + FEXP_ARRAY_CENTER];
}
