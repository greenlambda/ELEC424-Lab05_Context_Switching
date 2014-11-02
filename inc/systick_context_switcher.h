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
#define ESF_LEN			32
#define ESF_OFFSET_R0	0x00
#define ESF_OFFSET_R1	0x04
#define ESF_OFFSET_R2	0x08
#define ESF_OFFSET_R3	0x0C
#define ESF_OFFSET_R12	0x10
#define ESF_OFFSET_LR	0x14
#define ESF_OFFSET_PC	0x18
#define ESF_OFFSET_xPSR	0x1C

#define SSF_LEN			32
#define SSF_OFFSET_R4	0x00
#define SSF_OFFSET_R5	0x04
#define SSF_OFFSET_R6	0x08
#define SSF_OFFSET_R7	0x0C
#define SSF_OFFSET_R8	0x10
#define SSF_OFFSET_R9	0x14
#define SSF_OFFSET_R10	0x18
#define SSF_OFFSET_R11	0x1C

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
