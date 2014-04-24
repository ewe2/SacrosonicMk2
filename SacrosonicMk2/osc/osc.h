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

#define OSC_AMPLITUDE_MAX (1 << 15) // about half amplitude, which is actually pretty loud

#define OSC_PITCH_DEFAULT 440.0
#define OSC_WAVEFORM_DEFAULT 0.0
#define OSC_DUTY_DEFAULT 0.5;
#define OSC_PHASE_DEFAULT 0.0
#define OSC_AMPLITUDE_DEFAULT (OSC_AMPLITUDE_MAX / 2)

// 'public' variables
float osc_pitch; // pitch in Hz
float osc_waveform; // waveform ranging from 0.0 to 1.0
float osc_duty; // duty ranging from 0.0 to 1.0
float osc_phase; // phase ranging from 0.0 to 1.0
float osc_amplitude; // amplitude ranging from 0 to 1.0
// 'private' variables
float osc_stepSizeBase;
float osc_stepSizeHigh;
float osc_stepSizeLow;
int16_t osc_swing;

float osc_index;
int16_t osc_sample;
uint8_t osc_channel;

void osc_updateStepSizeBase();
void osc_updateStepSizeHigh();
void osc_updateStepSizeLow();
void osc_updateSwing();

void osc_updateDerivatives();

void osc_init();

void osc_generateNextSample(); // generates a new sample, incrementing the index and updating the osc_sample variable

int osc_attemptOutput(); // makes one attempt to output a sample, should be called repeatedly, returns 1 if successful, 0 if not.

uint32_t totalGenerationTime;

#endif /* OSC_H_INCLUDED */
