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
    float * waveTable1;
    float * waveTable2;
    FixedPoint mix;
    uint16_t resolution;
    FixedPoint amplitude;

    int16_t swing;
    FixedPoint stepSize;
    FixedPoint index;
} fOsc_struct;

void fOsc_updateSwing(fOsc_struct * oscillator);
void fOsc_updateStepSize(fOsc_struct * oscillator);

void fOsc_init(fOsc_struct * oscillator);

int16_t fOsc_getNextSample(fOsc_struct * oscillator);


#endif /* FASTOSC_H_INCLUDED */
