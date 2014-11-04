/*
 * doubly_linked_list.h - Manages a doubly linked list.
 *
 *  Created on: Mar 24, 2014
 *      Author: Jeremy Hunt and Christopher Buck
 */
#ifndef DOUBLY_LINKED_LIST_H_
#define DOUBLY_LINKED_LIST_H_

#include "systick_context_switcher.h"

/*
 * A doubly linked list is a linked structure made of these.
 */
typedef struct dl_list_s {
	struct dl_list_s* next;
	struct dl_list_s* prev;
} dl_list_t;

/*
 * A head is just an element, but define this for readability.
 */
typedef dl_list_t dl_list_head_t;

void dl_list_init(dl_list_head_t* head);
boolean dl_list_is_empty(dl_list_head_t* head);
void dl_list_insert_before(dl_list_t* to_insert, dl_list_t* dest);
void dl_list_insert_first(dl_list_t* to_insert, dl_list_head_t* head);
void dl_list_append(dl_list_t* to_insert, dl_list_head_t* head);
void dl_list_remove(dl_list_t* to_remove);
void dl_list_remove_first(dl_list_head_t* head);

/*
 * Useful macros for managing lists.
 */
/*
 * Loop over each element in a list. DO NOT REMOVE ANY ELEMENT!
 */
#define dl_list_for_each(elem, head)	\
	for((elem) = (head)->next; (elem) != (head); (elem) = (elem)->next)

/*
 * Loop over each element in a list in a manner that is safe for removing the current element.
 * DO NOT remove any element but the current.
 */
#define dl_list_for_each_safe(elem, next_save, head) \
	for((elem) = (head)->next, (next_save) = (elem)->next; (elem) != head; \
		(elem) = (next_save), (next_save) = (elem)->next)

#endif /* DOUBLY_LINKED_LIST_H_ */
