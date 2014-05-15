#ifndef POLY_H_INCLUDED
#define POLY_H_INCLUDED

#include "fixedPoint.h"
#include <stdint.h>
#include "../midi/midi.h"

#include "../voice/voice.h"

#define PLY_NUMBER_OF_VOICES 4

voc_voiceStruct ply_voices[PLY_NUMBER_OF_VOICES];

uint8_t ply_voiceIndex;
uint8_t ply_voiceUpdateIndex;

void ply_init();

void ply_noteOn(uint8_t midiNote);

void ply_noteOff(uint8_t midiNote);

void ply_releaseAll();

void ply_makeUpdateStep();

int16_t ply_getNextSample();

#endif /* POLY_H_INCLUDED */
