/*
 * mail.c: Starter code for Lab 7, Fall 2018
 *
 * Created by Zhao Zhang
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <driverlib/sysctl.h>
#include <inc/hw_ints.h>
#include <inc/hw_i2c.h>
#include "inc/hw_memmap.h"
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/i2c.h>
#include <pwmwaveform.h>
#include "launchpad.h"
#include "seg7.h"
#include "rotary.h"

#define RED_LED                 GPIO_PIN_1
#define BLUE_LED                GPIO_PIN_2
#define GREEN_LED               GPIO_PIN_3
#define WAVEFORM_CHECK_INTERVAL   10
#define WAVEFORM_ON_TIME          500
#define WAVEFORM_OFF_TIME         0
#define WAVEFORM_MIN_PERIOD       95557  // (highest freq, C5)
#define WAVEFORM_MAX_PERIOD       382225 // 50MHz / 130.813 (lowest freq, C3)
#define WAVEFORM_MAX_PULSE_WIDTH  50    // used same max pulse width as in LED example

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
} SignalLimits;

typedef struct
{
    enum
    {
        Inactive, redMode, greenMode, blueMode
    } state;
} modeIndicator;

static volatile SignalLimits waveform = {Off, false, 0, WAVEFORM_MAX_PERIOD, WAVEFORM_MIN_PERIOD, WAVEFORM_MAX_PULSE_WIDTH};
static volatile modeIndicator led = {Inactive};

// declare 7-segment codings for waveform type readings
static uint8_t clearCoding[4] = {
                 0b00000000,
                 0b00000000,
                 0b00000000,
                 0b00000000
};
static uint8_t squareCoding[4] = {
                 0b01101101,     // letter 'S'
                 0b00111111,     // letter 'Q'
                 0b00110111,     // letter 'U'
                 0b01111110      // letter 'A'
};

static uint8_t triangleCoding[4] = {
                0b01000110,      // letter 'T'
                0b00001110,      // letter 'R'
                0b00000110,      // letter 'I'
                0b01111110       // letter 'A'
};

static uint8_t sineCoding[4] = {
                0b01101101,      // letter 'S'
                0b00000110,      // letter 'I'
                0b00111110,      // letter 'N'
                0b01001111       // letter 'E'
};

void turnLEDOn(uint8_t pin) {
    GPIOPinWrite(GPIO_PORTF_BASE, pin, pin);
}

void turnLEDOff(uint8_t pin) {
    GPIOPinWrite(GPIO_PORTF_BASE, pin, 0);
}

// waveform functions that use degree as input
static inline double sine(unsigned int degree) {
    double radian = 2 * M_PI * ((double) (degree % 360) / 360);
    return sin(radian);
}

//static inline double triangle(unsigned int degree) {
    // linear function that has a max at 1 and min at 0

//}

// square wave flips between 1 and 0 dependent on the input degree
static inline int square(unsigned int degree) {
    if ((degree / 90) % 2 == 0) {
        return 0;
    }
    else {
        return 1;
    }
}

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

    uint32_t delay = 100;

    int button = pbRead();

    switch (button) {

    case 1:
        // this button is for switching the system on and off
        uprintf("Button 1 pressed\n");

        if (led.state == redMode) {
            turnLEDOff(RED_LED);
            led.state = Inactive;
            waveform.state = SwitchOff;
        }
        else if (led.state == greenMode) {
            turnLEDOff(GREEN_LED);
            led.state = Inactive;
            waveform.state = SwitchOff;
        }
        else if (led.state == blueMode) {
            turnLEDOff(BLUE_LED);
            led.state = Inactive;
            waveform.state = SwitchOff;
        }
        else {
            turnLEDOn(RED_LED);
            seg7Update(sineCoding);
            led.state = redMode;
            waveform.state = SwitchOn;
        }
        break;

    case 2:
        // switches the modes of operation of the system
        uprintf("Button 2 pressed\n");

        if (led.state == redMode) {
            turnLEDOff(RED_LED);
            turnLEDOn(GREEN_LED);
            led.state = greenMode;
            waveform.state = On;
        }
        else if (led.state == greenMode) {
            turnLEDOff(GREEN_LED);
            turnLEDOn(BLUE_LED);
            led.state = blueMode;
            waveform.state = On;
        }
        else if (led.state == blueMode) {
            turnLEDOff(BLUE_LED);
            turnLEDOn(RED_LED);
            led.state = redMode;
            waveform.state = On;
        }
        else {
            uprintf("Cannot change modes when system is inactive\n");
        }
        break;
    }

    if (led.state == Inactive) {
        seg7Update(clearCoding);
    }

    schdCallback(readPushbutton, time + delay);
}

// The buzzer play callback function
void waveformPlay(uint32_t time) {

    static unsigned int angle = 0;
    uint32_t delay = WAVEFORM_CHECK_INTERVAL; // the delay for next callback
    uint32_t arr[2];

    pwm_Waveform currWaveform;

    // WHEN A STATE CHANGES WE WANT TO SAVE THE SETTINGS OF THE PREVIOUS STATE
    arr[0] = checkRotary(0);
    arr[1] = checkRotary(1);

    uprintf("Rotary 1: %d\nRotary 2: %d\n", arr[0], arr[1]);

    uint32_t weight1 = 99 - arr[0] * 100 / 4096;
    uint32_t weight2 = 99 - arr[1] * 100 / 4096;

    if (led.state == redMode) {
        // this mode will manipulate the volume and frequency of the tone
        // weight1 -> volume
        currWaveform.volumeWeight = weight1;
        currWaveform.pulseWidth = weight1 * waveform.maxPulseWidth;
        // weight2 -> frequency
        currWaveform.periodWeight = weight2;
        currWaveform.period = weight2 * ((waveform.maxPeriod - waveform.minPeriod) / 100) + waveform.minPeriod;
    }
    else if (led.state == greenMode) {
        // AT THE MOMENT THIS DOES NOTHING
        // arr[0] -> ?
        // arr[1] -> ?
    }
    else if (led.state == blueMode) {
        // weight1 -> waveform selector DONE!
        switch (weight1 / 34) {
        case 0:
            currWaveform.type = Sine;
            break;
        case 1:
            currWaveform.type = Square;
            break;
        case 2:
            currWaveform.type = Triangle;
            break;
        }
    }
    else {
        seg7Update(clearCoding);
    }

    // VOLUME = DUTY CYCLE = pulseWidth / period
    //currWaveform.period = currWaveform.periodWeight * ((waveform.maxPeriod - waveform.minPeriod) / 100) + waveform.minPeriod;

    if (led.state != Inactive) {
        switch (currWaveform.type) {
        case Sine:
            seg7Update(sineCoding);
            currWaveform.pulseWidth = (currWaveform.volumeWeight / 10) * (sine(angle) * waveform.maxPulseWidth);
            break;
        case Square:
            seg7Update(squareCoding);
            currWaveform.pulseWidth = (currWaveform.volumeWeight / 10) * (square(angle) * waveform.maxPulseWidth);
            break;
        case Triangle:
            seg7Update(triangleCoding);
            // apply triangle wave function to pulseWidth
            break;
        }
    }

    switch (waveform.state) {
    case Off:
        break;
    case On:
        waveformPwmSet(currWaveform);
        break;
    case SwitchOff:
        waveform.state = Off;
        waveformOff();
        break;
    case SwitchOn:
        waveform.state = On;
        waveformPwmSet(currWaveform);
        break;
    }

    angle++;
    // schedule the next callback
    schdCallback(waveformPlay, time + delay);
}

void main(void)
{
    lpInit();
    seg7Init();
    initRotary();
    waveformInit();

    schdCallback(waveformPlay, 1001);
    schdCallback(readPushbutton, 1002);

    // Loop forever
    while (true)
    {
        schdExecute();
    }
}
