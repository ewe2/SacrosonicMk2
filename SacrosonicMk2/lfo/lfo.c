#include "lfo.h"

void lfo_init(lfo_Lfo * lfo){
    lfo->pitch = LFO_PITCH_DEFAULT;
    lfo->index = 0.0;
    lfo->lastSampleTime = timer_getTimerTicks();
    lfo->output = 0.0;
    lfo->wavetable = &wt_tri;
}

// output ranging from -1.0 to 1.0
float lfo_getNextSample(lfo_Lfo * lfo){
    float timePassed = (float)(timer_getTimerTicks() - lfo->lastSampleTime) / TIMER_CLOCK_SPEED;
    lfo->lastSampleTime = timer_getTimerTicks();
    // TODO: fix overflow

    lfo->index += timePassed * lfo->pitch;
    lfo->index -= (int)lfo->index;

    int16_t adjustedIndex = wt_int_getTableIndex(lfo->index * WT_EFFECTIVE_SIZE);
    if(adjustedIndex < 0){
        lfo->output = -1 * lfo->wavetable[adjustedIndex * -1];
    } else {
        lfo->output = lfo->wavetable[adjustedIndex];
    }

    return lfo->output;
}
