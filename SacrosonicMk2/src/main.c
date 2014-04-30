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
    fOsc1.mix = 100;
    fOsc1.dutyEnabled = 1;
    fOsc1.duty = 207;
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

void testFOscContinuous() {
    const uint8_t NUMBER_OF_OSCILLATORS = 10;

    fOsc_struct oscillators[NUMBER_OF_OSCILLATORS];
    int i = 0;
    for(; i < NUMBER_OF_OSCILLATORS; i++) {
        oscillators[i].sampleRate.p.i = 48000;
        oscillators[i].sampleRate.p.f = 0;
        oscillators[i].pitch.p.i = 440 + i;
        oscillators[i].pitch.p.f = 0;
        oscillators[i].amplitude.p.i = (1 << 14);
        oscillators[i].amplitude.p.f = 0;
        oscillators[i].waveTable1 = wt_sine;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].mix = 128;
        oscillators[i].dutyEnabled = 0;
        oscillators[i].duty = 0;
        oscillators[i].phase = 0;
        fOsc_init(&oscillators[i]);
    }

    int16_t sample = 0;
    int32_t sampleSum = 0;
    uint8_t channel = 0;
    while(1) {
        sample = sampleSum / NUMBER_OF_OSCILLATORS;
        sampleSum = 0;
        for(channel = 0; channel < 2; channel++) {
            while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE));
            SPI_I2S_SendData(CS43L22_I2S_PORT,sample);

            for(i = (NUMBER_OF_OSCILLATORS / 2) * channel; i < (NUMBER_OF_OSCILLATORS / 2) * (channel + 1); i++) {
                sampleSum += fOsc_getNextSample(&oscillators[i]);
            }
        }
    }
}

void testFOscContinuousWithPots() {
    const uint8_t NUMBER_OF_OSCILLATORS = 12;

    fOsc_struct oscillators[NUMBER_OF_OSCILLATORS];
    int i = 0;
    for(; i < NUMBER_OF_OSCILLATORS; i++) {
        oscillators[i].sampleRate.p.i = 48000;
        oscillators[i].sampleRate.p.f = 0;
        oscillators[i].pitch.p.i = 440;
        oscillators[i].pitch.p.f = 0;
        oscillators[i].amplitude.p.i = (1 << 14);
        oscillators[i].amplitude.p.f = 0;
        oscillators[i].waveTable1 = wt_sine;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].mix = 128;
        oscillators[i].dutyEnabled = 1;
        oscillators[i].duty = 0;
        oscillators[i].phase = 0;
        fOsc_init(&oscillators[i]);
    }

    int16_t sample = 0;
    int32_t sampleSum = 0;
    uint8_t channel = 1;
    i = 0;
    uint8_t updateStep = 0;
    uint8_t j = 0;
    while(1) {
        if(SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)) {
            SPI_I2S_SendData(CS43L22_I2S_PORT,sample);
            if(channel == 1) {
                channel = 2;
            } else {
                sample = sampleSum / NUMBER_OF_OSCILLATORS;
                sampleSum = 0;
                channel = 1;

                if(i != NUMBER_OF_OSCILLATORS) printf("UNDERRUN: ONLY %d SAMPLES GENERATED\n",i);
                i = 0;
            }
        }

        if(i < channel * NUMBER_OF_OSCILLATORS / 2) {
            sampleSum += fOsc_getNextSample(&oscillators[i]);
            i++;
        } else {

            switch(updateStep++) {
            case 0:
                oscillators[j].pitch.p.i = pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM;
                break;
            case 1:
                oscillators[j].duty = pots_getMappedAverage(DUTY_POT) * (FOSC_DUTY_RESOLUTION - 16);
                break;
            case 2:
                fOsc_updateStepSizeBase(&oscillators[j]);
                break;
            case 3:
                fOsc_updateStepSizeHigh(&oscillators[j]);
                break;
            case 4:
                fOsc_updateStepSizeLow(&oscillators[j]);
                break;
            case 5:
                oscillators[j].mix = pots_getMappedAverage(WAVEFORM_POT) * FOSC_MIX_RESOLUTION - 1;
                break;
            case 6:
                oscillators[j].phase = pots_getMappedAverage(PHASE_POT) * WT_EFFECTIVE_SIZE - 1;
                break;
            case 7:
                oscillators[j].amplitude.c = pots_getMappedAverage(AMPLITUDE_POT) * UINT32_MAX;
                break;
            case 8:
                fOsc_updateSwing(&oscillators[j]);
                break;
            default:
                updateStep = 0;
                j++;
                if(j == NUMBER_OF_OSCILLATORS) j = 0;
                break;
            }
        }
    }
}

int main(void) {
    printf("\f\n");
    osc_init();
    pots_initAndStart();
    timer_init();

    testFOscOneShot();
    testFOscContinuousWithPots();
    //testFOscContinuous();

    while(1);
}
