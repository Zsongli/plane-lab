#include "dynamic_buffer.h"
#include <stdlib.h>
#include <debugmalloc.h>

bool buffer_new(Buffer* this, size_t initial_capacity) {

	this->data = malloc(max(initial_capacity, 1));
	if (!this->data) return false;

	this->capacity = initial_capacity;
	this->size = 0;
	return true;
}

void buffer_delete(Buffer* this) {
	free(this->data);
}

bool buffer_reserve(Buffer* this, size_t new_capacity) {
	if (new_capacity <= this->capacity) return true;

	void* new_data = realloc(this->data, new_capacity);
	if (!new_data) return false;

	this->data = new_data;

	this->capacity = new_capacity;
	return true;
}

static const int GROWTH_FACTOR = 2;

bool buffer_push_back(Buffer* this, const void* data, size_t data_size) {
	if (this->size + data_size > this->capacity) {
		size_t new_capacity = max(this->capacity, 1) * GROWTH_FACTOR;
		while (new_capacity < this->size + data_size) new_capacity *= GROWTH_FACTOR;

		if (!buffer_reserve(this, new_capacity)) return false;
	}

	memcpy((uint8_t*)this->data + this->size, data, data_size);
	this->size += data_size;
	return true;
}

bool buffer_copy(Buffer* dest, const Buffer* src) {
	if (!buffer_reserve(dest, src->size)) return false;
	memcpy(dest->data, src->data, src->size);
	dest->size = src->size;
	return true;
}

bool buffer_consume(Buffer* this, size_t data_size, void* out_data) {
	if (data_size > this->size) return false;
	memcpy(out_data, this->data, data_size);
	memmove(this->data, (uint8_t*)this->data + data_size, this->size - data_size);
	this->size -= data_size;
	return true;
}
