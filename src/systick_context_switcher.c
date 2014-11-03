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

/* These are the other stacks */
#define STACK_LEN	2048
uint8_t stack_a[STACK_LEN];
uint8_t stack_b[STACK_LEN];
uint8_t* stack_b_top = (stack_b + sizeof(stack_b));

extern int scheduler_init(void (*thread)(void), void* new_stack_top);

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
 * Get the stack pointer for the next stack to switch to
 */
uint32_t pointer_b = (uint32_t)stack_b + sizeof(stack_b) - ((ESF_LEN_WORDS + SSF_LEN_WORDS)*4);
uint32_t pointer_a;
uint32_t thread_get_next_stack_top(uint32_t thread_cur_stack) {
	static int is_stack_a = 0;	//state variable

	/* Stack we are going to is B */
	if (is_stack_a == 0) {
		/* Next time, going to stack A */
		is_stack_a = 1;
		pointer_a = thread_cur_stack;
	} else {
		/* Stack we are going to is A */
		is_stack_a = 0;
		pointer_b = thread_cur_stack;
	}
	/* Inverted because we flipped the state variable */
	return (uint32_t)(is_stack_a ? pointer_b : pointer_a);
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

	/* Create the non-main thread. */
	thread_create(task_spin_motors, (stack_b + sizeof(stack_b)));

	/* Start the scheduler. This starts the given thread and enables all interrupts. */
	scheduler_init(task_blink_led, (stack_a + sizeof(stack_a)));

	/* Loop. Forever. */
	for (;;) {
	}
}

