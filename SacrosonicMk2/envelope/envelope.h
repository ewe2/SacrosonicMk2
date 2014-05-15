#ifndef ENVELOPE_H_INCLUDED
#define ENVELOPE_H_INCLUDED

#include "../timer/timer.h"

#define ENV_ATTACK_DEFAULT (0.2 * TIMER_CLOCK_SPEED)
#define ENV_DECAY_DEFAULT (0.1 * TIMER_CLOCK_SPEED)
#define ENV_SUSTAIN_DEFAULT 0.5
#define ENV_RELEASE_DEFAULT (0.3 * TIMER_CLOCK_SPEED)
#define ENV_PEAK_DEFAULT 1.0

typedef enum {
    ENV_STATE_ATTACK,
    ENV_STATE_DECAY,
    ENV_STATE_SUSTAIN,
    ENV_STATE_RELEASE,
    ENV_STATE_DEAD
} Env_state;

typedef struct {
    Env_state state;
    uint8_t isHeld;
    float output;
    float lastSampleTime;
    float attack;
    float decay;
    float sustain;
    float release;
    float peak;
} Env_envelope;

void env_init(Env_envelope * env);
float env_getNextSample(Env_envelope * env);
void env_trigger(Env_envelope * env);
void env_release(Env_envelope * env);

#endif /* ENVELOPE_H_INCLUDED */
