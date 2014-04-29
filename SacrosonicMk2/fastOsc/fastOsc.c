#include "fastOsc.h"

void fOsc_updateSwing(fOsc_struct * oscillator) {
    oscillator->swing = oscillator->amplitude.p.i / 2;
}

void fOsc_updateStepSize(fOsc_struct * oscillator) {
    oscillator->stepSizeBase.c = (oscillator->pitch.c / oscillator->sampleRate.p.i) * WT_INDEX_MAX;
}

void fOsc_updateStepSizeHigh(fOsc_struct * oscillator) {
    oscillator->stepSizeHigh.c = (oscillator->stepSizeBase.c * FOSC_DUTY_RESOLUTION) / (FOSC_DUTY_RESOLUTION - oscillator->duty);
}

void fOsc_updateStepSizeLow(fOsc_struct * oscillator) {
    oscillator->stepSizeLow.c = (oscillator->stepSizeBase.c * FOSC_DUTY_RESOLUTION) / (FOSC_DUTY_RESOLUTION + oscillator->duty);
}

void fOsc_updateDerivatives(fOsc_struct * oscillator) {
    fOsc_updateSwing(oscillator);
    fOsc_updateStepSize(oscillator);
    fOsc_updateStepSizeHigh(oscillator);
    fOsc_updateStepSizeLow(oscillator);
}

void fOsc_init(fOsc_struct * oscillator) {
    if(oscillator->sampleRate.p.i < 8000) {
        oscillator->sampleRate.p.i = 48000;
    }

    if(oscillator->pitch.p.i > 20000) {
        oscillator->pitch.p.i = 440;
        oscillator->pitch.p.f = 0;
    }

    if(oscillator->waveTable1 != wt_sine && oscillator->waveTable1 != wt_tri && oscillator->waveTable1 != wt_square) {
        oscillator->waveTable1 = wt_sine;
    }
    if(oscillator->waveTable2 != wt_sine && oscillator->waveTable2 != wt_tri && oscillator->waveTable2 != wt_square) {
        oscillator->waveTable2 = wt_square;
    }

    // not initializing mix because it can't break anything

    // not initializing amplitude because it can't break anything

    // not initializing duty because it can't break anything

    // not initializing phase because it can't break anything

    oscillator->index.c = 0;

    fOsc_updateDerivatives(oscillator);
}

int16_t fOsc_getNextSample(fOsc_struct * oscillator) {
    FixedPoint newIndex;
    newIndex.c = 0;

    if(oscillator->index.p.i <= WT_NODE_INDEX){ // starting in the high cycle
        newIndex.c = oscillator->index.c + oscillator->stepSizeHigh.c;

        if(newIndex.p.i > WT_NODE_INDEX){ // passing into the low cycle
            uint32_t subStepHigh = (WT_NODE_INDEX << 16) - oscillator->index.c;
            uint32_t subStepLow = ((oscillator->stepSizeHigh.c - subStepHigh) / oscillator->stepSizeHigh.c) * oscillator->stepSizeLow.c;
            newIndex.c = oscillator->index.c + subStepHigh + subStepLow;

            if(newIndex.p.i > WT_INDEX_MAX){ // passing into next high cycle
                subStepLow = (WT_EFFECTIVE_SIZE / 2) << 16;
                subStepHigh = ((oscillator->stepSizeLow.c - subStepLow) / oscillator->stepSizeLow.c) * oscillator->stepSizeHigh.c;
                newIndex.c = oscillator->index.c + subStepHigh + subStepLow;
            }
        }
    } else { // starting in the low cycle
        newIndex.c = oscillator->index.c + oscillator->stepSizeLow.c;

        if(newIndex.p.i > WT_INDEX_MAX){ // passing into the high cycle
            uint32_t subStepLow = (WT_INDEX_MAX << 16) - oscillator->index.c;
            uint32_t subStepHigh = ((oscillator->stepSizeLow.c - subStepLow) / oscillator->stepSizeLow.c) * oscillator->stepSizeHigh.c;
            newIndex.c = oscillator->index.c + subStepHigh + subStepLow;

            if(newIndex.p.i > WT_INDEX_MAX + WT_NODE_INDEX){ // passing into next low cycle
                subStepHigh = (WT_EFFECTIVE_SIZE / 2) << 16;
                subStepLow = ((oscillator->stepSizeHigh.c - subStepHigh) / oscillator->stepSizeHigh.c) * oscillator->stepSizeLow.c;
                newIndex.c = oscillator->index.c + subStepHigh + subStepLow;
            }
        }
    }

    oscillator->index.c = newIndex.c & WT_INDEX_MASK_32;

    int16_t adjustedIndex = wt_int_getTableIndex(oscillator->index.p.i + oscillator->phase);
    int negative = 0;
    if(adjustedIndex < 0) {
        adjustedIndex *= -1;
        negative = 1;
    }
    int16_t sample1 = (oscillator->swing) * oscillator->waveTable1[adjustedIndex];
    int16_t sample2 = (oscillator->swing) * oscillator->waveTable2[adjustedIndex];
    int16_t output = wt_int_mixSamples(sample1,sample2,oscillator->mix,FOSC_MIX_RESOLUTION);
    if(negative) output *= -1;
    return output;
}
