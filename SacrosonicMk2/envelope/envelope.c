#include "envelope.h"

void env_init(Env_envelope * env){
    env->state = ENV_STATE_DEAD;
    env->isHeld = 0;
    env->output = 0.0;
    env->attack = ENV_ATTACK_DEFAULT;
    env->decay = ENV_DECAY_DEFAULT;
    env->sustain = ENV_SUSTAIN_DEFAULT;
    env->release = ENV_RELEASE_DEFAULT;
    env->peak = ENV_PEAK_DEFAULT;
}

float env_getNextSample(Env_envelope * env){
    uint32_t timePassed = timer_getTimerTicks() - env->lastSampleTime;

    if(!env->isHeld && env->state != ENV_STATE_DEAD) env->state = ENV_STATE_RELEASE;

    switch(env->state){
    case ENV_STATE_DEAD:
        return 0.0; // return from function here if DEAD, nothing needs doing if DEAD.
    case ENV_STATE_ATTACK:
        env->output += timePassed / env->attack;
        if(env->output >= env->peak) {
            env->output = env->peak;
            env->state = ENV_STATE_DECAY;
        }
        break;
    case ENV_STATE_DECAY:
        env->output -= timePassed / env->decay;
        if(env->output <= env->sustain) {
            env->output = env->sustain;
            env->state = ENV_STATE_SUSTAIN;
        }
        break;
    case ENV_STATE_SUSTAIN:
        env->output = env->sustain;
        break;
    case ENV_STATE_RELEASE:
        env->output -= timePassed / env->release;
        if(env->output <= 0.0){
            env->output = 0.0;
            env->state = ENV_STATE_DEAD;
        }
        break;
    }
    env->lastSampleTime = timer_getTimerTicks();
    return env->output;
}

void env_trigger(Env_envelope * env){
    env->state = ENV_STATE_ATTACK;
    env->lastSampleTime = timer_getTimerTicks();
    env->isHeld = 1;
}

void env_release(Env_envelope * env){
    env->isHeld = 0;
}
