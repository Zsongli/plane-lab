#pragma once
#include <stdbool.h>
#include <stddef.h>
#include "dynamic_buffer.h"

typedef Buffer String;

bool string_new(String* this, char* value);
void string_delete(String* this);

bool string_set(String* this, char* value);
bool string_reserve(String* this, size_t new_capacity);
bool string_append(String* this, char* value);
