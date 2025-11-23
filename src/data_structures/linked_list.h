#pragma once
#include <stdlib.h>

typedef struct LinkedListNode {
	void* value;
	struct LinkedListNode* next;
} LinkedListNode;

typedef struct {
	LinkedListNode* head;
	LinkedListNode* tail;
	size_t count;
} LinkedList;

void linked_list_new(LinkedList* this);
void linked_list_delete(LinkedList* this);
void linked_list_push_back(LinkedList* this, void* value);
void linked_list_remove_at(LinkedList* this, size_t index);
void linked_list_remove_after(LinkedList* this, LinkedListNode* node);
LinkedListNode* linked_list_at(LinkedList* this, size_t index);
void* linked_list_value_at(LinkedList* this, size_t index);
