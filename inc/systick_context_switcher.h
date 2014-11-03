/*
 * systick_context_switcher.h - A basic scheduler.
 *
 *  Created on: Oct 27, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */
#ifndef SYSTICK_CONTEXT_SWITCHER_H_
#define SYSTICK_CONTEXT_SWITCHER_H_

#include <sys/types.h>

#define AHB_FREQ				72000000
#define APB1_FREQ				(AHB_FREQ/2)
#define APB2_FREQ				(AHB_FREQ/2)

/* 1ms SysTick Interrupt */
#define SYSTICK_IN_FREQ			(AHB_FREQ/8)
#define SYSTICK_FREQ			1
#define SYSTICK_COUNT			(SYSTICK_IN_FREQ/SYSTICK_FREQ)

/* Create a quick boolean type */
typedef int boolean;
#define TRUE	1
#define FALSE	0

/* Define the exception stack frame */
#define ESF_LEN_WORDS	8
#define ESF_OFFSET_R0	0
#define ESF_OFFSET_R1	1
#define ESF_OFFSET_R2	2
#define ESF_OFFSET_R3	3
#define ESF_OFFSET_R12	4
#define ESF_OFFSET_LR	5
#define ESF_OFFSET_PC	6
#define ESF_OFFSET_xPSR	7

#define SSF_LEN_WORDS	8
#define SSF_OFFSET_R4	0
#define SSF_OFFSET_R5	1
#define SSF_OFFSET_R6	2
#define SSF_OFFSET_R7	3
#define SSF_OFFSET_R8	4
#define SSF_OFFSET_R9	5
#define SSF_OFFSET_R10	6
#define SSF_OFFSET_R11	7

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

/*  */

#endif /* SYSTICK_CONTEXT_SWITCHER_H_ */
