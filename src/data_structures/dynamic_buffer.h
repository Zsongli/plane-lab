#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    size_t capacity;
    size_t size;
    void* data;
} Buffer;

bool buffer_new(Buffer* this, size_t initial_capacity);
void buffer_delete(Buffer* this);

bool buffer_reserve(Buffer* this, size_t new_capacity);
bool buffer_push_back(Buffer* this, const void* data, size_t data_size);
bool buffer_copy(Buffer* dest, const Buffer* src);
bool buffer_consume(Buffer* this, size_t data_size, void* out_data);
