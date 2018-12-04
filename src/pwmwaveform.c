#include <pwmwaveform.h>
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "launchpad.h"

#define WTIMER0         WTIMER0_BASE
#define WTIMER_PERIPH   SYSCTL_PERIPH_WTIMER0
#define BUZZER_PERIPH   SYSCTL_PERIPH_GPIOC
#define BUZZER_PORT     GPIO_PORTC_BASE
#define BUZZER_PIN      GPIO_PIN_4

/* Initialize like the leds from pwmled.c
 * for jumper 16?
 * BUZZER:         PC4, WT0CCP0, Timer 0-A
 */

void waveformInit() {
    SysCtlPeripheralEnable(WTIMER_PERIPH);
    SysCtlPeripheralEnable(BUZZER_PERIPH);

    GPIOPinTypeTimer(BUZZER_PORT, BUZZER_PIN);
    GPIOPinConfigure(GPIO_PC4_WT0CCP0);

    TimerConfigure(WTIMER0, (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM));
    TimerControlLevel(WTIMER0, TIMER_A, true);

    TimerLoadSet(WTIMER0, TIMER_A, 200);
    TimerMatchSet(WTIMER0, TIMER_A, 0);

    TimerEnable(WTIMER0, TIMER_A);
}

void waveformOn() {
    TimerLoadSet(WTIMER0, TIMER_A, 500000);
    TimerMatchSet(WTIMER0, TIMER_A, 250000);
}

void waveformOff() {
    TimerLoadSet(WTIMER0, TIMER_A, 0);
    TimerMatchSet(WTIMER0, TIMER_A, 0);
}

void waveformPwmSet(pwm_Waveform waveform) {
    TimerLoadSet(WTIMER0, TIMER_A, waveform.period);
    TimerMatchSet(WTIMER0, TIMER_A, waveform.pulseWidth);
}


