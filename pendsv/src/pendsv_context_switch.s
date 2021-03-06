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
.global		scheduler_init
scheduler_init:
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
/* We are now back to the MSP stack. */
/* Return to the function */
	POP	{lr}
	BX	lr
	.size	scheduler_init, .-scheduler_init


/* uint32_t thread_switch_info(uint32_t thread_cur_stack) */
.extern thread_switch_info

.thumb_func
.global PendSV_Handler
PendSV_Handler:
/* Figuring out which thread to switch to and which stack is a critical section. */
	CPSID	i

/*
 * We have just pushed all of the hardware exception stack frame, now push the software
 * exception stack frame and adjust the PSP.
 */
 	MRS	r0, PSP
 	SUB	r0, r0, #32
	MSR	PSP, r0
	ADD r0, r0, #32
	STMDB	r0!, {r4, r5, r6, r7, r8, r9, r10, r11}

/* Load the current stack pointer into r0 */
	MRS	r0, PSP

/* Get the next thread stack to switch to and save the current PSP. Both in r0 */
	BL	thread_switch_info

/* Re-pop the software exception stack frame in the new context. */
	LDMIA	r0!, {r4, r5, r6, r7, r8, r9, r10, r11}

/* Switch to the new stack by writing to PSP */
	MSR	PSP, r0

/* End critical section. Everything past here is read only. */
	CPSIE	i

/*
 * Finally return to thread mode. PendSV is the lowest priority, so we don't
 * need to.
 */
	LDR r2, =return_to_thread_val
	LDR r1, [r2]
	BX r1
	.size PendSV_Handler, .-PendSV_Handler

.align 4
return_to_thread_val:
	.word 0xFFFFFFFD



