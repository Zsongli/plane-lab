#include "shape_types.h"
#include <assert.h>
#include <stdlib.h>
#include "line.h"
#include "circle.h"
#include "parabola.h"
#include "hyperbola.h"
#include <debugmalloc.h>

static char* shape_type_strings[ShapeType_Count] = {
	"Line",
	"Circle",
	"Parabola",
	"Hyperbola"
};

char* shape_type_to_string(ShapeType shape_type) {
	assert(shape_type >= 0 && shape_type < ShapeType_Count);
#pragma warning(suppress : 33010) // msvc doesn't know that assert never returns
	return shape_type_strings[shape_type];
}

const ImVec4 DEFAULT_SHAPE_COLOR = { 0.0f, 0.0f, 0.0f, 1.0f };

const struct {
	ImPlotPoint p1;
	ImPlotPoint p2;
} DEFAULT_LINE = {
	.p1 = { -10.0, -10.0 },
	.p2 = { 10.0, 10.0 }
};

const struct {
	ImPlotPoint center;
	double radius;
} DEFAULT_CIRCLE = {
	.center = { 0.0, 0.0 },
	.radius = 10.0
};

const struct {
	ImPlotPoint vertex;
	double scalar;
} DEFAULT_PARABOLA = {
	.vertex = { 0.0, 0.0 },
	.scalar = 1.0
};

const struct {
	ImPlotPoint center;
	ImPlotPoint stretch;
} DEFAULT_HYPERBOLA = {
	.center = { 0.0, 0.0 },
	.stretch = { 5.0, 3.0 }
};

// templates would be so good here
// caller must free the returned shape
bool shape_new_default_from_type(ShapeType shape_type, Buffer* out_data) {
	assert(shape_type >= 0 && shape_type < ShapeType_Count);
	char* label = shape_type_to_string(shape_type);

	switch (shape_type) {
	case ShapeType_Line: {
		if (!buffer_reserve(out_data, sizeof(Line))) return false;
		if (!line_new(out_data->data, label, DEFAULT_SHAPE_COLOR, DEFAULT_LINE.p1, DEFAULT_LINE.p2)) return false;
		out_data->size = sizeof(Line);
		return true;
	}
	case ShapeType_Circle: {
		if (!buffer_reserve(out_data, sizeof(Circle))) return false;
		if (!circle_new(out_data->data, label, DEFAULT_SHAPE_COLOR, DEFAULT_CIRCLE.center, DEFAULT_CIRCLE.radius)) return false;
		out_data->size = sizeof(Circle);
		return true;
	}
	case ShapeType_Parabola: {
		if (!buffer_reserve(out_data, sizeof(Parabola))) return false;
		if (!parabola_new(out_data->data, label, DEFAULT_SHAPE_COLOR, DEFAULT_PARABOLA.vertex, DEFAULT_PARABOLA.scalar)) return false;
		out_data->size = sizeof(Parabola);
		return true;
	}
	case ShapeType_Hyperbola: {
		if (!buffer_reserve(out_data, sizeof(Hyperbola))) return false;
		if (!hyperbola_new(out_data->data, label, DEFAULT_SHAPE_COLOR, DEFAULT_HYPERBOLA.center, DEFAULT_HYPERBOLA.stretch)) return false;
		out_data->size = sizeof(Hyperbola);
		return true;
	}
	default:
		// shouldn't happen due to the assert above
		return false;
	}
}

bool shape_deserialize_from_file_entry(Buffer* in_data, Buffer* out_data) {
	uint8_t shape_type_byte;
	if (!buffer_consume(in_data, sizeof(uint8_t), &shape_type_byte)) return false;
	if (shape_type_byte >= ShapeType_Count) return false;
	ShapeType shape_type = (ShapeType)shape_type_byte;

	switch (shape_type) {
	case ShapeType_Line: {
		if (!buffer_reserve(out_data, sizeof(Line))) return false;
		if (!line_deserialize(out_data->data, in_data)) return false;
		return true;
	}
	case ShapeType_Circle: {
		if (!buffer_reserve(out_data, sizeof(Circle))) return false;
		if (!circle_deserialize(out_data->data, in_data)) return false;
		return true;
	}
	case ShapeType_Parabola: {
		if (!buffer_reserve(out_data, sizeof(Parabola))) return false;
		if (!parabola_deserialize(out_data->data, in_data)) return false;
		return true;
	}
	case ShapeType_Hyperbola: {
		if (!buffer_reserve(out_data, sizeof(Hyperbola))) return false;
		if (!hyperbola_deserialize(out_data->data, in_data)) return false;
		return true;
	}
	default:
		// shouldn't happen due to the earlier check
		return false;
	}
}
