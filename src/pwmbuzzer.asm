				.cdecls "stdint.h", "stdbool.h", "inc/hw_memmap.h", "driverlib/sysctl.h", "driverlib/gpio.h", "driverlib/pin_map.h", "driverlib/timer.h", "pwmbuzzer.h"

				.text

WTIMER_PERIPH 	.field 	SYSCTL_PERIPH_WTIMER0
BUZZER_PERIPH	.field	SYSCTL_PERIPH_GPIOC
WTIMER0			.field	WTIMER0_BASE
PORTC			.field	GPIO_PORTC_BASE
PIN_ROUTE_1		.field	GPIO_PC4_WT0CCP0

				.asmfunc
buzzerInit		PUSH	{lr}

				LDR 	r0, WTIMER_PERIPH
				BL		SysCtlPeripheralEnable

				LDR 	r0, BUZZER_PERIPH
				BL		SysCtlPeripheralEnable

				LDR 	r0, PORTC
				MOV		r1, #GPIO_PIN_4
				BL		GPIOPinTypeTimer

				LDR 	r0, PIN_ROUTE_1
				BL		GPIOPinConfigure

				LDR		r0, WTIMER0
				MOV		r1, #TIMER_CFG_SPLIT_PAIR
				ORR		r1, #TIMER_CFG_A_PWM
				BL		TimerConfigure

				LDR 	r0, WTIMER0
				MOV 	r1, #TIMER_A
				MOV		r2, #true
				BL 		TimerControlLevel

				LDR 	r0, WTIMER0
				MOV 	r1, #TIMER_A
				MOV 	r2, #200
				BL		TimerLoadSet

				LDR 	r0, WTIMER0
				MOV 	r1, #TIMER_A
				MOV		r2, #0
				BL		TimerMatchSet

				LDR		r0, WTIMER0
				MOV 	r1, #TIMER_A
				BL		TimerEnable

				POP		{pc}
				.endasmfunc

				.asmfunc
buzzerOff		PUSH 	{lr}

				LDR		r0, WTIMER0
				MOV 	r1, #TIMER_A
				MOV		r2, #0
				BL		TimerLoadSet

				LDR		r0, WTIMER0
				MOV		r1, #TIMER_A
				MOV		r2, #0
				BL		TimerMatchSet

				POP		{pc}
				.endasmfunc

				.asmfunc
buzzerPwmSet	PUSH	{lr, r0}

				MOV		r2, r0, LSR #16
				LDR		r0, WTIMER0
				MOV 	r1, #TIMER_A
				BL		TimerLoadSet

				LDR		r0, WTIMER0
				MOV		r1, #TIMER_A
				LDR 	r2, [sp, #0]
				BFC		r2, #16, #16
				BL		TimerMatchSet

				ADD		sp, #4
				POP		{pc}
				.endasmfunc


