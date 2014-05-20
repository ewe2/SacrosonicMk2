#include "fastOsc.h"

void fOsc_initModulator(fOsc_modStruct * modulator){
    modulator->modSource = &fOsc_default_mod_src;
    modulator->modAmount = 0;
    modulator->isLog = 0;
}

void fOsc_updateModResult(fOsc_modStruct * modulator) {
    float modulation = *modulator->modSource * modulator->modAmount;
    if(modulator->isLog){
        modulator->modResult = fExp(modulation);
    } else {
        modulator->modResult = modulation;
    }
}

void fOsc_modulatePitch(fOsc_struct* oscillator) {
    oscillator->modulatedPitch.c = oscillator->pitch.c * oscillator->pitchOffset * oscillator->pitchMod.modResult;

    if(oscillator->modulatedPitch.p.i >= FOSC_PITCH_MAX) {
        oscillator->modulatedPitch.p.i = FOSC_PITCH_MAX;
        oscillator->modulatedPitch.p.f = 0;
    }
}

void fOsc_modulateWaveMix(fOsc_struct* oscillator) {
    uint32_t tempMix = oscillator->waveMix + oscillator->waveMixMod.modResult * (FOSC_WAVE_MIX_RESOLUTION - 1);
    if(tempMix >= FOSC_WAVE_MIX_RESOLUTION) tempMix = FOSC_WAVE_MIX_RESOLUTION - 1;
    oscillator->modulatedWaveMix = tempMix;
}

void fOsc_modulateDuty(fOsc_struct* oscillator) {
    uint32_t tempDuty = oscillator->duty + oscillator->dutyMod.modResult * (FOSC_DUTY_RESOLUTION - 1);
    if(tempDuty >= FOSC_DUTY_RESOLUTION) tempDuty = FOSC_DUTY_RESOLUTION - 1;
    oscillator->modulatedDuty = tempDuty;
}

void fOsc_modulatePhase(fOsc_struct* oscillator) {
    oscillator->modulatedPhase = oscillator->phase + oscillator->phaseMod.modResult * (WT_EFFECTIVE_SIZE - 1);
    if(oscillator->modulatedPhase >= WT_EFFECTIVE_SIZE) oscillator->modulatedPhase = WT_EFFECTIVE_SIZE -1;
}

void fOsc_modulateAmplitude(fOsc_struct* oscillator) {
    oscillator->modulatedAmplitude = oscillator->amplitude * oscillator->amplitudeMod.modResult;
    if(oscillator->modulatedAmplitude > FOSC_AMPLITUDE_MAX) oscillator->modulatedAmplitude = FOSC_AMPLITUDE_MAX;
}

void fOsc_updateSwing(fOsc_struct * oscillator) {
    oscillator->swing = (oscillator->modulatedAmplitude / 2);
}

void fOsc_updateStepSizeBase(fOsc_struct * oscillator) {
    oscillator->stepSizeBase.c = (oscillator->modulatedPitch.c / oscillator->sampleRate.p.i) * WT_EFFECTIVE_SIZE;
}

void fOsc_updateStepSizeHigh(fOsc_struct * oscillator) {
    oscillator->stepSizeHigh.c = (oscillator->stepSizeBase.c / (FOSC_DUTY_RESOLUTION - oscillator->modulatedDuty)) * FOSC_DUTY_RESOLUTION;
}

void fOsc_updateStepSizeLow(fOsc_struct * oscillator) {
    oscillator->stepSizeLow.c = (oscillator->stepSizeBase.c / (FOSC_DUTY_RESOLUTION + oscillator->modulatedDuty))  * FOSC_DUTY_RESOLUTION;
}

void fOsc_updateDerivatives(fOsc_struct * oscillator) {
    fOsc_updateModResult(&oscillator->pitchMod);
    fOsc_updateModResult(&oscillator->waveMixMod);
    fOsc_updateModResult(&oscillator->dutyMod);
    fOsc_updateModResult(&oscillator->phaseMod);
    fOsc_updateModResult(&oscillator->amplitudeMod);

    fOsc_modulatePitch(oscillator);
    fOsc_modulateWaveMix(oscillator);
    fOsc_modulateDuty(oscillator);
    fOsc_modulatePhase(oscillator);
    fOsc_modulateAmplitude(oscillator);

    fOsc_updateSwing(oscillator);
    fOsc_updateStepSizeBase(oscillator);
    fOsc_updateStepSizeHigh(oscillator);
    fOsc_updateStepSizeLow(oscillator);
}

void fOsc_init(fOsc_struct * oscillator, uint8_t init) {
    if(oscillator->sampleRate.p.i < 8000 || init) {
        oscillator->sampleRate.p.i = 48000;
    }

    if(oscillator->pitch.p.i >= FOSC_PITCH_MAX || init) {
        oscillator->pitch.p.i = 440;
        oscillator->pitch.p.f = 0;
    }

    if(oscillator->pitchOffset < 0 || oscillator->pitchOffset > 10 || init) {
        oscillator->pitchOffset = 1.0;
    }

    if((oscillator->waveTable1 != wt_sine && oscillator->waveTable1 != wt_tri && oscillator->waveTable1 != wt_square) || init) {
        oscillator->waveTable1 = wt_sine;
    }
    if((oscillator->waveTable2 != wt_sine && oscillator->waveTable2 != wt_tri && oscillator->waveTable2 != wt_square )|| init) {
        oscillator->waveTable2 = wt_square;
    }

    if(oscillator->dutyEnabled > 1 || init) { // should be 1 for on and 0 for off, if it is more than 1, value has not yet been initialized
        oscillator->dutyEnabled = 0;
    }

    if(oscillator->amplitude > FOSC_AMPLITUDE_MAX || init){
        oscillator->amplitude = FOSC_AMPLITUDE_MAX;
    }

    if(init) {
        oscillator->waveMix = 0;
        oscillator->duty = 0;
        oscillator->phase = 0;
    }

    fOsc_default_mod_src = 1.0;

    fOsc_initModulator(&oscillator->pitchMod);
    fOsc_initModulator(&oscillator->waveMixMod);
    fOsc_initModulator(&oscillator->dutyMod);
    fOsc_initModulator(&oscillator->phaseMod);
    fOsc_initModulator(&oscillator->amplitudeMod);

    oscillator->pitchMod.isLog = 1;
    oscillator->amplitudeMod.isLog = 1;

    oscillator->index.c = 0;

    fOsc_updateDerivatives(oscillator);
}

int16_t fOsc_getNextSample(fOsc_struct * oscillator) {
    if(oscillator->dutyEnabled) {
        FixedPoint newIndex;
        newIndex.c = 0;

        if(oscillator->index.p.i < WT_NODE_INDEX) { // starting in the high cycle
            newIndex.c = oscillator->index.c + oscillator->stepSizeHigh.c;

            if(newIndex.p.i > WT_NODE_INDEX) { // passing into the low cycle
                uint32_t subStepHigh = (WT_NODE_INDEX - oscillator->index.p.i) << 16;
                uint32_t subStepLow = ((oscillator->stepSizeHigh.c - subStepHigh) * (float)oscillator->stepSizeLow.c) / oscillator->stepSizeHigh.c;
                newIndex.c = oscillator->index.c + subStepHigh + subStepLow;

                if(newIndex.p.i > WT_INDEX_MAX) { // passing into next high cycle
                    subStepLow = (WT_EFFECTIVE_SIZE << 16) / 2;
                    subStepHigh = ((oscillator->stepSizeLow.c - subStepLow) * (float)oscillator->stepSizeHigh.c) / oscillator->stepSizeLow.c;
                    newIndex.c = oscillator->index.c + subStepHigh + subStepLow;
                }
            }
        } else { // starting in the low cycle
            newIndex.c = oscillator->index.c + oscillator->stepSizeLow.c;

            if(newIndex.p.i > WT_INDEX_MAX) { // passing into the high cycle
                uint32_t subStepLow = (WT_INDEX_MAX - oscillator->index.p.i) << 16;
                uint32_t subStepHigh = ((oscillator->stepSizeLow.c - subStepLow) * (float)oscillator->stepSizeHigh.c) / oscillator->stepSizeLow.c;
                newIndex.c = oscillator->index.c + subStepHigh + subStepLow;

                if(newIndex.p.i > WT_INDEX_MAX + WT_NODE_INDEX) { // passing into next low cycle
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

    int16_t adjustedIndex = wt_int_getTableIndex(oscillator->index.p.i + oscillator->modulatedPhase);
    int negative = 0;
    if(adjustedIndex < 0) {
        adjustedIndex *= -1;
        negative = 1;
    }
    int16_t sample1 = (oscillator->swing) * oscillator->waveTable1[adjustedIndex];
    int16_t sample2 = (oscillator->swing) * oscillator->waveTable2[adjustedIndex];
    int16_t output = wt_int_mixSamples(sample1,sample2,oscillator->modulatedWaveMix,FOSC_WAVE_MIX_RESOLUTION);
    if(negative) output *= -1;
    return output;
}
