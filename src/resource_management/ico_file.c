#include "ico_file.h"
#include <stdlib.h>
#include <string.h>
#include <debugmalloc.h>

typedef struct {
	uint16_t _pad;
	uint16_t image_type;
	uint16_t image_count;
} IcoHeader;

IcoEntry* ico_file_get_entries(const void* ico_file_data, size_t* out_count) {
	const IcoHeader* header = ico_file_data;

	*out_count = header->image_count;
	IcoEntry* entries_copy = calloc(sizeof(IcoEntry), header->image_count);
	if (!entries_copy) return NULL;

	IcoEntry* entries_begin = (IcoEntry*)((uint8_t*)ico_file_data + sizeof(IcoHeader));

	memcpy(entries_copy, entries_begin, sizeof(IcoEntry) * header->image_count);
	return entries_copy;
}

bool ico_entry_is_png(IcoEntry entry, const void* ico_file_data) {
	const uint8_t png_signature[] = "\x89PNG\r\n\x1a\n";
	const void* entry_begin = (uint8_t*)ico_file_data + entry.offset;
	return strncmp(entry_begin, png_signature, sizeof(png_signature) - 1) == 0;
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



// returns a copy of the bmp data with a proper bmp header, also changes the width and height in the dib header to be correct
void* ico_entry_bmp_to_real_bmp(IcoEntry entry, const void* ico_file_data, size_t* out_size) {
	const void* entry_begin = (uint8_t*)ico_file_data + entry.offset;
	const uint32_t bmp_size = entry.size + sizeof(BmpHeader);

	*out_size = bmp_size;
	uint8_t* bmp_data = malloc(bmp_size);

	if (!bmp_data) return NULL;

	const SmallDib* dib = entry_begin; // the bmp data starts with the dib header, which starts with its size
	*(BmpHeader*)bmp_data = (BmpHeader){
		.header_field = {'B', 'M'},
		.file_size = bmp_size,
		._pad = 0,
		.data_offset = sizeof(BmpHeader) + dib->header_size // the pixel data starts after the dib header (only usually, but this is fine for ico files i think)
	};

	memcpy(bmp_data + sizeof(BmpHeader), entry_begin, entry.size);

	// fix width and height in dib header
	SmallDib* bmp_data_dib = (SmallDib*)(bmp_data + sizeof(BmpHeader));
	if (bmp_data_dib->header_size == 12) {
		bmp_data_dib->width = entry.width;
		bmp_data_dib->height = entry.height;
	}
	else {
		LargeDib* large_dib = (LargeDib*)bmp_data_dib;
		large_dib->width = entry.width;
		large_dib->height = entry.height;
	}

	return bmp_data;
}
