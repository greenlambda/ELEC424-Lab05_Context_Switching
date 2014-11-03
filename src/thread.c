/*
 * thread.c - Manages creation and destruction of threads.
 *
 *  Created on: Nov 2, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */

#include "thread.h"

#include "systick_context_switcher.h"

#include <stdint.h>

/* The thread return place in inline assembly */
extern void ThreadExitReturn();

/*
 * Initialize a thread's stack so that when it is context switched to it will start the given
 * function with the correct state. This also sets up the pc so that when the thread exits,
 * it jumps to the ThreadExitReturn, which switches to the main control loop taking
 * return values and cleaning up the thread.
 */
static void thread_stack_init(int (*thread)(void), void* stack_top) {
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
 * Create a new thread using the given call function.
 */
void thread_create(int (*thread)(void), void* stack_top) {
	thread_stack_init(thread, stack_top);
}

