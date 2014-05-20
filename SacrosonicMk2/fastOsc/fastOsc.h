#ifndef FASTOSC_H_INCLUDED
#define FASTOSC_H_INCLUDED

#include "../wavetable/wavetable.h"
#include "fixedPoint.h"
#include "../fastExp/fExp.h"
#define FOSC_DUTY_RESOLUTION 256
#define FOSC_WAVE_MIX_RESOLUTION 256

#define FOSC_PITCH_MAX 16000
#define FOSC_AMPLITUDE_MAX 0xFFFF

float fOsc_default_mod_src;

typedef struct{
    float * modSource;
    float modAmount;
    float modResult;
    uint8_t isLog;
} fOsc_modStruct;

typedef struct{
    FixedPoint sampleRate;

    FixedPoint pitch;
    float pitchOffset;
    FixedPoint modulatedPitch;
    FixedPoint stepSizeBase;
    FixedPoint stepSizeHigh;
    FixedPoint stepSizeLow;

    float * waveTable1;
    float * waveTable2;
    uint8_t waveMix;
    uint8_t modulatedWaveMix;

    uint8_t dutyEnabled;
    uint8_t duty;
    uint8_t modulatedDuty;

    uint16_t phase;
    uint16_t modulatedPhase;

    uint32_t amplitude;
    uint32_t modulatedAmplitude;
    int16_t swing;


    FixedPoint index;

    fOsc_modStruct pitchMod;
    fOsc_modStruct waveMixMod;
    fOsc_modStruct dutyMod;
    fOsc_modStruct phaseMod;
    fOsc_modStruct amplitudeMod;
} fOsc_struct;

void fOsc_initModulator(fOsc_modStruct * modulator);

void fOsc_updateModResult(fOsc_modStruct * modulator);

void fOsc_modulatePitch(fOsc_struct * oscillator);
void fOsc_modulateWaveMix(fOsc_struct * oscillator);
void fOsc_modulateDuty(fOsc_struct * oscillator);
void fOsc_modulatePhase(fOsc_struct * oscillator);
void fOsc_modulateAmplitude(fOsc_struct * oscillator);

void fOsc_updateSwing(fOsc_struct * oscillator);
void fOsc_updateStepSizeBase(fOsc_struct * oscillator);
void fOsc_updateStepSizeHigh(fOsc_struct * oscillator);
void fOsc_updateStepSizeLow(fOsc_struct * oscillator);

void fOsc_updateDerivatives(fOsc_struct * oscillator);

void fOsc_init(fOsc_struct * oscillator, uint8_t init);

int16_t fOsc_getNextSample(fOsc_struct * oscillator);


#endif /* FASTOSC_H_INCLUDED */
