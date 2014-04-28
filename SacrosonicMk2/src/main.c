/*
**
**                           Main.c
**
**
**********************************************************************/
/*
   Last committed:     $Revision: 00 $
   Last changed by:    $Author: $
   Last changed date:  $Date:  $
   ID:                 $Id:  $

**********************************************************************/

#include "stm32f4xx_conf.h"

#include <stdio.h>

#include "../osc/osc.h"
#include "../pots/pots.h"
#include "../envelope/envelope.h"
#include "../lfo/lfo.h"
#include "../fastOsc/fastOsc.h"
#include "../timer/timer.h"

#define PITCH_POT 0
#define WAVEFORM_POT 1
#define DUTY_POT 2
#define PHASE_POT 3
#define AMPLITUDE_POT 4

#define PITCH_BOTTOM 40
#define PITCH_RANGE 4000


#define NUMBER_OF_TESTS 25000
void testFOscOneShot() {
    fOsc_struct fOsc1;
    fOsc1.sampleRate.p.i = 48000;
    fOsc1.sampleRate.p.f = 0;
    fOsc1.pitch.p.i = 440;
    fOsc1.pitch.p.f = 0;
    fOsc1.amplitude.p.i = (1 << 14);
    fOsc1.amplitude.p.f = 0;
    fOsc1.waveTable1 = wt_tri;
    fOsc1.waveTable2 = wt_square;
    fOsc1.resolution = 128;
    fOsc1.mix.p.i = 100;
    fOsc1.mix.p.f = 0;
    fOsc_init(&fOsc1);

    uint16_t samples[NUMBER_OF_TESTS];
    uint32_t startTime;
    uint32_t totalTime;
    uint32_t i = 0;

    totalTime = 0;
    startTime = timer_getTimerTicks();
    for(i = 0; i < NUMBER_OF_TESTS; i++) {
        samples[i] = fOsc_getNextSample(&fOsc1);
    }
    totalTime = timer_getTimerTicks() - startTime;
    printf("time for %d fOsc samples: %d\n",NUMBER_OF_TESTS,totalTime);

    for(i = 0; i < NUMBER_OF_TESTS; i++) {
        while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE));
        SPI_I2S_SendData(CS43L22_I2S_PORT,samples[i]);
    }

}

#define NUMBER_OF_OSCILLATORS 30
void testFOscContinuous() {
    fOsc_struct oscillators[NUMBER_OF_OSCILLATORS];
    int i = 0;
    for(; i < NUMBER_OF_OSCILLATORS; i++) {
        oscillators[i].sampleRate.p.i = 48000;
        oscillators[i].sampleRate.p.f = 0;
        oscillators[i].pitch.p.i = 440 + i;
        oscillators[i].pitch.p.f = 0;
        oscillators[i].amplitude.p.i = (1 << 14);
        oscillators[i].amplitude.p.f = 0;
        oscillators[i].waveTable1 = wt_tri;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].resolution = 128;
        oscillators[i].mix.p.i = 100;
        oscillators[i].mix.p.f = 0;
        fOsc_init(&oscillators[i]);
    }

    int16_t sample = 0;
    int32_t sampleSum = 0;
    while(1) {
        while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE));
        SPI_I2S_SendData(CS43L22_I2S_PORT,sample);

        sampleSum = 0;
        for(i = 0; i < NUMBER_OF_OSCILLATORS / 2; i++) {
            sampleSum += fOsc_getNextSample(&oscillators[i]);
        }

        while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE));
        SPI_I2S_SendData(CS43L22_I2S_PORT,sample);

        for(i = NUMBER_OF_OSCILLATORS / 2; i < NUMBER_OF_OSCILLATORS; i++) {
            sampleSum += fOsc_getNextSample(&oscillators[i]);
        }
        sample = sampleSum / NUMBER_OF_OSCILLATORS;
    }
}

int main(void) {
    printf("\f\n");
    osc_init();
    pots_initAndStart();
    timer_init();

    testFOscOneShot();

    testFOscContinuous();

    while(1);


    /*
    // TODO: put envelopes and LFOs in arrays to make iterating through them easier.
    env_Envelope * osc1Envelope;
    env_Envelope * osc2Envelope;
    osc1Envelope = malloc(sizeof(env_Envelope));
    osc2Envelope = malloc(sizeof(env_Envelope));
    env_init(osc1Envelope);
    env_init(osc2Envelope);

    osc2Envelope->hold = 0.5;

    lfo_Lfo * osc1LfoPitch;
    lfo_Lfo * osc2LfoPitch;
    lfo_Lfo * osc1LfoDuty;
    lfo_Lfo * osc2LfoPhase;
    osc1LfoPitch = malloc(sizeof(lfo_Lfo));
    osc2LfoPitch = malloc(sizeof(lfo_Lfo));
    osc1LfoDuty = malloc(sizeof(lfo_Lfo));
    osc2LfoPhase = malloc(sizeof(lfo_Lfo));
    lfo_init(osc1LfoPitch);
    lfo_init(osc2LfoPitch);
    lfo_init(osc1LfoDuty);
    lfo_init(osc2LfoPhase);

    osc1LfoPitch->pitch = 0.1;
    osc1LfoPitch->amount = 0.1;

    osc2LfoPitch->pitch = 2;
    osc2LfoPitch->amount = 0.01;

    osc1LfoDuty->pitch = 0.5;
    osc1LfoDuty->amount = 0.3;

    osc2LfoPhase->pitch = 1.3;
    osc2LfoPhase->amount = 0.1;

    Osc_struct * osc = &osc_oscillator1;
    uint8_t updateStep = 0;
    uint32_t lfoAndEnvStep = 0; // note since we output one sample for each channel this actually counts at twice the output sample rate

    while(1) {
        while(!osc_attemptOutput());

        switch(lfoAndEnvStep++){
        case 0 * 2: // * 2 so we can alternate between the env/lfos and the adcs
            if(osc1Envelope->state == ENV_STATE_DEAD) env_trigger(osc1Envelope);
            env_getNextSample(osc1Envelope);
            break;
        case 1 * 2:
            if(osc2Envelope->state == ENV_STATE_DEAD) env_trigger(osc2Envelope);
            env_getNextSample(osc2Envelope);
            break;
        case 2 * 2:
            lfo_getNextSample(osc1LfoPitch);
            break;
        case 3 * 2:
            lfo_getNextSample(osc1LfoPitch);
            break;
        case 4 * 2:
            lfo_getNextSample(osc2LfoPitch);
            break;
        case 5 * 2:
            lfo_getNextSample(osc1LfoDuty);
            break;
        case 6 * 2:
            lfo_getNextSample(osc2LfoPhase);
            break;
        case 31:
            lfoAndEnvStep = 0;
            break;
        default:
            switch(updateStep++){
            case 0:
                if(osc == &osc_oscillator1){
                    osc->pitch = pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM;
                    osc->pitch *= 1.0 + osc1LfoPitch->output;
                } else {
                    osc->pitch = (pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM) * 2;
                    osc->pitch *= 1.0 + osc2LfoPitch->output;
                }
                break;
            case 1:
                osc_updateStepSizeBase(osc);
                break;
            case 2:
                osc_updateStepSizeHigh(osc);
                break;
            case 3:
                osc_updateStepSizeLow(osc);
                break;
            case 4:
                osc->waveform = pots_getMappedAverage(WAVEFORM_POT);
                break;
            case 5:
                osc->duty = pots_getMappedAverage(DUTY_POT);
                if(osc == &osc_oscillator1){
                    osc->duty *= 1.0 + osc1LfoDuty->output;
                }
                break;
            case 6:
                osc->phase = pots_getMappedAverage(PHASE_POT);
                if(osc == &osc_oscillator2){
                    osc->phase *= 1.0 + osc2LfoPhase->output;
                }
                break;
            case 7:
                osc->amplitude = pots_getMappedAverage(AMPLITUDE_POT);
                if(osc == &osc_oscillator1){
                    osc->amplitude *= osc1Envelope->output;
                } else {
                    osc->amplitude *= osc2Envelope->output;
                }
                break;
            default:
                updateStep = 0;
                if(osc == &osc_oscillator1){
                    osc = &osc_oscillator2;
                } else {
                    osc = &osc_oscillator1;
                }
                break;
            }
        }
    }*/
}
