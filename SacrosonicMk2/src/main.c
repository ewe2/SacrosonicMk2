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

#include "../osc/osc.h"
#include "../pots/pots.h"

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


    uint8_t updateStep = 0;

    while(1) {
        while(!osc_attemptOutput());

        switch(updateStep){
        case 0:
            osc_pitch = pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM;
            updateStep++;
            break;
        case 1:
            osc_updateStepSizeBase();
            updateStep++;
            break;
        case 2:
            osc_updateStepSizeHigh();
            updateStep++;
            break;
        case 3:
            osc_updateStepSizeLow();
            updateStep++;
            break;
        case 4:
            osc_waveform = pots_getMappedAverage(WAVEFORM_POT);
            updateStep++;
            break;
        case 5:
            osc_duty = pots_getMappedAverage(DUTY_POT);
            updateStep++;
            break;
        case 6:
            osc_phase = pots_getMappedAverage(PHASE_POT);
            updateStep++;
            break;
        case 7:
            osc_amplitude = pots_getMappedAverage(AMPLITUDE_POT);
            updateStep++;
            break;
        case 8:
            osc_updateSwing();
            updateStep++;
            break;
        default:
            updateStep = 0;
            break;
        }
    }
}
