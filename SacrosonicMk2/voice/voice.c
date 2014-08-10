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
        voice->oscillators[i].amplitude = 0;
        voice->oscillators[i].amplitudeMod.modSource = &voice->envelopes[VOC_GLOBAL_AMPLITUDE_ENVELOPE].output;
        voice->oscillators[i].amplitudeMod.modAmount = 1.0;
        voice->oscillators[i].dutyMod.modSource = &voice->lfos[0].output;
        voice->oscillators[i].dutyMod.modAmount = 0.5;
    }

    for(i = 0; i < VOC_ENVELOPES_PER_VOICE; i++){
        env_init(&voice->envelopes[i]);
    }

    for(i = 0; i < VOC_LFOS_PER_VOICE; i++){
        lfo_init(&voice->lfos[i]);
    }

    voice->lfos[0].pitch = 4;

    voice->envelopes[1].attack = 1.0 * TIMER_CLOCK_SPEED;
    voice->envelopes[2].attack = 0.5 * TIMER_CLOCK_SPEED;

    //voice->oscillators[0].pitchMod.modSource = &voice->envelopes[1].output;
    //voice->oscillators[0].pitchMod.modAmount = 12.0;
    //voice->oscillators[0].phaseMod.modSource = &voice->envelopes[2].output;
    //voice->oscillators[0].phaseMod.modAmount = 1.0;

    voice->updateStep = 0;
    voice->oscillatorUpdateIndex = 0;
    voice->envelopeUpdateIndex = 0;
}

void voc_trigger(voc_voiceStruct * voice, uint8_t midiNote) {
    voc_setPitch(voice, midiNote);
    int i = 0;
    for(; i < VOC_ENVELOPES_PER_VOICE; i++){
        env_trigger(&voice->envelopes[i]);
    }
}

void voc_release(voc_voiceStruct * voice) {
    int i = 0;
    for(; i < VOC_ENVELOPES_PER_VOICE; i++){
        env_release(&voice->envelopes[i]);
    }
}

int voc_isActive(voc_voiceStruct * voice) {
    int i = 0;
    for(; i < VOC_ENVELOPES_PER_VOICE; i++){
        if(voice->envelopes[i].state != ENV_STATE_DEAD) return 1;
    }
    return 0;
}

int16_t voc_getNextSample(voc_voiceStruct* voice) {
    int32_t total = 0;

    int i = 0;
    for(; i < VOC_OSCILLATORS_PER_VOICE; i++) {
        total += fOsc_getNextSample(&voice->oscillators[i]);
    }

    total /= VOC_OSCILLATORS_PER_VOICE;
    return (int16_t)total;
}

void voc_makeUpdateStep(voc_voiceStruct* voice) {
    fOsc_struct * oscillator = &voice->oscillators[voice->oscillatorUpdateIndex];

    switch(voice->updateStep++) {
    case 0:
        fOsc_updateModResult(&oscillator->pitchMod);
        break;
    case 1:
        fOsc_updateModResult(&oscillator->waveMixMod);
        break;
    case 2:
        fOsc_updateModResult(&oscillator->dutyMod);
        break;
    case 3:
        fOsc_updateModResult(&oscillator->phaseMod);
        break;
    case 4:
        fOsc_updateModResult(&oscillator->amplitudeMod);
        break;
    case 5:
        fOsc_modulatePitch(oscillator);
        break;
    case 6:
        fOsc_modulateWaveMix(oscillator);
        break;
    case 7:
        fOsc_modulateDuty(oscillator);
        break;
    case 8:
        fOsc_modulatePhase(oscillator);
        break;
    case 9:
        fOsc_modulateAmplitude(oscillator);
        break;
    case 10:
        fOsc_updateStepSizeBase(oscillator);
        break;
    case 11:
        fOsc_updateStepSizeHigh(oscillator);
        break;
    case 12:
        fOsc_updateStepSizeLow(oscillator);
        break;
    case 13:
        fOsc_updateSwing(oscillator);
        break;
    case 14:
        env_getNextSample(&voice->envelopes[voice->envelopeUpdateIndex++]);
        if(voice->envelopeUpdateIndex >= VOC_ENVELOPES_PER_VOICE) voice->envelopeUpdateIndex = 0;
        break;
    case 15:
        lfo_getNextSample(&voice->lfos[voice->lfoUpdateIndex++]);
        if(voice->lfoUpdateIndex >= VOC_LFOS_PER_VOICE) voice->lfoUpdateIndex = 0;
        break;
    default:
        voice->updateStep = 0;
        voice->oscillatorUpdateIndex++;
        if(voice->oscillatorUpdateIndex >= VOC_OSCILLATORS_PER_VOICE) voice->oscillatorUpdateIndex = 0;
    }
}

void voc_updateAll(voc_voiceStruct* voice){
    int i = 0;
    for(i = 0; i < VOC_ENVELOPES_PER_VOICE; i++){
        env_getNextSample(&voice->envelopes[i]);
    }
    for(i = 0; i < VOC_LFOS_PER_VOICE; i++){
        lfo_getNextSample(&voice->lfos[i]);
    }
    for(i = 0; i < VOC_OSCILLATORS_PER_VOICE; i++){
        fOsc_updateDerivatives(&voice->oscillators[i]);
    }
}


