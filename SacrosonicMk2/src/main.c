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

#include "../cs43l22/cs43l22.h"
#include "../pots/pots.h"
#include "../timer/timer.h"
#include "../leds/leds.h"
#include "../buttons/buttons.h"
#include "../midi/midi.h"
#include "../poly/poly.h"
#include <math.h>
#include "../fastExp/fExp.h"
#include "../HD44780/HD44780.h"

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
    oscillator.amplitude = (1 << 14);
    oscillator.waveTable1 = wt_tri;
    oscillator.waveTable2 = wt_square;
    oscillator.waveMix = 100;
    oscillator.dutyEnabled = dutyEnabled;
    oscillator.duty = 207;
    fOsc_init(&oscillator,0);

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
        oscillators[i].amplitude = (1 << 14);
        oscillators[i].waveTable1 = wt_sine;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].waveMix = 128;
        oscillators[i].dutyEnabled = dutyEnabled;
        oscillators[i].duty = 0;
        oscillators[i].phase = 0;
        fOsc_init(&oscillators[i],0);
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
        } else if(potsEnabled) {
            float newValue = 0.0;
            switch(updateStep++) {
            case 0:
                if(pots_readIfActive(PITCH_POT,&newValue)) {
                    oscillators[selectedOscillator].pitch.p.i = newValue * PITCH_RANGE + PITCH_BOTTOM;
                }
                break;
            case 1:
                if(pots_readIfActive(DUTY_POT,&newValue)) {
                    oscillators[selectedOscillator].duty = newValue * FOSC_DUTY_MAX;
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
                if(pots_readIfActive(WAVEFORM_POT,&newValue)) {
                    oscillators[selectedOscillator].waveMix = newValue * FOSC_WAVE_MIX_RESOLUTION - 1;
                }
                break;
            case 6:
                if(pots_readIfActive(PHASE_POT,&newValue)) {
                    oscillators[selectedOscillator].phase = newValue * WT_EFFECTIVE_SIZE - 1;
                }
                break;
            case 7:
                if(pots_readIfActive(AMPLITUDE_POT,&newValue)) {
                    oscillators[selectedOscillator].amplitude = newValue * FOSC_AMPLITUDE_MAX;
                }
                break;
            case 8:
                fOsc_updateSwing(&oscillators[selectedOscillator]);
                break;
            default:
                if(btn_readOneShot(button)) {
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

void testFOscContinuousWithMidi(uint8_t numberOfOscillators, uint8_t dutyEnabled, uint8_t potsEnabled, Btn_struct * button) {

    fOsc_struct oscillators[numberOfOscillators];
    int i = 0;
    for(; i < numberOfOscillators; i++) {
        oscillators[i].sampleRate.p.i = 48000;
        oscillators[i].sampleRate.p.f = 0;
        oscillators[i].pitch.p.i = 440;
        oscillators[i].pitch.p.f = 0;
        oscillators[i].pitchOffset = 1.0;
        oscillators[i].amplitude = (1 << 14);
        oscillators[i].waveTable1 = wt_sine;
        oscillators[i].waveTable2 = wt_square;
        oscillators[i].waveMix = 128;
        oscillators[i].dutyEnabled = dutyEnabled;
        oscillators[i].duty = 0;
        oscillators[i].phase = 0;
        fOsc_init(&oscillators[i],0);
    }

    Env_envelope env;
    env_init(&env);

    int16_t sample = 0;
    int32_t sampleSum = 0;
    uint8_t channel = 1;
    i = 0;
    uint8_t updateStep = 0;
    uint8_t selectedOscillator = 0;

    float envSample = 0.0;

    midi_init();

    Midi_basicMsg midiMsg;
    while(1) {
        if(SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)) {
            SPI_I2S_SendData(CS43L22_I2S_PORT,sample);
            if(channel == 1) {
                channel = 2;
            } else {
                channel = 1;

                sample = (sampleSum / numberOfOscillators);
                sampleSum = 0;

                if(i != numberOfOscillators) printf("UNDERRUN: ONLY %d SAMPLES GENERATED\n",i);
                i = 0;
            }
        }

        if(i < channel * numberOfOscillators / 2) {
            sampleSum += envSample * fOsc_getNextSample(&oscillators[i]);
            i++;
        } else if(potsEnabled) {
            float newValue = 0.0;
            fOsc_struct * oscillator = &oscillators[selectedOscillator];
            switch(updateStep++) {
            case 0:
                if(pots_readIfActive(PITCH_POT,&newValue)) {
                    oscillator->pitchOffset = newValue * 3 + 0.5;
                }
                break;
            case 1:
                if(pots_readIfActive(DUTY_POT,&newValue)) {
                    oscillator->duty = newValue * (FOSC_DUTY_MAX);
                }
                break;
            case 2:
                fOsc_updateStepSizeBase(oscillator);
                break;
            case 3:
                fOsc_updateStepSizeHigh(oscillator);
                break;
            case 4:
                fOsc_updateStepSizeLow(oscillator);
                break;
            case 5:
                if(pots_readIfActive(WAVEFORM_POT,&newValue)) {
                    oscillator->waveMix = newValue * FOSC_WAVE_MIX_RESOLUTION - 1;
                }
                break;
            case 6:
                if(pots_readIfActive(PHASE_POT,&newValue)) {
                    oscillator->phase = newValue * WT_EFFECTIVE_SIZE - 1;
                }
                break;
            case 7:
                if(pots_readIfActive(AMPLITUDE_POT,&newValue)) {
                    oscillator->amplitude = newValue * FOSC_AMPLITUDE_MAX;
                }
                break;
            case 8:
                fOsc_updateSwing(oscillator);
                break;
            case 9:
                envSample = env_getNextSample(&env);
                break;
            case 10:
                if(!midi_getMsgIfAble(&midiMsg)) {
                    midiMsg.msgType = 0;
                }
                break;
            case 11:
                if(midiMsg.msgType == MIDI_MSG_TYPE_NOTE_ON) {
                    if(midiMsg.dataBytes[1] == 0) {
                        env_release(&env);
                    } else {
                        oscillator->pitch.c = midi_notes[midiMsg.dataBytes[0]].c;
                        env_trigger(&env);
                    }
                } else if(midiMsg.msgType == MIDI_MSG_TYPE_NOTE_OFF) {
                    env_release(&env);
                }
                break;
            case 12:
                if(btn_readOneShot(button)) {
                    selectedOscillator++;
                    if(selectedOscillator >= numberOfOscillators) selectedOscillator = 0;
                    pots_switchFunction();
                }
                break;
            default:
                updateStep = 0;
                break;
            }
        }
    }
}

void testFOscContinuousPolyphonicMidi(Btn_struct * button) {
    ply_init();
    midi_init();

    int16_t sample = 0;
    uint8_t channel = 1;
    uint8_t updateStep = 0;
    uint8_t selectedOscillator = 0;
    float newValue = 0.0;

    Midi_basicMsg midiMsg;

    while(1) {
        if(SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)) {
            SPI_I2S_SendData(CS43L22_I2S_PORT,sample);
            if(channel == 1) {
                channel = 2;
            } else {
                channel = 1;
                sample = ply_getNextSample();
            }
        }

        switch(updateStep++) {
        case 0:
            if(pots_readIfActive(PITCH_POT,&newValue)) {
                ply_setPitchOffset(selectedOscillator, newValue * 3 + 0.5);
            }
            break;
        case 1:
            if(pots_readIfActive(WAVEFORM_POT,&newValue)) {
                ply_setWaveMix(selectedOscillator, newValue * FOSC_WAVE_MIX_RESOLUTION - 1);
            }
            break;
        case 2:
            if(pots_readIfActive(DUTY_POT,&newValue)) {
                ply_setDuty(selectedOscillator, newValue * FOSC_DUTY_MAX);
            }
            break;
        case 3:
            if(pots_readIfActive(PHASE_POT,&newValue)) {
                ply_setPhase(selectedOscillator, newValue * WT_EFFECTIVE_SIZE - 1);
            }
            break;
        case 4:
            if(pots_readIfActive(AMPLITUDE_POT,&newValue)) {
                ply_setAmplitude(selectedOscillator, newValue * FOSC_AMPLITUDE_MAX);
            }
            break;
        case 5:
            if(btn_readOneShot(button)) {
                selectedOscillator++;
                if(selectedOscillator >= VOC_OSCILLATORS_PER_VOICE) selectedOscillator = 0;
                pots_switchFunction();
                if(selectedOscillator == 0) led_setAll(1,0,0,0);
                else if(selectedOscillator == 1) led_setAll(1,1,0,0);
                else if(selectedOscillator == 2) led_setAll(1,1,1,0);
            }
            break;
        //========= 6-28 gets to do this
        default:
            ply_makeUpdateStep();
            break;
        //=========
        case 29:
            if(!midi_getMsgIfAble(&midiMsg)) {
                midiMsg.msgType = 0;
            }
            break;
        case 30:
            if(midiMsg.msgType == MIDI_MSG_TYPE_NOTE_ON) {
                if(midiMsg.dataBytes[1] == 0) {
                    ply_noteOff(midiMsg.dataBytes[0]);
                } else {
                    ply_noteOn(midiMsg.dataBytes[0]);
                }
            } else if(midiMsg.msgType == MIDI_MSG_TYPE_NOTE_OFF) {
                ply_noteOff(midiMsg.dataBytes[0]);
            }
            break;
        case 31:
            updateStep = 0;
            break;
        }

    }
}

void testButtonsAndLeds(Btn_struct * button) {
    int ledState = 1;
    while(1) {
        if(btn_readOneShot(button)) {
            switch(ledState++) {
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

void testPot(uint8_t pot) {

    float potValue = 0.0;
    while(1) {
        pots_readIfActive(pot,&potValue);
        if(potValue < 0.25) {
            led_setAll(0,0,0,0);
        } else if(potValue < 0.5) {
            led_setAll(1,0,0,0);
        } else if(potValue < 0.75) {
            led_setAll(1,1,0,0);
        } else if(potValue < 1.0) {
            led_setAll(1,1,1,0);
        } else {
            led_setAll(1,1,1,1);
        }
        timer_delay(0.1);
    }
}

void testExponent(){
    int numberOfTests = 0;

    uint32_t totalTimeExp2f = 0;
    uint32_t totalTimeFExp = 0;
    uint32_t startTime = 0;

    float input = 0;
    float outputExp2f = 0;
    float outputFExp = 0;

    float deviationTotal = 0.0;

    int semitoneStep = -630;
    for(; semitoneStep < 640; semitoneStep += 1){
        input = ((float)semitoneStep / 10.0) / 12.0;

        startTime = timer_getTimerTicks();

        outputExp2f = exp2f(input);

        totalTimeExp2f += timer_getTimerTicks() - startTime;


        startTime = timer_getTimerTicks();

        outputFExp = fExp((float)semitoneStep / 10.0);

        totalTimeFExp += timer_getTimerTicks() - startTime;


        if(outputExp2f > outputFExp){
            deviationTotal += (outputExp2f - outputFExp);
        } else {
            deviationTotal += (outputFExp - outputExp2f);
        }
        //printf(":%d,%d,%d;",semitoneStep,(int)(outputExp2f * 1000),(int)(outputFExp * 1000));

        numberOfTests++;
    }

    printf("\n");
    printf("exp2f: %d, %d\n",totalTimeExp2f, totalTimeExp2f / numberOfTests);
    printf("fExp: %d, %d\n", totalTimeFExp, totalTimeFExp / numberOfTests);
    printf("deviation: %d, %d\n", (uint32_t)(deviationTotal * 1000), (uint32_t)((deviationTotal / numberOfTests) * 1000000));
}

__IO uint8_t int_channel = 0;
__IO int16_t int_sample = 0;
void SPI3_IRQHandler(void){
    if(SPI_I2S_GetFlagStatus(CS43L22_I2S_PORT, SPI_I2S_FLAG_TXE)) {
        cs43l22_outputSample(int_sample);
        if(int_channel == 0) {
            int_channel++;
        } else {
            int_channel = 0;
            int_sample = ply_getNextSample();
        }
    }
}

void testInterruptClock(Btn_struct * button){
    ply_init();
    midi_init();
    cs43l22_initInterrupt();
    cs43l22_enableInterrupt();

    float newValue = 0;
    uint8_t selectedOscillator = 0;
    Midi_basicMsg midiMsg;

    while(1){
        if(pots_readIfActive(PITCH_POT,&newValue)) {
            ply_setPitchOffset(selectedOscillator, newValue * 3 + 0.5);
        }
        if(pots_readIfActive(WAVEFORM_POT,&newValue)) {
            ply_setWaveMix(selectedOscillator, newValue * FOSC_WAVE_MIX_RESOLUTION - 1);
        }
        if(pots_readIfActive(DUTY_POT,&newValue)) {
            ply_setDuty(selectedOscillator, newValue * FOSC_DUTY_MAX);
        }
        if(pots_readIfActive(PHASE_POT,&newValue)) {
            ply_setPhase(selectedOscillator, newValue * WT_EFFECTIVE_SIZE - 1);
        }
        if(pots_readIfActive(AMPLITUDE_POT,&newValue)) {
            ply_setAmplitude(selectedOscillator, newValue * FOSC_AMPLITUDE_MAX);
        }
        if(btn_readOneShot(button)) {
            selectedOscillator++;
            if(selectedOscillator >= VOC_OSCILLATORS_PER_VOICE) selectedOscillator = 0;
            pots_switchFunction();
            if(selectedOscillator == 0) led_setAll(1,0,0,0);
            else if(selectedOscillator == 1) led_setAll(1,1,0,0);
            else if(selectedOscillator == 2) led_setAll(1,1,1,0);
        }

        ply_updateAll();

        if(midi_getMsgIfAble(&midiMsg)) {
            if(midiMsg.msgType == MIDI_MSG_TYPE_NOTE_ON) {
                if(midiMsg.dataBytes[1] == 0) {
                    ply_noteOff(midiMsg.dataBytes[0]);
                } else {
                    ply_noteOn(midiMsg.dataBytes[0]);
                }
            } else if(midiMsg.msgType == MIDI_MSG_TYPE_NOTE_OFF) {
                ply_noteOff(midiMsg.dataBytes[0]);
            }
        }
    }
}

void testLCD(){
    timer_init();
    HD44780_init();

    char c = 'A';
    while(1){
        HD44780_fillScreen(c++);
        if(c > 'z') c = 'A';
        timer_delay(0.5);
    }
}

void testPotsAndLCD(){
    timer_init();
    HD44780_init();
    pots_initAndStart();

    uint8_t pot;
    float value = 0.0;
    HD44780_writeCommand(0x01);
    while(1){
        for(pot = 0; pot < POTS_NUMBER; pot++){
            if(pot == 0){
                HD44780_switchToLine(1);
            } else if(pot == 4){
                HD44780_switchToLine(2);
            }

            if(pots_readIfActive(pot,&value)){
                HD44780_writeData('0' + value * 9.9 );
                HD44780_writeData('0' + (int)(value * 99.9) % 10);
                HD44780_writeData('0' + (int)(value * 999.9) % 10);
                HD44780_writeData(' ');
            }
        }
        //timer_delay(0.05);
    }
}

void testPotsAndButtonsAndLCD(){
    timer_init();
    HD44780_init();
    pots_initAndStart();

    Btn_struct btn0;
    btn0.debounceTime = TIMER_CLOCK_SPEED / 100;
    btn0.portLetter = 'E';
    btn0.pin = 7;
    btn_init(&btn0);

    Btn_struct btn1;
    btn1.debounceTime = btn0.debounceTime;
    btn1.portLetter = 'E';
    btn1.pin = 9;
    btn_init(&btn1);

    Btn_struct btn2;
    btn2.debounceTime = btn0.debounceTime;
    btn2.portLetter = 'E';
    btn2.pin = 11;
    btn_init(&btn2);

    Btn_struct btn3;
    btn3.debounceTime = btn0.debounceTime;
    btn3.portLetter = 'E';
    btn3.pin = 13;
    btn_init(&btn3);

    Btn_struct btn4;
    btn4.debounceTime = btn0.debounceTime;
    btn4.portLetter = 'E';
    btn4.pin = 15;
    btn_init(&btn4);

    Btn_struct btn5;
    btn5.debounceTime = btn0.debounceTime;
    btn5.portLetter = 'B';
    btn5.pin = 11;
    btn_init(&btn5);

    Btn_struct btn6;
    btn6.debounceTime = btn0.debounceTime;
    btn6.portLetter = 'B';
    btn6.pin = 13;
    btn_init(&btn6);

    Btn_struct btn7;
    btn7.debounceTime = btn0.debounceTime;
    btn7.portLetter = 'B';
    btn7.pin = 15;
    btn_init(&btn7);


    uint8_t btnPushed[8];
    int i = 0;
    for(; i < 8; i++){
        btnPushed[i] = 0;
    }

    uint8_t pot;
    float value = 0.0;
    HD44780_writeCommand(0x01);
    while(1){
        if(btn_readOneShot(&btn0)) btnPushed[0] = 1;
        if(btn_readOneShot(&btn1)) btnPushed[1] = 1;
        if(btn_readOneShot(&btn2)) btnPushed[2] = 1;
        if(btn_readOneShot(&btn3)) btnPushed[3] = 1;
        if(btn_readOneShot(&btn4)) btnPushed[4] = 1;
        if(btn_readOneShot(&btn5)) btnPushed[5] = 1;
        if(btn_readOneShot(&btn6)) btnPushed[6] = 1;
        if(btn_readOneShot(&btn7)) btnPushed[7] = 1;

        for(pot = 0; pot < POTS_NUMBER; pot++){
            if(pot == 0){
                HD44780_switchToLine(1);
            } else if(pot == 4){
                HD44780_switchToLine(2);
            }

            if(pots_readIfActive(pot,&value)){
                HD44780_writeData('0' + (int)(value * 9.9) % 10);
                HD44780_writeData('0' + (int)(value * 99.9) % 10);
                HD44780_writeData('0' + (int)(value * 999.9) % 10);

                if(btnPushed[pot]){
                    HD44780_writeData('X');
                } else {
                    HD44780_writeData(' ');
                }
            }
        }

        for(i = 0; i < 8; i++){
            btnPushed[i] = 0;
        }

        timer_delay(0.05);
    }
}

int main(void) {
    #ifdef DEBUG
        printf("\fStarting Debug Mode\n");
    #endif

    testPotsAndButtonsAndLCD();

    /*pots_initAndStart();
    timer_init();
    led_init();
    fExp_init();
    cs43l22_init();
    wt_init();


    Btn_struct button1;
    button1.debounceTime = TIMER_CLOCK_SPEED / 100;
    button1.portLetter = 'E';
    button1.pin = 7;
    btn_init(&button1);

    //testButtonsAndLeds(&button1);

    //testPot(PITCH_POT);

    //testFOscOneShot(25000,1);

    //testFOscContinuous(2,1,1,&button1);
    //testFOscContinuousWithMidi(1,1,1,&button1);

    //testExponent();

    //testFOscContinuousPolyphonicMidi(&button1);

    testInterruptClock(&button1);
    */

    while(1);
}
