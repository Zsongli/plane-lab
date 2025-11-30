#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../data_structures/dynamic_buffer.h"

typedef struct {
	uint8_t width;
	uint8_t height;
	uint8_t palette;
	uint8_t _pad;
	uint16_t planes;
	uint16_t bits_per_pixel;
	uint32_t size;
	uint32_t offset;
} IcoEntry;

bool ico_file_get_entries(const void* ico_file_data, Buffer* out_icon_entries);
bool ico_entry_is_png(IcoEntry* entry, const void* ico_file_data);
bool ico_entry_bmp_to_real_bmp(IcoEntry* entry, const void* ico_file_data, Buffer* out_bmp_data);
bool ico_file_load_icons(const void* ico_file_data, size_t ico_size, Buffer* out_icon_images);
