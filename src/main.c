/*
 * mail.c: Starter code for Lab 7, Fall 2018
 *
 * Created by Zhao Zhang
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "launchpad.h"
#include "rotary.h"
#include "pwmbuzzer.h"

#define RED_LED                 GPIO_PIN_1
#define BLUE_LED                GPIO_PIN_2
#define GREEN_LED               GPIO_PIN_3
#define BUZZER_CHECK_INTERVAL   30
#define BUZZER_ON_TIME          200
#define BUZZER_OFF_TIME         (2000 - BUZZER_ON_TIME)
#define BUZZER_MIN_PERIOD       95557 // (highest f, C5)
#define BUZZER_MAX_PERIOD       191110 // 50MHz / 261.63 (lowest f, C4)
#define BUZZER_MAX_PULSE_WIDTH  100 // used same max pulse width as in LED example

typedef struct
{
    enum
    {
        Off, On, SwitchOff, SwitchOn
    } state;            // the running state of the buzzer system
    bool buzzing;       // if the buzzer is buzzing
    int32_t timeLeft;   // the time left for the buzzer to buzz or not buzz
    int maxPeriod;      // maximum possible period
    int minPeriod;      // minimum possible period
    int maxPulseWidth;  // used same value as for LED example
} buzzer_t;

static volatile buzzer_t buzzer = {SwitchOn, false, 0, BUZZER_MAX_PERIOD, BUZZER_MIN_PERIOD, BUZZER_MAX_PULSE_WIDTH};

// a sine function that uses degree as input
static inline double sine(unsigned int degree)
{
    double radian = 2 * M_PI * ((double) (degree % 360) / 360);
    return sin(radian);
}

uint32_t checkRotary(int whichReading) {
    uint32_t val[2];

    rotaryRead(val);

    // return the first index of array, O.W return whole array
    if (whichReading == 0) return val[0];
    else return val[1];
}

// The buzzer play callback function
void buzzerPlay(uint32_t time)
{
    uint32_t delay = BUZZER_CHECK_INTERVAL; // the delay for next callback
    uint32_t arr[2];

    arr[0] = checkRotary(0);
    arr[1] = checkRotary(1);

    // checking that our values are read correctly
    //uprintf("val[0]: %d\nval[1]: %d", arr[0], arr[1]);

    // check what values these return
    uint32_t volumeWeight = 99 - arr[0] * 100 / 4096;
    uint32_t periodWeight = 99 - arr[1] * 100 / 4096;

    buzzerpwm_t currentBuzzer;

    uprintf("volume Weight: %d\nperiod Weight: %d", volumeWeight, periodWeight);

    currentBuzzer.period = periodWeight * ((buzzer.maxPeriod - buzzer.minPeriod) / 100) + buzzer.minPeriod;
    currentBuzzer.pulseWidth = volumeWeight * buzzer.maxPulseWidth;
    // VOLUME = DUTY CYCLE = pulseWidth / period

    switch (buzzer.state) {
    case Off:           // the buzzer system is turned off
        break;

    case On:            // the buzzer system is active, turn buzzer on and off
        if (buzzer.buzzing)
        {
            // If the buzzer has been buzzing for enough time, turn it off
            if ((buzzer.timeLeft -= BUZZER_CHECK_INTERVAL) <= 0) {
                buzzerOff();
                buzzer.buzzing = false;
                buzzer.timeLeft = BUZZER_OFF_TIME;
            }
        }
        else
        {
            // If the buzzer has been silent for enough time, turn it on
            if ((buzzer.timeLeft -= BUZZER_CHECK_INTERVAL) <= 0) {
                buzzerPwmSet(currentBuzzer);
                buzzer.buzzing = true;
                buzzer.timeLeft = BUZZER_ON_TIME;
            }
        }
        break;

    case SwitchOff:             // De-activate the buzzer system
        if (buzzer.buzzing)
            buzzerOff();
        buzzer.state = Off;
        buzzer.buzzing = Off;
        break;

    case SwitchOn:              // Activate the buzzer system
        buzzerPwmSet(currentBuzzer);
        buzzer.state = On;
        buzzer.buzzing = true;
        buzzer.timeLeft = BUZZER_ON_TIME;
        break;
    }
    // schedule the next callback
    schdCallback(buzzerPlay, time + delay);
}

void main(void)
{
    lpInit();
    initRotary();
    buzzerInit();

    schdCallback(buzzerPlay, 1001);

    // Loop forever
    while (true)
    {
        schdExecute();
    }
}
