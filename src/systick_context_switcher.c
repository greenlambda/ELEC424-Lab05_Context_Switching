/*
 * Name: systick_context_switcher.c
 * Authors: Jeremy Hunt and Christopher Buck
 * Creation Date: Oct 27, 2014
 * Description: A simple task scheduler for varying priority tasks.
 */

#include "systick_context_switcher.h"

#include "thread.h"
#include "sys_init.h"
#include "tasks.h"

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

/* Here are the thread control blocks. */
thread_control_block_t threads[2];

/* These are the other stacks */
#define STACK_LEN	2048
uint8_t stack_a[STACK_LEN];
uint8_t stack_b[STACK_LEN];

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
	SysTick->LOAD = (SYSTICK_COUNT & SysTick_LOAD_RELOAD_Msk) - 1;
	/* Set Priority for Cortex-M0 System Interrupts */
	NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	/* Load the SysTick Counter Value */
	SysTick->VAL = 0;
	/* Enable SysTick IRQ and SysTick Timer */
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

/*
 * Main function.  Initializes the GPIO, Timers, and
 */
int main() {
	/* Turn off non-essential interrupts. */
	__disable_irq();

	/* Initialize the system clock, motors, and leds */
	init_system_clk();
	init_motors();
	init_blink();

	/* Set up the SysTick for 1ms intervals */
	systick_init();
	/* TODO: Set prio of pendSV to lowest */

	/* Create the init thread. */
	thread_create_init(&threads[0], task_blink_led, stack_a + STACK_LEN);

	/* Create the non-main thread. */
	thread_create(&threads[1], task_spin_motors, stack_b + STACK_LEN);

	/* Start the scheduler. This starts the init thread and enables all interrupts. */
	thread_scheduler_start();

	/* Loop. Forever. We should never get here. */
	for (;;) {
	}
}

