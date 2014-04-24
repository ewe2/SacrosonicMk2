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

    Osc_struct * osc = &osc_oscillator1;
    uint8_t updateStep = 0;

    while(1) {
        while(!osc_attemptOutput());

        switch(updateStep){
        case 0:
            osc->pitch = pots_getMappedAverage(PITCH_POT) * PITCH_RANGE + PITCH_BOTTOM;
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
}
