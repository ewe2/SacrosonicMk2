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
void ply_updateAll();
int16_t ply_getNextSample();

void ply_setPitchOffset(uint8_t oscillator, float offset);
void ply_setWaveMix(uint8_t oscillator, uint8_t waveMix);
void ply_setDuty(uint8_t oscillator, uint8_t duty);
void ply_setPhase(uint8_t oscillator, uint16_t phase);
void ply_setAmplitude(uint8_t oscillator, uint32_t amplitude);

#endif /* POLY_H_INCLUDED */
