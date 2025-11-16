#include <stdbool.h>

#define CONCAT_IMPL(x, y) x##y
#define CONCAT( x, y ) CONCAT_IMPL(x,y)

#define SAFE_INVOKE(fn, ...) \
	do { \
		if (fn) { \
			fn(__VA_ARGS__); \
		} \
	} while (0)
	

#define DO_ONCE_BEGIN() _DO_ONCE_BEGIN(CONCAT(_do_once_flag_, __COUNTER__))
#define _DO_ONCE_BEGIN(varname) static bool varname = true; if(varname) { varname = false;
#define DO_ONCE_END() }

#define DISCARD(x) ((void)(x))
