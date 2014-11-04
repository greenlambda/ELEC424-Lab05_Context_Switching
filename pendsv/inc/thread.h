/*
 * thread.h - Manages creation and destruction of threads.
 *
 *  Created on: Nov 2, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */

#ifndef THREAD_H_
#define THREAD_H_

#include "doubly_linked_list.h"

/*
 * The thread control block contains
 */
typedef struct thread_control_block_s {
	/*
	 * The context for a thread is stored on that thread's stack, so we just
	 * need to save it's stack pointer to save all of the context.
	 */
	void* sp;

	/* Pointers to other thread lists. */
	dl_list_t sched_elem;
} thread_control_block_t;

void thread_create_init(thread_control_block_t* first_thread, void (*thread_func)(void), void* stack_top);
void thread_create(thread_control_block_t* thread, void (*thread_func)(void), void* stack_top);
void thread_scheduler_start();

#endif /* THREAD_H_ */
