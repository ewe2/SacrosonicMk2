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
#include "../leds/leds.h"
#include "../buttons/buttons.h"

#define PITCH_POT 0
#define WAVEFORM_POT 1
#define DUTY_POT 2
#define PHASE_POT 3
#define AMPLITUDE_POT 4

#define PITCH_BOTTOM 40
#define PITCH_RANGE 4000

void testFOscOneShot(uint16_t numberOfTests, uint8_t dutyEnabled) {
    fOsc_struct oscillator;
    oscillator.sampleRate.p.i = 48000;
    oscillator.sampleRate.p.f = 0;
    oscillator.pitch.p.i = 440;
    oscillator.pitch.p.f = 0;
    oscillator.amplitude.p.i = (1 << 14);
    oscillator.amplitude.p.f = 0;
    oscillator.waveTable1 = wt_tri;
    oscillator.waveTable2 = wt_square;
    oscillator.mix = 100;
    oscillator.dutyEnabled = dutyEnabled;
    oscillator.duty = 207;
    fOsc_init(&oscillator);

    uint16_t samples[numberOfTests];
    uint32_t startTime;
    uint32_t totalTime;
    uint32_t i = 0;

    totalTime = 0;
    startTime = timer_getTimerTicks();
    for(i = 0; i < numberOfTests; i++) {
        samples[i] = fOsc_getNextSample(&oscillator);
    }
    totalTime = timer_getTimerTicks() - startTime;
    if(dutyEnabled) printf("duty enabled, set to: %d\n",oscillator.duty);
    printf("time for %d fOsc samples: %d\n",numberOfTests,totalTime);


    for(i = 0; i < numberOfTests; i++) {
        while(!SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE));
        SPI_I2S_SendData(CS43L22_I2S_PORT,samples[i]);
    }

}

void testFOscContinuous(uint8_t numberOfOscillators, uint8_t dutyEnabled, uint8_t potsEnabled, Btn_struct * button) {

    fOsc_struct oscillators[numberOfOscillators];
    int i = 0;
    for(; i < numberOfOscillators; i++) {
        oscillators[i].sampleRate.p.i = 48000;
        oscillators[i].sampleRate.p.f = 0;
        oscillators[i].pitch.p.i = 440;
        oscillators[i].pitch.p.f = 0;
        oscillators[i].amplitude.p.i = (1 << 14);
        oscillators[i].amplitude.p.f = 0;
        oscillators[i].waveTable1 = wt_sine;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].mix = 128;
        oscillators[i].dutyEnabled = dutyEnabled;
        oscillators[i].duty = 0;
        oscillators[i].phase = 0;
        fOsc_init(&oscillators[i]);
    }

    int16_t sample = 0;
    int32_t sampleSum = 0;
    uint8_t channel = 1;
    i = 0;
    uint8_t updateStep = 0;
    uint8_t selectedOscillator = 0;

    while(1) {
        if(SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)) {
            SPI_I2S_SendData(CS43L22_I2S_PORT,sample);
            if(channel == 1) {
                channel = 2;
            } else {
                sample = sampleSum / numberOfOscillators;
                sampleSum = 0;
                channel = 1;

                if(i != numberOfOscillators) printf("UNDERRUN: ONLY %d SAMPLES GENERATED\n",i);
                i = 0;
            }
        }

        if(i < channel * numberOfOscillators / 2) {
            sampleSum += fOsc_getNextSample(&oscillators[i]);
            i++;
        } else if(potsEnabled){
            float newValue = 0.0;
            switch(updateStep++) {
            case 0:
                if(pots_readIfActive(PITCH_POT,&newValue)){
                    oscillators[selectedOscillator].pitch.p.i = newValue * PITCH_RANGE + PITCH_BOTTOM;
                }
                break;
            case 1:
                if(pots_readIfActive(DUTY_POT,&newValue)){
                    oscillators[selectedOscillator].duty = newValue * (FOSC_DUTY_RESOLUTION - 16);
                }
                break;
            case 2:
                fOsc_updateStepSizeBase(&oscillators[selectedOscillator]);
                break;
            case 3:
                fOsc_updateStepSizeHigh(&oscillators[selectedOscillator]);
                break;
            case 4:
                fOsc_updateStepSizeLow(&oscillators[selectedOscillator]);
                break;
            case 5:
                if(pots_readIfActive(WAVEFORM_POT,&newValue)){
                    oscillators[selectedOscillator].mix = newValue * FOSC_MIX_RESOLUTION - 1;
                }
                break;
            case 6:
               if(pots_readIfActive(PHASE_POT,&newValue)){
                    oscillators[selectedOscillator].phase = newValue * WT_EFFECTIVE_SIZE - 1;
                }
                break;
            case 7:
                if(pots_readIfActive(AMPLITUDE_POT,&newValue)){
                    oscillators[selectedOscillator].amplitude.c = newValue * UINT32_MAX;
                }
                break;
            case 8:
                fOsc_updateSwing(&oscillators[selectedOscillator]);
                break;
            default:
                if(btn_readOneShot(button)){
                    selectedOscillator++;
                    if(selectedOscillator >= numberOfOscillators) selectedOscillator = 0;
                    pots_switchFunction();
                }
                updateStep = 0;
                break;
            }
        }
    }
}

void testButtonsAndLeds(Btn_struct * button){
    int ledState = 1;
    while(1){
        if(btn_readOneShot(button)){
            switch(ledState++){
            case 1:
                led_setAll(1,0,0,0);
                break;
            case 2:
                led_setAll(0,1,0,0);
                break;
            case 3:
                led_setAll(0,0,1,0);
                break;
            case 4:
                led_setAll(0,0,0,1);
                break;
            default:
                led_setAll(0,0,0,0);
                return;
            }
        }
    }
}

void testPot(uint8_t pot){

    float potValue = 0.0;
    while(1){
        pots_readIfActive(pot,&potValue);
        if(potValue < 0.25){
            led_setAll(0,0,0,0);
        } else if(potValue < 0.5){
            led_setAll(1,0,0,0);
        } else if(potValue < 0.75){
            led_setAll(1,1,0,0);
        } else if(potValue < 1.0){
            led_setAll(1,1,1,0);
        } else {
            led_setAll(1,1,1,1);
        }
        timer_delay(0.1);
    }
}

int main(void) {
    printf("\f\n");
    osc_init();
    pots_initAndStart();
    timer_init();
    led_init();

    Btn_struct button1;
    button1.debounceTime = TIMER_CLOCK_SPEED / 100;
    button1.portLetter = 'E';
    button1.pin = 7;
    btn_init(&button1);

    //testButtonsAndLeds(&button1);

    testPot(PITCH_POT);

    testFOscOneShot(25000,1);

    testFOscContinuous(2,1,1,&button1);

    while(1);
}
