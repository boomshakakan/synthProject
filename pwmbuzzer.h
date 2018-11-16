#ifndef PWMBUZZER_H_
#define PWMBUZZER_H_

#include <stdint.h>

typedef struct {
    uint16_t pulseWidth;
    uint16_t period;
} buzzerpwm_t;

void buzzerInit();

void buzzerOn();

void buzzerOff();

void buzzerPwmSet(buzzerpwm_t buzzer);

#endif
