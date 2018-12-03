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
#define BUZZER_OFF_TIME         0
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

typedef struct
{
    enum
    {
        Inactive, redMode, greenMode, blueMode
    } state;
} modeIndicator;

typedef struct
{
    enum
    {
        Sine, Square, Triangle
    } state;
} waveForm;

static volatile buzzer_t buzzer = {SwitchOn, false, 0, BUZZER_MAX_PERIOD, BUZZER_MIN_PERIOD, BUZZER_MAX_PULSE_WIDTH};
static volatile modeIndicator led = {Inactive};
static volatile waveForm wave = {Sine};

void turnLEDOn(uint8_t pin) {
    GPIOPinWrite(GPIO_PORTF_BASE, pin, pin);
}

void turnLEDOff(uint8_t pin) {
    GPIOPinWrite(GPIO_PORTF_BASE, pin, 0);
}

// a sine function that uses degree as input
static inline double sine(unsigned int degree) {
    double radian = 2 * M_PI * ((double) (degree % 360) / 360);
    return sin(radian);
}

//static inline double triangle(unsigned int degree) {

//}

//static inline int square(unsigned int degree) {

//}

// make a function for the triangle waveform

// make a function for the square waveform

uint32_t checkRotary(int whichReading) {
    uint32_t val[2];

    rotaryRead(val);

    // return the first index of array, O.W return whole array
    if (whichReading == 0) return val[0];
    else return val[1];
}

void readPushbutton(uint32_t time) {

    uint32_t delay = 20;

    int button = pbRead();

    switch (button) {

    case 1:
        // this button is for switching the system on and off
        uprintf("Button 1 pressed\n");

        if (led.state == redMode) {
            turnLEDOff(RED_LED);
            led.state = Inactive;
            buzzer.state = SwitchOff;
        }
        else if (led.state == greenMode) {
            turnLEDOff(GREEN_LED);
            led.state = Inactive;
            buzzer.state = SwitchOff;
        }
        else if (led.state == blueMode) {
            turnLEDOff(BLUE_LED);
            led.state = Inactive;
            buzzer.state = SwitchOff;
        }
        else {
            turnLEDOn(RED_LED);
            led.state = redMode;
            buzzer.state = SwitchOn;
        }
        break;

    case 2:
        // switches the modes of operation of the system
        uprintf("Button 2 pressed\n");

        if (led.state == redMode) {
            turnLEDOff(RED_LED);
            turnLEDOn(GREEN_LED);
            led.state = greenMode;
        }
        else if (led.state == greenMode) {
            turnLEDOff(GREEN_LED);
            turnLEDOn(BLUE_LED);
            led.state = blueMode;
        }
        else if (led.state == blueMode) {
            turnLEDOff(BLUE_LED);
            turnLEDOn(RED_LED);
            led.state = redMode;
        }
        else {
            uprintf("Cannot change modes when system is inactive\n");
        }
        break;
    }

    schdCallback(readPushbutton, time + delay);
}

// The buzzer play callback function
void buzzerPlay(uint32_t time) {
    static unsigned int angle = 0;
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
    switch (wave.state) {
    case Sine:
        break;
    case Square:
        break;
    case Triangle:
        break;
    }

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

    angle++;
    // schedule the next callback
    schdCallback(buzzerPlay, time + delay);
}

void main(void)
{
    lpInit();
    initRotary();
    buzzerInit();

    schdCallback(buzzerPlay, 1001);
    schdCallback(readPushbutton, 1002);

    // Loop forever
    while (true)
    {
        schdExecute();
    }
}
