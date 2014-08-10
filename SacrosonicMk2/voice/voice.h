#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED

#include "fixedPoint.h"
#include <stdint.h>

#include "../fastOsc/fastOsc.h"
#include "../envelope/envelope.h"
#include "../lfo/lfo.h"
#include "../midi/midi.h"

#define VOC_OSCILLATORS_PER_VOICE 3
#define VOC_ENVELOPES_PER_VOICE 3
#define VOC_GLOBAL_AMPLITUDE_ENVELOPE 0
#define VOC_LFOS_PER_VOICE 1

typedef struct {
    uint8_t currentNote;
    FixedPoint pitch;
    fOsc_struct oscillators[VOC_OSCILLATORS_PER_VOICE];
    Env_envelope envelopes[VOC_ENVELOPES_PER_VOICE];
    lfo_Lfo lfos[VOC_LFOS_PER_VOICE];

    uint8_t oscillatorUpdateIndex;
    uint8_t envelopeUpdateIndex;
    uint8_t lfoUpdateIndex;
    uint8_t updateStep;
} voc_voiceStruct;

void voc_setPitch(voc_voiceStruct * voice, uint8_t midiNote);
void voc_init(voc_voiceStruct * voice);

void voc_trigger(voc_voiceStruct * voice, uint8_t midiNote);
void voc_release(voc_voiceStruct * voice);
int voc_isActive(voc_voiceStruct * voice);

int16_t voc_getNextSample(voc_voiceStruct * voice);
void voc_makeUpdateStep(voc_voiceStruct * voice);
void voc_updateAll(voc_voiceStruct * voice);

#endif /* VOICE_H_INCLUDED */
