#ifndef LFO_H_INCLUDED
#define LFO_H_INCLUDED

#define LFO_PITCH_DEFAULT 1.0

#include "../wavetable/wavetable.h"
#include "../timer/timer.h"
#include "fixedPoint.h"

typedef struct{
    float pitch;
    float index;
    float * wavetable;
    uint32_t lastSampleTime;
    float output;
} lfo_Lfo;

void lfo_init(lfo_Lfo * lfo);
float lfo_getNextSample(lfo_Lfo * lfo);




#endif /* LFO_H_INCLUDED */
