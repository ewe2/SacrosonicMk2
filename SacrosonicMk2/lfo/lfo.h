#ifndef LFO_H_INCLUDED
#define LFO_H_INCLUDED

#define LFO_SAMPLE_RATE (96000 / 32);

#define LFO_PITCH_DEFAULT 1.0
#define LFO_AMOUNT_DEFAULT 1.0

typedef struct{
    float pitch;
    float amount;
    float output;
    float index;
} lfo_Lfo;

void lfo_init(lfo_Lfo * lfo);
float lfo_getNextSample(lfo_Lfo * lfo);



#endif /* LFO_H_INCLUDED */
