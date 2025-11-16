#include "linked_list.h"
#include <assert.h>

void linked_list_new(LinkedList* this) {
	this->head = NULL;
	this->tail = NULL;
	this->count = 0;
}

void linked_list_delete(LinkedList* this) {
	LinkedListNode* iter = this->head;
	while (iter) {
		LinkedListNode* next = iter->next;
		free(iter);
		iter = next;
	}
}

void linked_list_push_back(LinkedList* this, void* value) {
	LinkedListNode* node = (LinkedListNode*)malloc(sizeof(LinkedListNode));
	if (!node) return;

	node->value = value;
	node->next = NULL;

	if (this->tail) this->tail->next = node;
	else this->head = node;
	this->tail = node;
	this->count++;
}

void linked_list_remove_at(LinkedList* this, size_t index) {
	if (index == 0) {
		LinkedListNode* to_remove = this->head;
		this->head = to_remove->next;
		if (this->tail == to_remove) this->tail = NULL;
		free(to_remove);
	}
	else {
		LinkedListNode* prev = linked_list_at(this, index - 1);
		assert(prev != NULL && "attempted to remove element in empty list");

		LinkedListNode* to_remove = prev->next;
		assert(to_remove != NULL && "attempted to remove element out of bounds");

		prev->next = to_remove->next;
		if (this->tail == to_remove) this->tail = prev;
		free(to_remove);
	}
	this->count--;
}

LinkedListNode* linked_list_at(LinkedList* this, size_t index) {
	LinkedListNode* iter = this->head;
	for (size_t i = 0; i < index; i++) {
		if (!iter) return NULL;
		iter = iter->next;
	}
	return iter;
}

void* linked_list_value_at(LinkedList* this, size_t index)
{
	return linked_list_at(this, index)->value;
}
