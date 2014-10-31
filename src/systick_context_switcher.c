/*
 * Name: systick_context_switcher.c
 * Authors: Jeremy Hunt and Christopher Buck
 * Creation Date: Oct 27, 2014
 * Description: A simple task scheduler for varying priority tasks.
 */

#include "systick_context_switcher.h"

#include "motor.h"
#include "sys_clk_init.h"

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/* These are the other stacks */
#define STACK_LEN	2048
uint8_t stack_a[STACK_LEN];
uint8_t stack_b[STACK_LEN];

/* The thread return place in inline assembly */
extern void ThreadExitReturn();

/*
 * Initialize the SysTick interrupt for 1ms.
 */
void systick_init() {
	/* Set up the SysTick interrupt  */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	/* This sets the time-base and starts the interrupt */
	if (SysTick_Config(SYSTICK_COUNT)) {
		/* We failed. Stop. */
		for (;;) {}
	}
}

/*
 * Initialize the GPIO which controls the LED
 */
static void leds_init() {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* The Green LED is connected to PB5. Init it and turn it on. */
	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = GPIO_Pin_5;
	gpioStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpioStructure);
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
}

/*
 * Blink the light without interrupts to test a basic thread.
 */
int blink_light() {
	unsigned int greenLedState = 0;
	volatile int i;
	int j;

	for (j = 0; j < 20; ++j) {
		/* Toggle the green led */
		if (greenLedState) {
			GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
		} else {
			GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);
		}
		/* Flip the state for next operation */
		greenLedState = 1 - greenLedState;

		for (i = 0; i < 1000000; ++i) {}
	}

	return 0xDEADBEEF;
}

/*
 * Initialize a thread's stack so that when it is context switched to it will start the given
 * function with the correct state. This also sets up the pc so that when the thread exits,
 * it jumps to the ThreadExitReturn, which switches to the main control loop taking
 * return values and cleaning up the thread.
 */
void stack_thread_init(int (*thread)(void), void* stack_top) {
	/* Figure out where the exception and software stack frames should be on the stack */
	uint32_t* esf = (uint32_t*)((uint8_t*)stack_top - ESF_LEN);
	uint32_t* ssf = (uint32_t*)((uint8_t*)stack_top - ESF_LEN - SSF_LEN);

	/* Artificially push the information that would normally be pushed by hardware. */
	/* Registers are all 0 */
	esf[ESF_OFFSET_R0] = 0;
	esf[ESF_OFFSET_R1] = 0;
	esf[ESF_OFFSET_R2] = 0;
	esf[ESF_OFFSET_R3] = 0;
	esf[ESF_OFFSET_R12] = 0;
	/* The thread should return to ThreadExitReturn on exit */
	esf[ESF_OFFSET_LR] = (uint32_t)(&ThreadExitReturn);
	/* The thread should start at the given handler */
	esf[ESF_OFFSET_PC] = (uint32_t)(thread);
	/* This is the reset value of the PSR  TODO: Should this be something else? */
	esf[ESF_OFFSET_xPSR] = 0x01000000;

	/* Now push the info for the software (i.e. the other registers) */
	ssf[SSF_OFFSET_R4] = 0;
	ssf[SSF_OFFSET_R5] = 0;
	ssf[SSF_OFFSET_R6] = 0;
	ssf[SSF_OFFSET_R7] = 0;
	ssf[SSF_OFFSET_R8] = 0;
	ssf[SSF_OFFSET_R9] = 0;
	ssf[SSF_OFFSET_R10] = 0;
	ssf[SSF_OFFSET_R11] = 0;
}

/*
 * Switch the stack to the PSP from the MSP and execute the given function.
 */
int stack_switch(int (*thread)(void), void* new_stack_top) {
	int ret_value = -1;

	asm volatile (
			/* Set the top of the stack */
			"MSR psp, %[stacktop]\n\t"
			/*
			 * Switch over to PSP, the second bit in the CONTROL register.
			 * We can't access any local or stack variables past this point.
			 */
    		"MRS r0, control\n\t"
    		"ORR r0, r0, #0x2\n\t"
    		"MSR control, r0\n\t"
			/* Call the function. This sets PC and LR. The function sets R0. */
			"BLX %[funcptr]\n"
			/* When thread ends, it should return here. Switch back the stack. */
    		"ThreadExitReturn:	MRS r1, control\n\t"
    		"BIC r1, r1, #0x2\n\t"
    		"MSR control, r1\n\t"
			/* Save the return value. */
			"MOV %[retval], r0"
			/* Write the return value */
			: [retval]"=r" (ret_value)
			/* Read new top of stack and the function pointer */
			: [stacktop]"r" ((uint32_t)new_stack_top),
			  [funcptr]"r" ((uint32_t)thread)
			/* Clobber r0-r1 and lr (since we call the function) */
			: "r0", "r1", "lr");

   return ret_value;
}

/*
 * Main function.  Initializes the GPIO, Timers, and
 */
int main() {
	/* Turn off non-essential interrupts. */
	__disable_irq();

	if (sys_clk_init_72mhz() != SUCCESS) {
		/* We failed. Stop. */
		for (;;) {}
	}

	leds_init();
	motor_pins_init();
	motor_timers_init();
	motor_pwm_init();
	systick_init();

	/* Turn on interrupts */
	__enable_irq();

	/* Loop. Forever. */
	for (;;) {
		stack_switch(blink_light, (stack_a + sizeof(stack_a)));
	}
}

/*
 * Interrupt service routines.
 * Function callers setup in "startup_stm32f10x_md.s"
 */
void SysTick_Handler() {

}

