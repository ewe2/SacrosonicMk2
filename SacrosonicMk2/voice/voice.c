#include "voice.h"

void voc_setPitch(voc_voiceStruct * voice, uint8_t midiNote) {
    voice->currentNote = midiNote;
    voice->pitch.c = midi_notes[midiNote].c;

    int i = 0;
    for(; i < VOC_OSCILLATORS_PER_VOICE; i++) {
        voice->oscillators[i].pitch.c = voice->pitch.c;
    }
}

void voc_init(voc_voiceStruct * voice) {
    voc_setPitch(voice, MIDI_NOTE_A4_INDEX);

    int i = 0;
    for(; i < VOC_OSCILLATORS_PER_VOICE; i++) {
        fOsc_init(&voice->oscillators[i],1);
        voice->oscillators[i].dutyEnabled = 1;
    }

    env_init(&voice->envelope);

    voice->updateStep = 0;
    voice->updateOscillator = 0;
}

void voc_trigger(voc_voiceStruct * voice, uint8_t midiNote) {
    voc_setPitch(voice, midiNote);
    env_trigger(&voice->envelope);
}

void voc_release(voc_voiceStruct * voice) {
    env_release(&voice->envelope);
}

int voc_isActive(voc_voiceStruct * voice) {
    return (voice->envelope.state != ENV_STATE_DEAD);
}

int16_t voc_getNextSample(voc_voiceStruct* voice) {
    int32_t total = 0;

    int i = 0;
    for(; i < VOC_OSCILLATORS_PER_VOICE; i++) {
        total += fOsc_getNextSample(&voice->oscillators[i]);
    }

    total /= VOC_OSCILLATORS_PER_VOICE;
    total *= voice->envelope.output;
    return (int16_t)total;
}

void voc_makeUpdateStep(voc_voiceStruct* voice) {
    fOsc_struct * oscillator = &voice->oscillators[voice->updateOscillator];

    switch(voice->updateStep++) {
    case 0:
        fOsc_updateStepSizeBase(oscillator);
        break;
    case 1:
        fOsc_updateStepSizeHigh(oscillator);
        break;
    case 2:
        fOsc_updateStepSizeLow(oscillator);
        break;
    case 3:
        fOsc_updateSwing(oscillator);
        break;
    case 4:
        if(voice->updateOscillator == 0) { // we don't have to do this separately for every oscillator
            env_getNextSample(&voice->envelope);
        }
        break;
    default:
        voice->updateStep = 0;
        voice->updateOscillator++;
        if(voice->updateOscillator >= VOC_OSCILLATORS_PER_VOICE) voice->updateOscillator = 0;
    }
}

