#pragma once
#include <stdlib.h>
#include <stdbool.h>

typedef struct LinkedListNode {
	void* value;
	struct LinkedListNode* next;
} LinkedListNode;

typedef struct {
	LinkedListNode* head;
} LinkedList;

void linked_list_new(LinkedList* this);
void linked_list_delete(LinkedList* this);
bool linked_list_push_back(LinkedList* this, void* value);
void linked_list_remove_after(LinkedListNode* node);
void linked_list_remove_head(LinkedList* this);
LinkedListNode* linked_list_at(LinkedList* this, size_t index);
void* linked_list_value_at(LinkedList* this, size_t index);
size_t linked_list_count(LinkedList* this);
LinkedListNode* linked_list_find_preceding(LinkedList* this, void* value);
LinkedListNode* linked_list_find(LinkedList* this, void* value);
