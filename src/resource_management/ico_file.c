#include "ico_file.h"
#include "../data_structures/dynamic_buffer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stb_image.h>
#include <GLFW/glfw3.h>

typedef struct {
	uint16_t _pad;
	uint16_t image_type;
	uint16_t image_count;
} IcoHeader;

bool ico_file_get_entries(const void* ico_file_data, Buffer* out_icon_entries) {
	const IcoHeader* header = ico_file_data;
	IcoEntry* entries_begin = (IcoEntry*)((uint8_t*)ico_file_data + sizeof(IcoHeader));
	return buffer_push_back(out_icon_entries, entries_begin, sizeof(IcoEntry) * header->image_count);
}

static const uint8_t PNG_SIGNATURE[] = "\x89PNG\r\n\x1a\n";
bool ico_entry_is_png(IcoEntry* entry, const void* ico_file_data) {
	const void* entry_begin = (uint8_t*)ico_file_data + entry->offset;
	return strncmp(entry_begin, PNG_SIGNATURE, sizeof(PNG_SIGNATURE) - 1) == 0;
}

#pragma pack(push, 1) // the compiler adds padding by default and screws up stbi's validation
typedef struct {
	uint8_t header_field[2];
	uint32_t file_size;
	uint32_t _pad;
	uint32_t data_offset;
} BmpHeader;
#pragma pack(pop)

typedef struct {
	uint32_t header_size;
	uint16_t width;
	uint16_t height;
	// the rest doesn't matter
} SmallDib;

typedef struct {
	uint32_t header_size;
	int32_t width;
	int32_t height;
	// the rest doesn't matter
} LargeDib;



// writes a copy of the bmp data with a proper bmp header, also changes the width and height in the dib header to be correct
bool ico_entry_bmp_to_real_bmp(IcoEntry* entry, const void* ico_file_data, Buffer* out_bmp_data) {
	const void* entry_begin = (uint8_t*)ico_file_data + entry->offset;
	const uint32_t bmp_size = entry->size + sizeof(BmpHeader);

	if (!buffer_reserve(out_bmp_data, bmp_size)) return false;

	BmpHeader header = {
		.header_field = {'B', 'M'},
		.file_size = bmp_size,
		._pad = 0,
		.data_offset = sizeof(BmpHeader) + ((SmallDib*)entry_begin)->header_size // the pixel data starts after the dib header (only usually, but this is fine for ico files i think)
	};

	if (!buffer_push_back(out_bmp_data, &header, sizeof(BmpHeader))) return false;

	if (!buffer_push_back(out_bmp_data, entry_begin, entry->size)) return false;

	// fix width and height in dib header
	SmallDib* bmp_data_dib = (SmallDib*)((uint8_t*)out_bmp_data->data + sizeof(BmpHeader));
	if (bmp_data_dib->header_size == 12) {
		bmp_data_dib->width = entry->width;
		bmp_data_dib->height = entry->height;
	}
	else {
		LargeDib* large_dib = (LargeDib*)bmp_data_dib;
		large_dib->width = entry->width;
		large_dib->height = entry->height;
	}

	return true;
}

bool ico_file_load_icons(const void* ico_file_data, size_t ico_size, Buffer* out_icon_images) {
	Buffer entries_buffer;
	if (!buffer_new(&entries_buffer, 0)) return false;
	if (!ico_file_get_entries(ico_file_data, &entries_buffer)) goto fail_get_entries;

	IcoEntry* entries = (IcoEntry*)entries_buffer.data;
	const size_t entry_count = entries_buffer.size / sizeof(IcoEntry); // the buffer has items of type IcoEntry

	for (size_t i = 0; i < entry_count; i++) {
		GLFWimage image;
		if (ico_entry_is_png(&entries[i], ico_file_data)) {
			assert(entries[i].size <= INT_MAX);
			// stbi can read pngs directly so just pass the data over
			image.pixels = stbi_load_from_memory(
				(uint8_t*)ico_file_data + entries[i].offset,
				(int)entries[i].size,
				&image.width,
				&image.height,
				NULL,
				4
			);
		}
		else {
			Buffer bmp_data_buffer;
			if (!buffer_new(&bmp_data_buffer, 0)) goto fail_load_icons;

			// convert to a format that stbi can read without complaining
			if (!ico_entry_bmp_to_real_bmp(&entries[i], ico_file_data, &bmp_data_buffer)) {
				buffer_delete(&bmp_data_buffer);
				goto fail_load_icons;
			}

			assert(bmp_data_buffer.size <= INT_MAX);
			image.pixels = stbi_load_from_memory(
				bmp_data_buffer.data,
				(int)bmp_data_buffer.size,
				&image.width,
				&image.height,
				NULL,
				4
			);
			buffer_delete(&bmp_data_buffer);
		}

		if (!image.pixels) goto fail_load_icons;
		if (!buffer_push_back(out_icon_images, &image, sizeof(GLFWimage))) {
			stbi_image_free(image.pixels);
			goto fail_load_icons;
		}
	}

	buffer_delete(&entries_buffer);
	return true;

fail_load_icons:
	GLFWimage* out_icon_images_data = (GLFWimage*)out_icon_images->data;
	const size_t out_icon_count = out_icon_images->size / sizeof(GLFWimage);
	for (size_t i = 0; i < out_icon_count; i++) stbi_image_free(out_icon_images_data[i].pixels);
fail_get_entries:
	buffer_delete(&entries_buffer);
	return false;
}
