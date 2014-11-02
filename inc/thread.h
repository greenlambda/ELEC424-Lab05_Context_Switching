/*
 * thread.h - Manages creation and destruction of threads.
 *
 *  Created on: Nov 2, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */

#ifndef THREAD_H_
#define THREAD_H_

void thread_create(int (*thread)(void), void* stack_top);

#endif /* THREAD_H_ */
