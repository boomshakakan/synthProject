/*
 * rotary.c
 *
 *  Created on: Oct 16, 2018
 *      Author: mattstevenson
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "launchpad.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

void initRotary() {

    // Enable module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Wait for module ready
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_ADC0))
    {
    }

    // Jumper 9 -> channel 2
    // Jumper 8 -> channel 4

    // Enable first sample sequencer to capture the value of channel 0 when the processor trigger occurs
    ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_CH2);
    ADCSequenceStepConfigure(ADC0_BASE, 0, 1, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH4);
    ADCIntClear(ADC0_BASE, 0);
    ADCSequenceEnable(ADC0_BASE, 0);
}

void rotaryRead(uint32_t val[]) {

    // Trigger sample sequence
    ADCProcessorTrigger(ADC0_BASE, 0);

    // Wait until the sample sequence has completed
    while(!ADCIntStatus(ADC0_BASE, 0, false))
    {
    }

    ADCSequenceDataGet(ADC0_BASE, 0, val);
    ADCIntClear(ADC0_BASE, 0);
}


