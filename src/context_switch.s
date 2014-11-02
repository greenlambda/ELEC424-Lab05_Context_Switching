/*
 * context_switch.s - Assembly level functions and code for dealing with registers.
 *
 * Created on: Nov 2, 2014
 *     Author: Jeremy Hunt and Christopher Buck
 *      Class: ELEC 424
 */

.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.text

/*
 * C Callable Function: stack_switch
 * Switch the stack to the PSP from the MSP and execute the given function.
 *
 * extern int stack_switch(int (*thread)(void), void* new_stack_top);
 */
.global		stack_switch
stack_switch:
	PUSH	{lr}
/* -1 Is the defualt return value */
	MOV.W	r4, #-1
/* Set the top of the stack */
	MSR	PSP, r1
/*
 * Switch over to PSP, the second bit in the CONTROL register.
 * We can't access any stack variables past this point.
 */
	MRS	r2, CONTROL
	ORR.W	r2, r2, #2
	MSR	CONTROL, r2
/* Call the function. This sets PC and LR. The function sets R0. */
	BLX	r0
/*
 * When thread ends, it should return here. Switch back the stack.
 * The function may have modified R0-R3. r0 is the return.
 */
.global		ThreadExitReturn
ThreadExitReturn:
	MRS	r2, CONTROL
	BIC.W	r2, r2, #2
	MSR	CONTROL, r2
/* We are now back to the MSR stack. */
/* Return to the function */
	POP	{lr}
	BX	lr


/* TODO: Remove me! */
.extern stack_b_top

/*
 * The SysTick_Handler switches from one thread to another.
 *
 * void SysTick_Handler();
 */
 // TODO: Why you no work!!! :C
.global SysTick_Handler
SysTick_Handler:
	BX lr
/*
 * We have just pushed all of the hardware exception stack frame, now push the software
 * exception stack frame.
 */
//	PUSH {r4-r11}
/* TODO: Write and call a function to return where to get the new stack. */
//	LDR	r0, =stack_b_top
/* Switch to the new stack by writing to PSP */
//	MSR	PSP, r0
/* Re-pop the software exception stack frame in the new context. */
//	POP {r4-r11}
/* Finally return to thread mode. TODO: Change this to a PendSV so that we don't fuck up other interupts. */
//	LDR pc, =return_to_thread_val
return_to_thread_val:
	.word 0xFFFFFFFD



