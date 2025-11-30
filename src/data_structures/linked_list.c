#include "linked_list.h"
#include <assert.h>
#include <debugmalloc.h>

void linked_list_new(LinkedList* this) {
	this->head = NULL;
}

void linked_list_delete(LinkedList* this) {
	LinkedListNode* iter = this->head;
	while (iter) {
		LinkedListNode* next = iter->next;
		free(iter);
		iter = next;
	}
}

bool linked_list_push_back(LinkedList* this, void* value) {
	LinkedListNode* node = (LinkedListNode*)malloc(sizeof(LinkedListNode));
	if (!node) return false;

	*node = (LinkedListNode){
		.value = value,
		.next = NULL
	};

	if (!this->head) {
		this->head = node;
		return true;
	}

	LinkedListNode* tail = this->head;
	while (tail->next) tail = tail->next;

	tail->next = node;
	return true;
}

void linked_list_remove_after(LinkedListNode* node) {
	assert(node != NULL && "give me something"); // https://youtu.be/ASrCKnoViMI?t=18

	LinkedListNode* to_remove = node->next;
	assert(to_remove != NULL && "attempted to remove element out of bounds");

	node->next = to_remove->next;
	free(to_remove);
}

LinkedListNode* linked_list_at(LinkedList* this, size_t index) {
	size_t current_index = 0;
	for (LinkedListNode* iter = this->head; iter != NULL; iter = iter->next) {
		if (current_index == index) return iter;
		current_index++;
	}
	return NULL;
}

void* linked_list_value_at(LinkedList* this, size_t index) {
	return linked_list_at(this, index)->value;
}

size_t linked_list_count(LinkedList* this) {
	size_t length = 0;
	for (LinkedListNode* iter = this->head; iter != NULL; iter = iter->next)
		length++;
	return length;
}

void linked_list_remove_head(LinkedList* this) {
	LinkedListNode* new_head = this->head->next;
	free(this->head);
	this->head = new_head;
}

LinkedListNode* linked_list_find_preceding(LinkedList* this, void* value) {
	LinkedListNode* iter = this->head;
	while (iter->next && iter->next->value != value) iter = iter->next;
	if (!iter->next) return NULL;
	return iter;
}

LinkedListNode* linked_list_find(LinkedList* this, void* value) {
	return linked_list_find_preceding(this, value)->next;
}
