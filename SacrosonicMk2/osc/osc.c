
#include "osc.h"
#include "../time/time.h"

void osc_updateStepSizeBase(){
    osc_stepSizeBase = osc_pitch / OSC_SAMPLE_RATE;
}

void osc_updateStepSizeHigh(){
    osc_stepSizeHigh = osc_stepSizeBase / (osc_duty * 2);

    if(osc_stepSizeHigh < osc_stepSizeBase / 2) {
        osc_stepSizeHigh = osc_stepSizeBase / 2;
    }
}

void osc_updateStepSizeLow(){
    osc_stepSizeLow = osc_stepSizeBase / ((1-osc_duty) * 2);

    if(osc_stepSizeLow < osc_stepSizeBase / 2) {
        osc_stepSizeLow = osc_stepSizeBase / 2;
    }
}

void osc_updateSwing(){
    osc_swing = (OSC_AMPLITUDE_MAX / 2) * osc_amplitude;
}

void osc_updateDerivatives(){
    osc_updateStepSizeBase();
    osc_updateStepSizeHigh();
    osc_updateStepSizeLow();
    osc_updateSwing();
}

void osc_init(){
    cs43l22_init();
    wt_init();

    osc_pitch = OSC_PITCH_DEFAULT;
    osc_waveform = OSC_WAVEFORM_DEFAULT;
    osc_duty = OSC_DUTY_DEFAULT;
    osc_phase = OSC_PHASE_DEFAULT;
    osc_amplitude = OSC_AMPLITUDE_DEFAULT;

    osc_index = 0.0;
    osc_sample = 0;
    osc_channel = 0;

    osc_updateDerivatives();

    totalGenerationTime = 0;
}

void osc_generateNextSample(){
    // to make this faster and more flexible, think waveform * num_of_waveforms for indexing a table.

    float * waveform1;
    float * waveform2;
    float mix;
    if(osc_waveform <= OSC_WAVEFORM_INTERVAL){
        waveform1 = OSC_WAVEFORM_1;
        waveform2 = OSC_WAVEFORM_2;
        mix = osc_waveform * OSC_NUM_OF_WAVEFORMS;
    } else if(osc_waveform <= OSC_WAVEFORM_INTERVAL * 2){
        waveform1 = OSC_WAVEFORM_2;
        waveform2 = OSC_WAVEFORM_3;
        mix = (osc_waveform - OSC_WAVEFORM_INTERVAL) * OSC_NUM_OF_WAVEFORMS;
    } else if(osc_waveform <= 1.0){
        waveform1 = OSC_WAVEFORM_3;
        waveform2 = OSC_WAVEFORM_1;
        mix = (osc_waveform - OSC_WAVEFORM_INTERVAL * 2) * OSC_NUM_OF_WAVEFORMS;
    } else {
        return;
    }

    float waveform1Sample = wt_getSampleFromTable(waveform1,osc_index,osc_phase);
    float waveform2Sample = wt_getSampleFromTable(waveform2,osc_index,osc_phase);

    osc_sample = wt_mixSamples(waveform1Sample,waveform2Sample,mix) * osc_swing;

    float newIndex = 0.0;

    if(osc_index <= 0.5){
        newIndex = osc_index + osc_stepSizeHigh;

        if(newIndex > 0.5){ // the step goes beyond the duty, the step will have to be part stepSizeHigh and part stepSizeLow
            float subStepHigh = 0.5 - osc_index;
            float subStepLow = (1.0 - (subStepHigh / osc_stepSizeHigh)) * osc_stepSizeLow;
            newIndex = osc_index + subStepHigh + subStepLow;

            if(newIndex >= 1.0){ // the step goes beyond this cycle and into the next, the step will have to be part stepSizeHigh and part stepSizeLow
                subStepLow = 0.5;
                subStepHigh = (1.0 - (subStepLow / osc_stepSizeLow)) * osc_stepSizeHigh;
                newIndex = osc_index + subStepHigh + subStepLow;
            }
        }
    } else {
        newIndex = osc_index + osc_stepSizeLow;

        if(newIndex > 1.0){ // the step goes into a new cycle, the step will have to be part stepSizeLow and part stepSizeHigh
            float subStepLow = 1.0 - osc_index;
            float subStepHigh = (1.0 - (subStepLow / osc_stepSizeLow)) * osc_stepSizeHigh;
            newIndex = osc_index + subStepLow + subStepHigh;

            if(newIndex > 1.5){ // the step goes beyond the duty of the next cycle, the step will have to be part stepSizeLow and part stepSizeHigh
                subStepHigh = 0.5;
                subStepLow = (1.0 - (subStepHigh / osc_stepSizeHigh)) * osc_stepSizeLow;
                newIndex = osc_index + subStepLow + subStepHigh;
            }
        }
    }
    osc_index = newIndex;

    osc_index -= (int)osc_index; // optimized way of performing a modulo 1
}

int osc_attemptOutput(){
    if(osc_channel == 0 && cs43l22_attemptOutputSample(osc_sample)) {
        osc_channel = 1;
        return 1;
    } else if(osc_channel == 1 && cs43l22_attemptOutputSample(osc_sample)) {
        osc_channel = 0;
        uint32_t startTime = time_getTimerTicks();
        osc_generateNextSample();
        totalGenerationTime += time_getTimerTicks() - startTime;
        return 1;
    } else {
        return 0;
    }
}
