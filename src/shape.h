#pragma once
#include <cimgui.h>

typedef struct {
	void(*plot)(void* this, bool is_selected);
	void(*draw_properties_window)(void* this);
	void(*delete)(void* this);
} ShapeVTable;

typedef struct {
	ShapeVTable* vtable;
	char* label;
	char* validated_label;
	ImVec4 color;
} Shape;

typedef struct {
	double x, y;
} DVec2;

bool shape_new(Shape* this, const char* label, ImVec4 color);
void shape_delete(Shape* this);
void shape_draw_properties_window(void* this);
void shape_plot(void* this);
