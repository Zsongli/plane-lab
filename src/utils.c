#include "utils.h"
#include <string.h>

const char* filename(const char* path) {
	char* last_slash = strrchr(path, '/');
	char* last_backslash = strrchr(path, '\\');
	if (last_slash) return last_slash + 1;
	if (last_backslash) return last_backslash + 1;
	return path;
}
