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

#define PITCH_POT 0
#define WAVEFORM_POT 1
#define DUTY_POT 2
#define PHASE_POT 3
#define AMPLITUDE_POT 4

#define PITCH_BOTTOM 40
#define PITCH_RANGE 4000

int main(void) {
    osc_init();
    pots_initAndStart();

    env_Envelope * osc1Envelope;
    env_Envelope * osc2Envelope;
    osc1Envelope = malloc(sizeof(env_Envelope));
    osc2Envelope = malloc(sizeof(env_Envelope));
    env_init(osc1Envelope);
    env_init(osc2Envelope);

    osc2Envelope->hold = 0.5;

    Osc_struct * osc = &osc_oscillator1;
    uint8_t updateStep = 0;
    uint32_t sampleCounter = 0; // note since we output one sample for each channel this actually counts at twice the output sample rate

    while(1) {
        while(!osc_attemptOutput());

        if(sampleCounter == 15){
            if(osc1Envelope->state == ENV_STATE_DEAD) env_trigger(osc1Envelope);
            env_getNextOutput(osc1Envelope);
        } else if(sampleCounter == 31){
            sampleCounter = 0;
            if(osc2Envelope->state == ENV_STATE_DEAD) env_trigger(osc2Envelope);
            env_getNextOutput(osc2Envelope);
        } else {
            switch(updateStep){
            case 0:
                if(osc == &osc_oscillator1){
                    osc->pitch = pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM;
                } else {
                    osc->pitch = (pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM) * 2;
                }
                updateStep++;
                break;
            case 1:
                osc_updateStepSizeBase(osc);
                updateStep++;
                break;
            case 2:
                osc_updateStepSizeHigh(osc);
                updateStep++;
                break;
            case 3:
                osc_updateStepSizeLow(osc);
                updateStep++;
                break;
            case 4:
                osc->waveform = pots_getMappedAverage(WAVEFORM_POT);
                updateStep++;
                break;
            case 5:
                osc->duty = pots_getMappedAverage(DUTY_POT);
                updateStep++;
                break;
            case 6:
                osc->phase = pots_getMappedAverage(PHASE_POT);
                updateStep++;
                break;
            case 7:
                osc->amplitude = pots_getMappedAverage(AMPLITUDE_POT);
                if(osc == &osc_oscillator1){
                    osc->amplitude *= osc1Envelope->output;
                } else {
                    osc->amplitude *= osc2Envelope->output;
                }
                updateStep++;
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

        sampleCounter++;
    }
}
