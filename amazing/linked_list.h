#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "common.h"

typedef struct ListNode {
	CourseRecord data;
	struct ListNode *prev;
	struct ListNode *next;
} ListNode;

typedef struct {
	ListNode *head;
	ListNode *tail;
	int count;
} LinkedList;

LinkedList* linked_list_create();
DSInterface linked_list_get_interface();

#endif
