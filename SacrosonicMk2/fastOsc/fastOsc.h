#ifndef FASTOSC_H_INCLUDED
#define FASTOSC_H_INCLUDED

#include "../wavetable/wavetable.h"
#include "fixedPoint.h"
#define FOSC_DUTY_RESOLUTION 256
#define FOSC_MIX_RESOLUTION 256

typedef struct{
    FixedPoint sampleRate;
    FixedPoint pitch;

    float pitchOffset;

    float * waveTable1;
    float * waveTable2;

    uint8_t mix;

    uint8_t dutyEnabled;
    uint8_t duty;

    uint16_t phase;

    FixedPoint amplitude;
    int16_t swing;

    FixedPoint stepSizeBase;
    FixedPoint stepSizeHigh;
    FixedPoint stepSizeLow;
    FixedPoint index;
} fOsc_struct;

void fOsc_updateSwing(fOsc_struct * oscillator);
void fOsc_updateStepSizeBase(fOsc_struct * oscillator);
void fOsc_updateStepSizeHigh(fOsc_struct * oscillator);
void fOsc_updateStepSizeLow(fOsc_struct * oscillator);

void fOsc_updateDerivatives(fOsc_struct * oscillator);

void fOsc_init(fOsc_struct * oscillator, uint8_t init);

int16_t fOsc_getNextSample(fOsc_struct * oscillator);


#endif /* FASTOSC_H_INCLUDED */
