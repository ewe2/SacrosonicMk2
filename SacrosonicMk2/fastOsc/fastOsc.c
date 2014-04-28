#include "fastOsc.h"

void fOsc_updateStepSize(fOsc_struct * oscillator){
    oscillator->stepSize.c = (oscillator->pitch.c / oscillator->sampleRate.p.i) * WT_INDEX_MAX;
}

void fOsc_init(fOsc_struct * oscillator){
    if(oscillator->sampleRate.p.i < 8000){
        oscillator->sampleRate.p.i = 48000;
    }

    if(oscillator->pitch.p.i > 20000){
        oscillator->pitch.p.i = 440;
        oscillator->pitch.p.f = 0;
    }

    if(oscillator->waveform != wt_sine && oscillator->waveform != wt_tri && oscillator->waveform != wt_square){
        oscillator->waveform = wt_sine;
    }

    // not initializing amplitude because it can't break anything

    fOsc_updateStepSize(oscillator);

    oscillator->index.c = 0;
}

float fOsc_getSampleFromTable(float * table, uint16_t index){
    index &= ~(3 << 14); // mask off last two bits, the maximum index is 16363 (14 full bits)

    int negative = 0;

    if(index >= WT_EFFECTIVE_SIZE / 2){ // between 0.5 and 1.0
        index -= WT_EFFECTIVE_SIZE / 2;
        negative = 1;
    }
    if(index >= WT_EFFECTIVE_SIZE / 4){ // between 0.25 and 0.5
        index = WT_EFFECTIVE_SIZE / 2 - index - 1;
    }
    if(negative) return table[index] * -1;
    return table[index];
}

int16_t fOsc_getNextSample(fOsc_struct * oscillator){
    oscillator->index.c += oscillator->stepSize.c;
    float sampleFloat = fOsc_getSampleFromTable(oscillator->waveform,oscillator->index.p.i);
    int16_t sample = (oscillator->amplitude.p.i / 2) * sampleFloat;
    return sample;
}
