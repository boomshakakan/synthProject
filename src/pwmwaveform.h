#ifndef PWMWAVEFORM_H_
#define PWMWAVEFORM_H_

#include <stdint.h>

typedef struct
{
    enum
    {
        Sine, Square, Triangle
    } type;
    uint16_t pulseWidth;
    uint16_t period;
    uint16_t volumeWeight;
    uint16_t periodWeight;
} pwm_Waveform;

void waveformInit();

void waveformOn();

void waveformOff();

void waveformPwmSet(pwm_Waveform waveform);

#endif
