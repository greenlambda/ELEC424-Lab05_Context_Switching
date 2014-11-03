/*
 * thread.h - Manages creation and destruction of threads.
 *
 *  Created on: Nov 2, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */

#ifndef THREAD_H_
#define THREAD_H_

void thread_create(void (*thread)(void), void* stack_top);

/*
 * The thread control block contains
 */
typedef struct thread_control_block_s {
	/*
	 * The context for a thread is stored on that thread's stack, so we just
	 * need to save it's stack pointer to save all of the context.
	 */
	void* sp;
} thread_control_block_t;

#endif /* THREAD_H_ */
