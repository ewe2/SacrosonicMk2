#include "fastOsc.h"

void fOsc_updateSwing(fOsc_struct * oscillator) {
    oscillator->swing = oscillator->amplitude.p.i / 2;
}

void fOsc_updateStepSizeBase(fOsc_struct * oscillator) {
    oscillator->stepSizeBase.c = ((float)oscillator->pitch.c / oscillator->sampleRate.p.i) * WT_EFFECTIVE_SIZE;
}

void fOsc_updateStepSizeHigh(fOsc_struct * oscillator) {
    oscillator->stepSizeHigh.c = (oscillator->stepSizeBase.c / (FOSC_DUTY_RESOLUTION - oscillator->duty)) * FOSC_DUTY_RESOLUTION;
}

void fOsc_updateStepSizeLow(fOsc_struct * oscillator) {
    oscillator->stepSizeLow.c = (oscillator->stepSizeBase.c / (FOSC_DUTY_RESOLUTION + oscillator->duty))  * FOSC_DUTY_RESOLUTION;
}

void fOsc_updateDerivatives(fOsc_struct * oscillator) {
    fOsc_updateSwing(oscillator);
    fOsc_updateStepSizeBase(oscillator);
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

    if(oscillator->dutyEnabled > 1){ // should be 1 for on and 0 for off, if it is more than 1, value has not yet been initialized
        oscillator->dutyEnabled = 0;
    }

    // not initializing duty because it can't break anything

    // not initializing phase because it can't break anything

    oscillator->index.c = 0;

    fOsc_updateDerivatives(oscillator);
}

int16_t fOsc_getNextSample(fOsc_struct * oscillator) {
    if(oscillator->dutyEnabled){
        FixedPoint newIndex;
        newIndex.c = 0;

        if(oscillator->index.p.i < WT_NODE_INDEX){ // starting in the high cycle
            newIndex.c = oscillator->index.c + oscillator->stepSizeHigh.c;

            if(newIndex.p.i > WT_NODE_INDEX){ // passing into the low cycle
                uint32_t subStepHigh = (WT_NODE_INDEX - oscillator->index.p.i) << 16;
                uint32_t subStepLow = ((oscillator->stepSizeHigh.c - subStepHigh) * (float)oscillator->stepSizeLow.c) / oscillator->stepSizeHigh.c;
                newIndex.c = oscillator->index.c + subStepHigh + subStepLow;

                if(newIndex.p.i > WT_INDEX_MAX){ // passing into next high cycle
                    subStepLow = (WT_EFFECTIVE_SIZE << 16) / 2;
                    subStepHigh = ((oscillator->stepSizeLow.c - subStepLow) * (float)oscillator->stepSizeHigh.c) / oscillator->stepSizeLow.c;
                    newIndex.c = oscillator->index.c + subStepHigh + subStepLow;
                }
            }
        } else { // starting in the low cycle
            newIndex.c = oscillator->index.c + oscillator->stepSizeLow.c;

            if(newIndex.p.i > WT_INDEX_MAX){ // passing into the high cycle
                uint32_t subStepLow = (WT_INDEX_MAX - oscillator->index.p.i) << 16;
                uint32_t subStepHigh = ((oscillator->stepSizeLow.c - subStepLow) * (float)oscillator->stepSizeHigh.c) / oscillator->stepSizeLow.c;
                newIndex.c = oscillator->index.c + subStepHigh + subStepLow;

                if(newIndex.p.i > WT_INDEX_MAX + WT_NODE_INDEX){ // passing into next low cycle
                    subStepHigh = (WT_EFFECTIVE_SIZE << 16) / 2;
                    subStepLow = ((oscillator->stepSizeHigh.c - subStepHigh) * (float)oscillator->stepSizeLow.c) / oscillator->stepSizeHigh.c;
                    newIndex.c = oscillator->index.c + subStepHigh + subStepLow;
                }
            }
        }

        oscillator->index.c = newIndex.c & WT_INDEX_MASK_32;
    } else {
        oscillator->index.c += oscillator->stepSizeBase.c;
    }

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
