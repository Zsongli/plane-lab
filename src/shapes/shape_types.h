#pragma once
#include <stddef.h>
#include "shape.h"

typedef enum {
	ShapeType_Line = 0,
	ShapeType_Circle,
	ShapeType_Parabola,
	ShapeType_Hyperbola,
	ShapeType_Count
} ShapeType;

const char* shape_type_to_string(ShapeType shape_type);
bool shape_new_default_from_type(ShapeType shape_type, Buffer* out_data);
bool shape_deserialize_from_file_entry(Buffer* in_data, Buffer* out_data);
