#pragma once
#include <cimgui.h>
#include "../data_structures/dynamic_string.h"
#include "../data_structures/dynamic_buffer.h"
#include "shape.h"

typedef struct {
	void(*on_plot)(void* this, bool is_selected);
	void(*on_draw_properties_window)(void* this);
	void(*delete)(void* this);
	bool (*serialize)(void* this, Buffer* out_data);
} ShapeVTable;

typedef struct {
	ShapeVTable* vtable;
	String label;
	String validated_label;
	ImVec4 color;
} Shape;

bool shape_new(Shape* this, char* label, ImVec4 color);
void shape_delete(Shape* this);
void shape_draw_properties_window(Shape* this);
void shape_on_draw_properties_window(void* this);
void shape_plot(Shape* this, bool is_selected);
void shape_on_plot(void* this, bool is_selected);
bool shape_serialize(void* this, Buffer* out_data);
bool shape_deserialize(Shape* this, Buffer* in_data);
bool shape_draw_point_tooltip(Shape* this, bool(*value_fn)(void* this, double x, Buffer* out_values));
