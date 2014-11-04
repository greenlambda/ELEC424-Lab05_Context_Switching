/*
 * thread.c - Manages creation and destruction of threads.
 *
 *  Created on: Nov 2, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */

#include "thread.h"

#include "pendsv_context_switcher.h"

#include "doubly_linked_list.h"

#include "core_cm3.h"

#include <stdint.h>

/* The thread return place in inline assembly */
extern void ThreadExitReturn();
/* The function that starts the first function. */
extern int scheduler_init(void (*thread_thread)(void), void* new_stack_top);

/* Store info for the thread scheduler */
thread_control_block_t* active_thread;
dl_list_head_t ready_threads;
uint32_t sp_temp_store;

/*
 * Initialize a thread's stack so that when it is context switched to it will start the given
 * function with the correct state. This also sets up the pc so that when the thread exits,
 * it jumps to the ThreadExitReturn, which switches to the main control loop taking
 * return values and cleaning up the thread.
 */
static void thread_stack_init(thread_control_block_t* thread, void (*thread_func)(void), void* stack_top) {
	/* Figure out where the exception and software stack frames should be on the stack */
	uint32_t* esf = (uint32_t*)stack_top - ESF_LEN_WORDS;
	uint32_t* ssf = (uint32_t*)stack_top - (ESF_LEN_WORDS + SSF_LEN_WORDS);

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
	esf[ESF_OFFSET_PC] = (uint32_t)(thread_func);
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

	/*
	 * Finally, store the stack pointer into the tcb.
	 * Since it hasn't started yet, this is the end of the ssf.
	 */
	thread->sp = (void*)ssf;
}

/*
 * Create a new thread using the given call function.
 */
void thread_create(thread_control_block_t* thread, void (*thread_func)(void), void* stack_top) {
	/* Initialize the stack. */
	thread_stack_init(thread, thread_func, stack_top);
	/* Initialize the schedule list and add it to the ready list. */
	dl_list_append(&(thread->sched_elem), &ready_threads);
}

/*
 * Creates the first thread to run and sets up the scheduler.
 */
static thread_control_block_t* init_thread;
static void (*init_thread_func)(void);
void thread_create_init(thread_control_block_t* first_thread, void (*thread_func)(void), void* stack_top) {
	/* Init the ready list. */
	dl_list_init(&ready_threads);
	/* Cache the values for later */
	init_thread = first_thread;
	init_thread_func = thread_func;
	/* Set up the stack by leaving it empty. */
	init_thread->sp = stack_top;
	/* Initialize the sched elem */
	dl_list_init(&(init_thread->sched_elem));
}

/* Initialize the thread scheduler and starts the first thread. */
void thread_scheduler_start() {
	active_thread = init_thread;
	scheduler_init(init_thread_func, init_thread->sp);
}

/*
 * This function is called once each SysTick. It takes in the current stack
 * pointer and returns the stack pointer to switch to. If no switch is to
 * take place, it should return the current stack pointer.
 */
static const int32_t sched_elem_offset = GET_OFFSET(active_thread, &(active_thread->sched_elem));
void* thread_tick(void* sp) {
	/* Only switch every 5000 ticks. 5s */
	static int tick_count = 0;
	if (tick_count < 5000) {
		tick_count++;
		return sp;
	} else {
		tick_count = 0;

		/* Context switch. Use the active and ready queues. */
		thread_control_block_t* next_thread = GET_CONTAINER(ready_threads.next, sched_elem_offset, thread_control_block_t*);
		dl_list_remove_first(&ready_threads);

		/* Update the active thread's stack pointer */
		active_thread->sp = sp;

		/* Put the active thread on the ready_thread list. */
		dl_list_append(&(active_thread->sched_elem), &ready_threads);

		/* Make the next thread the active thread */
		active_thread = next_thread;

		/* Switch to the new active thread */
		return active_thread->sp;
	}
}
