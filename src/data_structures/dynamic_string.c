#include "dynamic_string.h"
#include <string.h>
#include <stdlib.h>
#include <debugmalloc.h>

bool string_new(String* this, const char* value) {
	size_t len = strlen(value);
	this->capacity =  len + 1;

	this->data = strdup(value);
	if (!this->data) return false;

	return true;
}

void string_delete(String* this) {
	free(this->data);
}

bool string_set(String* this, const char* value) {
	size_t len = strlen(value);
	
	if (len + 1 > this->capacity) {
		char* new_data = realloc(this->data, len + 1);
		if (!new_data) return false;
		this->data = new_data;
		this->capacity = len + 1;
	}
	
	strcpy_s(this->data, this->capacity, value);
	
	return true;
}

bool string_resize(String* this, size_t new_capacity) {
	if (new_capacity <= this->capacity) return true;
	
	char* new_data = realloc(this->data, new_capacity);
	if (!new_data) return false;
	
	this->data = new_data;
	this->capacity = new_capacity;
	
	return true;
}
