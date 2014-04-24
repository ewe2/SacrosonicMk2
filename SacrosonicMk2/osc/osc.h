#ifndef OSC_H_INCLUDED
#define OSC_H_INCLUDED

#include "../cs43l22/cs43l22.h"
#include "../wavetable/wavetable.h"

#include <stdint.h>

#define OSC_SAMPLE_RATE (CS43L22_SAMPLE_RATE)

#define OSC_PITCH_MAX 20000

#define OSC_NUM_OF_WAVEFORMS 3
#define OSC_WAVEFORM_1 wt_tri
#define OSC_WAVEFORM_2 wt_sine
#define OSC_WAVEFORM_3 wt_square
#define OSC_WAVEFORM_INTERVAL (1.0 / OSC_NUM_OF_WAVEFORMS)

#define OSC_SWING (1 << 15) // about half of what the DAC can output, which is actually pretty loud

#define OSC_PITCH_DEFAULT 440.0
#define OSC_WAVEFORM_DEFAULT 0.0
#define OSC_DUTY_DEFAULT 0.5;
#define OSC_PHASE_DEFAULT 0.0
#define OSC_AMPLITUDE_DEFAULT 1.0

typedef struct {
    // 'public' variables
    float pitch; // pitch in Hz
    float waveform; // waveform ranging from 0.0 to 1.0
    float duty; // duty ranging from 0.0 to 1.0
    float phase; // phase ranging from 0.0 to 1.0
    float amplitude; // amplitude ranging from 0 to 1.0
    // 'private' variables
    float stepSizeBase; // base stepsize, before duty-cycle adjustments
    float stepSizeHigh; // stepsize of the high cycle
    float stepSizeLow; // stepsize of the low cycle

    float index; // index for wavetables
    float sample; // current sample
} Osc_struct;

Osc_struct osc_oscillator1;
Osc_struct osc_oscillator2;

uint8_t osc_channel;

int16_t osc_sample;


void osc_updateStepSizeBase(Osc_struct * osc);
void osc_updateStepSizeHigh(Osc_struct * osc);
void osc_updateStepSizeLow(Osc_struct * osc);

void osc_updateDerivatives(Osc_struct * osc);

void osc_init();

void osc_initOscillator(Osc_struct * osc);

void osc_generateNextSample(Osc_struct * osc); // generates a new sample, incrementing the index and updating the sample variable

int osc_attemptOutput(); // makes one attempt to output a sample, should be called repeatedly, returns 1 if successful, 0 if not.


#endif /* OSC_H_INCLUDED */
