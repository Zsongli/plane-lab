#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct {
	char* data;
	size_t capacity;
} String;

bool string_new(String* this, const char* value);
void string_delete(String* this);

bool string_set(String* this, const char* value);
bool string_resize(String* this, size_t new_capacity);
