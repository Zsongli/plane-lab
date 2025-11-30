#include "dynamic_string.h"
#include <string.h>
#include <stdlib.h>
#include <debugmalloc.h>

bool string_new(String* this, char* value) {
	if (!value) {
		if (!buffer_new(this, 0)) return false;
		return true;
	}

	size_t len = strlen(value);
	
	if (!buffer_new(this, len + 1)) return false;
	if (!buffer_push_back(this, value, len + 1)) return false;
	
	return true;
}

void string_delete(String* this) {
	buffer_delete(this);
}

bool string_set(String* this, char* value) {
	size_t len = strlen(value);

	if(!buffer_reserve(this, len + 1)) return false;

	if (strcpy_s(this->data, this->capacity, value) != 0) return false;
	this->size = len + 1;
	return true;
}

bool string_reserve(String* this, size_t new_capacity) {
	return buffer_reserve(this, new_capacity);
}

bool string_append(String* this, char* value) {
	size_t len = strlen(value);
	size_t new_size = this->size + len;
	if (!buffer_reserve(this, new_size)) return false;

	if (strcpy_s((char*)this->data + this->size - 1, this->capacity - (this->size - 1), value) != 0) return false;
	this->size = new_size;

	return true;
}

