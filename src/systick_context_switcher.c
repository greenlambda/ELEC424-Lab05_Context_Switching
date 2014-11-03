/*
 * Name: systick_context_switcher.c
 * Authors: Jeremy Hunt and Christopher Buck
 * Creation Date: Oct 27, 2014
 * Description: A simple task scheduler for varying priority tasks.
 */

#include "systick_context_switcher.h"

#include "motor.h"
#include "thread.h"
#include "sys_clk_init.h"
#include "sys_init.h"

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/* These are the other stacks */
#define STACK_LEN	2048
uint8_t stack_a[STACK_LEN];
uint8_t stack_b[STACK_LEN];
uint8_t* stack_b_top = (stack_b + sizeof(stack_b));

extern int scheduler_init(int (*thread)(void), void* new_stack_top);

/*
 * Initialize the SysTick interrupt for 1ms.
 */
void systick_init() {
	/* Set up the SysTick interrupt  */
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	/*
	 * This sets the time-base and starts the interrupt.
	 * Don't use SysTick_Config() because it has a bug in it so that it always uses the
	 * processor clock...
	 */
	 /* Set reload register */
	 SysTick->LOAD  = (SYSTICK_COUNT & SysTick_LOAD_RELOAD_Msk) - 1;
	 /* Set Priority for Cortex-M0 System Interrupts */
	 NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
	 /* Load the SysTick Counter Value */
	 SysTick->VAL   = 0;
	 /* Enable SysTick IRQ and SysTick Timer */
	 SysTick->CTRL  = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
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
 * Blink the light without interrupts to test a basic thread.
 */
int blink_light_2() {
	unsigned int greenLedState = 0;
	volatile int i;
	int j;

	for (j = 0; j < 20; ++j) {
		/* Toggle the green led */
		if (greenLedState) {
			motor_set(Motor1, 10);
		} else {
			motor_set(Motor1, 0);
		}
		/* Flip the state for next operation */
		greenLedState = 1 - greenLedState;

		for (i = 0; i < 1000000; ++i) {}
	}

	return 0xA5A55A5A;
}


/*
 * test comment
 */
//void* thread_get_next_stack_top(void* thread_cur_stack)
//{
//	static void* next = stack_b;	//initially set to B to be reset to A
//
//	if (next == stack_b)
//		next = stack_a;
//	else
//		next = stack_b;
//	return next;
//}



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

	/* Create the non-main thread. */
	thread_create(blink_light_2, (stack_b + sizeof(stack_b)));

	/* Start the scheduler. This starts the given thread and enables all interrupts. */
	scheduler_init(blink_light, (stack_a + sizeof(stack_a)));

	/* Loop. Forever. */
	for (;;) {
	}
}

