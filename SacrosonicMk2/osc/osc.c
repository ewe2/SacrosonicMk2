
#include "osc.h"

void osc_updateStepSizeBase(Osc_struct * osc){
    osc->stepSizeBase = osc->pitch / OSC_SAMPLE_RATE;
}

void osc_updateStepSizeHigh(Osc_struct * osc){
    osc->stepSizeHigh = osc->stepSizeBase / (osc->duty * 2);
}

void osc_updateStepSizeLow(Osc_struct * osc){
    osc->stepSizeLow = osc->stepSizeBase / ((1-osc->duty) * 2);
}

void osc_updateDerivatives(Osc_struct * osc){
    osc_updateStepSizeBase(osc);
    osc_updateStepSizeHigh(osc);
    osc_updateStepSizeLow(osc);
}

void osc_init(){
    cs43l22_init();
    wt_init();
    osc_channel = 0;
    osc_initOscillator(&osc_oscillator1);
    osc_initOscillator(&osc_oscillator2);
}

void osc_initOscillator(Osc_struct * osc){
    osc->pitch = OSC_PITCH_DEFAULT;
    osc->waveform = OSC_WAVEFORM_DEFAULT;
    osc->duty = OSC_DUTY_DEFAULT;
    osc->phase = OSC_PHASE_DEFAULT;
    osc->amplitude = OSC_AMPLITUDE_DEFAULT;

    osc->index = 0.0;
    osc->sample = 0;

    osc_updateDerivatives(osc);
}

void osc_generateNextSample(Osc_struct * osc){
    // to make this faster and more flexible, think waveform * num_of_waveforms for indexing a table.

    float * waveform1;
    float * waveform2;
    float mix;
    if(osc->waveform <= OSC_WAVEFORM_INTERVAL){
        waveform1 = OSC_WAVEFORM_1;
        waveform2 = OSC_WAVEFORM_2;
        mix = osc->waveform * OSC_NUM_OF_WAVEFORMS;
    } else if(osc->waveform <= OSC_WAVEFORM_INTERVAL * 2){
        waveform1 = OSC_WAVEFORM_2;
        waveform2 = OSC_WAVEFORM_3;
        mix = (osc->waveform - OSC_WAVEFORM_INTERVAL) * OSC_NUM_OF_WAVEFORMS;
    } else if(osc->waveform <= 1.0){
        waveform1 = OSC_WAVEFORM_3;
        waveform2 = OSC_WAVEFORM_1;
        mix = (osc->waveform - OSC_WAVEFORM_INTERVAL * 2) * OSC_NUM_OF_WAVEFORMS;
    } else {
        return;
    }

    float waveform1Sample = wt_getSampleFromTable(waveform1,osc->index,osc->phase);
    float waveform2Sample = wt_getSampleFromTable(waveform2,osc->index,osc->phase);

    osc->sample = wt_mixSamples(waveform1Sample,waveform2Sample,mix) * osc->amplitude;

    float newIndex = 0.0;

    if(osc->index <= 0.5){
        newIndex = osc->index + osc->stepSizeHigh;

        if(newIndex > 0.5){ // the step goes beyond the duty, the step will have to be part stepSizeHigh and part stepSizeLow
            float subStepHigh = 0.5 - osc->index;
            float subStepLow = (1.0 - (subStepHigh / osc->stepSizeHigh)) * osc->stepSizeLow;
            newIndex = osc->index + subStepHigh + subStepLow;

            if(newIndex > 1.0){ // the step goes beyond this cycle and into the next, the step will have to be part stepSizeHigh and part stepSizeLow
                subStepLow = 0.5;
                subStepHigh = (1.0 - (subStepLow / osc->stepSizeLow)) * osc->stepSizeHigh;
                newIndex = osc->index + subStepHigh + subStepLow;
            }
        }
    } else {
        newIndex = osc->index + osc->stepSizeLow;

        if(newIndex > 1.0){ // the step goes into a new cycle, the step will have to be part stepSizeLow and part stepSizeHigh
            float subStepLow = 1.0 - osc->index;
            float subStepHigh = (1.0 - (subStepLow / osc->stepSizeLow)) * osc->stepSizeHigh;
            newIndex = osc->index + subStepLow + subStepHigh;

            if(newIndex > 1.5){ // the step goes beyond the duty of the next cycle, the step will have to be part stepSizeLow and part stepSizeHigh
                subStepHigh = 0.5;
                subStepLow = (1.0 - (subStepHigh / osc->stepSizeHigh)) * osc->stepSizeLow;
                newIndex = osc->index + subStepLow + subStepHigh;
            }
        }
    }
    osc->index = newIndex;

    osc->index -= (int)osc->index; // optimized way of performing a modulo 1
}

int osc_attemptOutput(){
    if(osc_channel == 0 && cs43l22_attemptOutputSample(osc_sample)) {
        osc_channel = 1;
        osc_generateNextSample(&osc_oscillator1);
        return 1;
    } else if(osc_channel == 1 && cs43l22_attemptOutputSample(osc_sample)) {
        osc_channel = 0;
        osc_generateNextSample(&osc_oscillator2);
        osc_sample = (osc_oscillator1.sample + osc_oscillator2.sample) * OSC_SWING / 2;
        return 1;
    } else {
        return 0;
    }
}
