#ifndef FASTOSC_H_INCLUDED
#define FASTOSC_H_INCLUDED

#include "../wavetable/wavetable.h"


typedef union {
    struct {
        uint16_t f;
        uint16_t i;
    } p;
    uint32_t c;
} FixedPoint;

typedef struct{
    FixedPoint sampleRate;
    FixedPoint pitch;
    float * waveform;
    FixedPoint amplitude;


    FixedPoint stepSize;
    FixedPoint index;
} fOsc_struct;

void fOsc_updateStepSize(fOsc_struct * oscillator);

void fOsc_init(fOsc_struct * oscillator);

float fOsc_getSampleFromTable(float * table, uint16_t index);

int16_t fOsc_getNextSample(fOsc_struct * oscillator);


#endif /* FASTOSC_H_INCLUDED */
