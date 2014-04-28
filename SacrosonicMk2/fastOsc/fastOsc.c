#include "fastOsc.h"

void fOsc_updateSwing(fOsc_struct * oscillator){
    oscillator->swing = oscillator->amplitude.p.i / 2;
}

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

    if(oscillator->waveTable1 != wt_sine && oscillator->waveTable1 != wt_tri && oscillator->waveTable1 != wt_square){
        oscillator->waveTable1 = wt_sine;
    }
    if(oscillator->waveTable2 != wt_sine && oscillator->waveTable2 != wt_tri && oscillator->waveTable2 != wt_square){
        oscillator->waveTable2 = wt_square;
    }

    if(oscillator->mix.p.i > oscillator->resolution){
        oscillator->mix.p.i = 64;
        oscillator->mix.p.f = 0;
        oscillator->resolution = 128;
    }

    // not initializing amplitude because it can't break anything
    fOsc_updateSwing(oscillator);

    fOsc_updateStepSize(oscillator);

    oscillator->index.c = 0;
}

int16_t fOsc_getNextSample(fOsc_struct * oscillator){
    oscillator->index.c += oscillator->stepSize.c;
    int16_t adjustedIndex = wt_int_getTableIndex(oscillator->index.p.i);
    int negative = 0;
    if(adjustedIndex < 0) {
        adjustedIndex *= -1;
        negative = 1;
    }
    int16_t sample1 = (oscillator->swing) * oscillator->waveTable1[adjustedIndex];
    int16_t sample2 = (oscillator->swing) * oscillator->waveTable2[adjustedIndex];
    int16_t output = wt_int_mixSamples(sample1,sample2,oscillator->mix.p.i,oscillator->resolution);
    if(negative) output *= -1;
    return output;
}
