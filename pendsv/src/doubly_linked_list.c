/*
 * doubly_linked_list.c - Manages a doubly linked list.
 *
 *  Created on: Mar 24, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */

#include "doubly_linked_list.h"

#include "systick_context_switcher.h"

#include <stdint.h>
#include <stddef.h>


/*
 * In this implementation of a doubly linked list, the head of the list is an element in the
 * list. The head cannot be removed from the list.
 */

/*
 * This initializes a doubly linked list so that it is empty
 * (e.g. the head points to itself).
 */
void dl_list_init(dl_list_head_t* head) {
	head->next = head;
	head->prev = head;
}

/*
 * Returns TRUE iff the doubly linked list headed by head is empty. Head can actually be any
 * element in the list, because the list must always contain the head element, so if this is
 * called on anything but the head, the list must not be empty.
 */
boolean dl_list_is_empty(dl_list_head_t* head) {
	if (head->next == head && head->prev == head) {
		return TRUE;
	} else {
		return FALSE;
	}
}


/*
 * Inserts an element before the given point in the doubly linked list.
 */
void dl_list_insert_before(dl_list_t* to_insert, dl_list_t* dest) {
	/* Link the new process to the dest. */
	to_insert->next = dest;
	to_insert->prev = dest->prev;
	/* Link the previous process to the new process */
	dest->prev->next = to_insert;
	/* Link the next process to the new process */
	dest->prev = to_insert;
}

/*
 * Helper function to insert an element to be the first of a list given the head.
 */
void dl_list_insert_first(dl_list_t* to_insert, dl_list_head_t* head) {
	dl_list_insert_before(to_insert, head->next);
}

/*
 * Helper function to insert an element at the end of a list with the given head.
 */
void dl_list_append(dl_list_t* to_insert, dl_list_head_t* head) {
	dl_list_insert_before(to_insert, head);
}

/*
 * Removes the element at the given point from a doubly linked list.
 *
 * Updates the given head if needed.
 */
void dl_list_remove(dl_list_t* to_remove) {
	/* Skip over the links to this process. */
	to_remove->prev->next = to_remove->next;
	to_remove->next->prev = to_remove->prev;
	/* Null the internal pointers. */
	to_remove->next = NULL;
	to_remove->prev = NULL;
}

/*
 * Helper function to remove the first element of a list given the head.
 */
void dl_list_remove_first(dl_list_head_t* head) {
	dl_list_remove(head->next);
}

