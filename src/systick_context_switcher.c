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

/* This is the second stack */
uint8_t stack_a[2048];

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
			"BLX %[funcptr]\n\t"
			/* When thread ends, it should return here. Switch back the stack. */
    		"MRS r1, control\n\t"
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
	if (sys_clk_init_72mhz() != SUCCESS) {
		/* We failed. Stop. */
		for (;;) {}
	}

	leds_init();
	motor_pins_init();
	motor_timers_init();
	motor_pwm_init();

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

