#include "wavetable.h"


#include <stdint.h>

#include "math.h"

void wt_init() {
    wt_initSine();
    wt_initSquare();
    wt_initTri();
}

void wt_initSine() {
    uint32_t i = 0;
    for(i = 0; i < WT_SIZE; i++) {
        float sample = sinf((2.0 * PI / WT_SIZE) * i * WT_PHASE_RANGE);
        wt_sine[i] = sample;
    }
}

void wt_initTri() {
    uint32_t i = 0;
    for(i = 0; i < WT_SIZE; i++) {
        wt_tri[i] = (1.0 / WT_SIZE) * i;
    }
}

void wt_initSquare() {
    uint32_t i = 0;
    for(i = 0; i < WT_SIZE; i++) {
        wt_square[i] = 1.0;
    }
}

float wt_getSampleFromTable(float * table, float index, float offset) {
    index += offset;
    while(index >= 1.0) index -= 1.0;

    if(index < 0.25) {
        index /= WT_PHASE_RANGE;
        int intIndex = (int)(index * WT_SIZE);
        return table[intIndex];

    } else if(index == 0.25){
        return table[WT_SIZE - 1]; // there is no good way of handling this borderline case, this is the best thing I can think of.

    } else if(index < 0.5) {
        index = WT_PHASE_RANGE * 2 - index;
        return wt_getSampleFromTable(table, index, 0.0);

    } else if(index < 1.0){
        index -= WT_PHASE_RANGE * 2;
        return -1.0 * wt_getSampleFromTable(table, index, 0.0);

    } else {
        return 0.0;
    }
}

int16_t wt_int_getTableIndex(uint16_t index){ // NOTE: RETURNS NEGATIVE INDEX IF IN NEGATIVE CYCLE OF WAVE
    index &= ~(3 << 14); // mask off last two bits, the maximum index is 16363 (14 full bits)

    int negative = 0;

    if(index >= WT_EFFECTIVE_SIZE / 2){ // between 0.5 and 1.0
        index -= WT_EFFECTIVE_SIZE / 2;
        negative = 1;
    }
    if(index >= WT_EFFECTIVE_SIZE / 4){ // between 0.25 and 0.5
        index = WT_EFFECTIVE_SIZE / 2 - index - 1;
    }
    if(negative) index *= -1;
    return index;
}

float wt_mixSamples(float sample1, float sample2, float mix){
    if(mix < 0.0 || mix > 1.0) return 0;
    return sample1 * (1.0 - mix) + sample2 * mix;
}

int16_t wt_int_mixSamples(int16_t sample1, int16_t sample2, uint16_t mix, uint16_t resolution){
    return (sample1 * mix + sample2 * (resolution - mix)) / resolution;
}
