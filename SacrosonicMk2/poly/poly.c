#include "poly.h"

void ply_init() {
    int i = 0;
    for(; i < PLY_NUMBER_OF_VOICES; i++) {
        voc_init(&ply_voices[i]);
    }

    ply_voiceIndex = 0;
    ply_voiceUpdateIndex = 0;
}

void ply_noteOn(uint8_t midiNote) {
    voc_voiceStruct * voice = 0;

    int i = 0;
    for(; i < PLY_NUMBER_OF_VOICES; i++) {
        if(!voc_isActive(&ply_voices[i])) {
            voice = &ply_voices[i];
            ply_voiceIndex = i + 1;
            break;
        }
    }

    if(voice == 0) {
        voice = &ply_voices[ply_voiceIndex++];
    }

    if(ply_voiceIndex >= PLY_NUMBER_OF_VOICES) ply_voiceIndex = 0;

    voc_trigger(voice,midiNote);
}

void ply_noteOff(uint8_t midiNote) {
    int i = 0;
    for(; i < PLY_NUMBER_OF_VOICES; i++) {
        if(ply_voices[i].currentNote == midiNote) voc_release(&ply_voices[i]);
    }
}

void ply_releaseAll() {
    int i = 0;
    for(; i < PLY_NUMBER_OF_VOICES; i++) {
        voc_release(&ply_voices[i]);
    }
}

void ply_makeUpdateStep() {
    voc_makeUpdateStep(&ply_voices[ply_voiceUpdateIndex++]);
    if(ply_voiceUpdateIndex >= PLY_NUMBER_OF_VOICES) ply_voiceUpdateIndex = 0;
}

int16_t ply_getNextSample() {
    int32_t total = 0;
    int i = 0;
    for(; i < PLY_NUMBER_OF_VOICES; i++){
        total += voc_getNextSample(&ply_voices[i]);
    }
    total /= PLY_NUMBER_OF_VOICES;
    return (int16_t)total;
}

