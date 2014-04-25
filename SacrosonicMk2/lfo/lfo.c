#include "lfo.h"

void lfo_init(lfo_Lfo * lfo){
    lfo->pitch = LFO_PITCH_DEFAULT;
    lfo->amount = LFO_AMOUNT_DEFAULT;
    lfo->output = 0.0;
    lfo->index = 0.0;
}

// output ranging from -1.0 to 1.0
float lfo_getNextSample(lfo_Lfo * lfo){
    if(lfo->index <= 0.5){
        lfo->output = (-1 + 4 * lfo->index) * lfo->amount;
    } else {
        lfo->output = (1 - 4 * (lfo->index - 0.5)) * lfo->amount;
    }

    lfo->index += lfo->pitch / LFO_SAMPLE_RATE;
    if(lfo->index > 1.0) lfo->index -= 1.0;

    return lfo->output;
}
