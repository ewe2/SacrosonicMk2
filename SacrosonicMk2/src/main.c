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

#define NUMBER_OF_OSCILLATORS 10
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
        oscillators[i].waveTable1 = wt_sine;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].mix = 128;
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
        for(channel = 0; channel < 2; channel++){
            while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE));
            SPI_I2S_SendData(CS43L22_I2S_PORT,sample);

            sampleSum = 0;
            for(i = (NUMBER_OF_OSCILLATORS / 2) * channel; i < (NUMBER_OF_OSCILLATORS / 2) * (channel + 1); i++) {
                sampleSum += fOsc_getNextSample(&oscillators[i]);
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
    testFOscContinuous();

    while(1);
}
