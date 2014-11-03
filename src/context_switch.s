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
.align  2

.text

/*
 * C Callable Function: stack_switch
 * Switch the stack to the PSP from the MSP and execute the given function.
 *
 * extern int stack_switch(int (*thread)(void), void* new_stack_top);
 */
.thumb_func
.global		stack_switch
stack_switch:
	PUSH	{lr}
/* Set the top of the stack */
	MSR	PSP, r1
/*
 * Switch over to PSP, the second bit in the CONTROL register.
 * We can't access any stack variables past this point.
 */
	MRS	r2, CONTROL
	ORR.W	r2, r2, #2
	MSR	CONTROL, r2
/* Turn on interupts. This starts the scheduler for systick. */
	CPSIE I
/* Call the function. This sets PC and LR. The function sets R0. */
	BLX	r0


/*
 * When thread ends, it should return here. Switch back the stack.
 * The function may have modified R0-R3. r0 is the return.
 */
.thumb_func
.global		ThreadExitReturn
ThreadExitReturn:
	MRS	r2, CONTROL
	BIC.W	r2, r2, #2
	MSR	CONTROL, r2
/* We are now back to the MSR stack. */
/* Return to the function */
	POP	{lr}
	BX	lr
	.size	stack_switch, .-stack_switch


/* TODO: Remove me! */
.extern stack_b_top

/*
 * The SysTick_Handler switches from one thread to another.
 *
 * void SysTick_Handler();
 */
/* Declare this a thumb_func to make sure the hardware jumps to an odd address */
.thumb_func
.global SysTick_Handler
SysTick_Handler:
/*
 * We have just pushed all of the hardware exception stack frame, now push the software
 * exception stack frame.
 */
 	MRS	r0, PSP
 	SUB	r0, r0, #32
	MSR	PSP, r0
	ADD r0, r0, #32
	STMDB	r0!, {r4, r5, r6, r7, r8, r9, r10, r11}

// void* thread_get_next_stack_top(void* cur_stack_top);
	BL	thread_get_next_stack_top

/* TODO: Save the old PSP somewhere. */
/* TODO: Write and call a function to return where to get the new stack and get rid of this code. */
	LDR	r1, =stack_b_top
	LDR r0, [r1]
	SUB r0, r0, #64

/* Re-pop the software exception stack frame in the new context. */
	LDMIA	r0!, {r4, r5, r6, r7, r8, r9, r10, r11}

/* Switch to the new stack by writing to PSP */
	MSR	PSP, r0

/* Finally return to thread mode. TODO: Change this to a PendSV so that we don't fuck up other interupts. */
	LDR r2, =return_to_thread_val
	LDR r1, [r2]
	BX r1

	.size	SysTick_Handler, .-SysTick_Handler


.align 4
return_to_thread_val:
	.word 0xFFFFFFFD



