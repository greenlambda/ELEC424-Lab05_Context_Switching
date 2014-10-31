/*
 * systick_context_switcher.h - A basic scheduler.
 *
 *  Created on: Oct 27, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */
#ifndef SYSTICK_CONTEXT_SWITCHER_H_
#define SYSTICK_CONTEXT_SWITCHER_H_

#include <sys/types.h>

/* 10ms Scheduler Ticks */
#define HIGH_PRIO_TICK_FREQ		100
#define LOW_PRIO_TICK_FREQ		2
#define APB1_FREQ				36000000
#define APB2_FREQ				36000000


/* Create a quick boolean type */
typedef int boolean;
#define TRUE	1
#define FALSE	0

/* Define the exception stack frame */
typedef struct exception_stack_frame_s {
	uint32_t R0;
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R12;
	uint32_t LR;
	uint32_t PC;
	uint32_t xPSR;
} __attribute__((packed, aligned(4))) exception_stack_frame_t;

#endif /* SYSTICK_CONTEXT_SWITCHER_H_ */
