#include "ico_file.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	unsigned short _pad;
	unsigned short image_type;
	unsigned short image_count;
} IcoHeader;

IcoEntry* ico_file_get_entries(unsigned char* data, int* out_count) {
	IcoHeader* header = (IcoHeader*)data;

	*out_count = header->image_count;
	IcoEntry* entries_copy = calloc(sizeof(IcoEntry), header->image_count);
	if (!entries_copy) return NULL;

	IcoEntry* entries = data + sizeof(IcoHeader);

	memcpy(entries_copy, entries, sizeof(IcoEntry) * header->image_count);
	return entries_copy;
}