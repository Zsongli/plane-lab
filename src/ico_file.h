#pragma once
#include <stddef.h>

typedef struct {
	unsigned char width;
	unsigned char height;
	unsigned char palette;
	unsigned char _pad;
	unsigned short planes;
	unsigned short bits_per_pixel;
	unsigned int size;
	unsigned int offset;
} IcoEntry;

IcoEntry* ico_file_get_entries(unsigned char* data, int* out_count);
