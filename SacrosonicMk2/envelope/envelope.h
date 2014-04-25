#ifndef ENVELOPE_H_INCLUDED
#define ENVELOPE_H_INCLUDED

#define ENV_SAMPLE_RATE (96000 / 32)
#define ENV_SAMPLE_PERIOD (1.0 / ENV_SAMPLE_RATE)

#define ENV_HOLD_DEFAULT 1.0
#define ENV_ATTACK_DEFAULT 0.01
#define ENV_DECAY_DEFAULT 0.1
#define ENV_SUSTAIN_DEFAULT 0.5
#define ENV_RELEASE_DEFAULT 0.5

typedef enum {
    ENV_STATE_ATTACK,
    ENV_STATE_DECAY,
    ENV_STATE_SUSTAIN,
    ENV_STATE_RELEASE,
    ENV_STATE_DEAD
} env_State;

typedef struct {
    env_State state;
    float output;
    float runTime;
    float hold;
    float attack;
    float decay;
    float sustain;
    float release;
} env_Envelope;

void env_init(env_Envelope * envelope);
float env_getNextOutput(env_Envelope * envelope);
void env_trigger(env_Envelope * envelope);

#endif /* ENVELOPE_H_INCLUDED */
