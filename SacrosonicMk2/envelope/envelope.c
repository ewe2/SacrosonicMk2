#include "envelope.h"

void env_init(env_Envelope * envelope){
    envelope->state = ENV_STATE_DEAD;
    envelope->output = 0.0;
    envelope->runTime = 0.0;

    envelope->hold = ENV_HOLD_DEFAULT;
    envelope->attack = ENV_ATTACK_DEFAULT;
    envelope->decay = ENV_DECAY_DEFAULT;
    envelope->sustain = ENV_SUSTAIN_DEFAULT;
    envelope->release = ENV_RELEASE_DEFAULT;
}

float env_getNextOutput(env_Envelope * envelope){
    switch(envelope->state){
    case ENV_STATE_DEAD:
        return; // return from function here if DEAD, nothing needs doing if DEAD.
    case ENV_STATE_ATTACK:
        envelope->output += ENV_SAMPLE_PERIOD / envelope->attack;
        if(envelope->output >= 1.0) {
            envelope->output = 1.0;
            envelope->state = ENV_STATE_DECAY;
        }
        break;
    case ENV_STATE_DECAY:
        envelope->output -= ENV_SAMPLE_PERIOD / envelope->decay;
        if(envelope->output <= envelope->sustain) {
            envelope->output = envelope->sustain;
            envelope->state = ENV_STATE_SUSTAIN;
        }
        break;
    case ENV_STATE_SUSTAIN:
        if(envelope->runTime >= envelope->hold){
            envelope->state = ENV_STATE_RELEASE;
        }
        break;
    case ENV_STATE_RELEASE:
        envelope->output -= ENV_SAMPLE_PERIOD / envelope->release;
        if(envelope->output <= 0.0){
            envelope->output = 0.0;
            envelope->state = ENV_STATE_DEAD;
            envelope->runTime = 0.0;
        }
        break;
    }
    envelope->runTime += ENV_SAMPLE_PERIOD;
}

void env_trigger(env_Envelope * envelope){
    envelope->state = ENV_STATE_ATTACK;
    envelope->runTime = 0.0;
}
