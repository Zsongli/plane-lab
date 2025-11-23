#pragma once
#include <cimgui.h>
#include "../data_structures/dynamic_string.h"

typedef struct {
	void(*on_plot)(void* this, bool is_selected);
	void(*on_draw_properties_window)(void* this);
	void(*delete)(void* this);
} ShapeVTable;

typedef struct {
	ShapeVTable* vtable;
	String label;
	String validated_label;
	ImVec4 color;
} Shape;

typedef struct {
	double x, y;
} DVec2;

bool shape_new(Shape* this, const char* label, ImVec4 color);
void shape_delete(Shape* this);
void shape_draw_properties_window(Shape* this);
void shape_on_draw_properties_window(void* this);
void shape_plot(Shape* this, bool is_selected);
void shape_on_plot(void* this, bool is_selected);
