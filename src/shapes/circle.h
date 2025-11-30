#pragma once
#include "shape.h"
#include <cimplot.h>

typedef struct {
	Shape base;
	ImPlotPoint center;
	double radius;
	ImPlotPoint radius_tool_pos;

} Circle;

bool circle_new(Circle* this, char* initial_label, ImVec4 initial_color, ImPlotPoint center, double radius);
void circle_on_draw_properties_window(void* this);
void circle_on_plot(void* this, bool is_selected);
bool circle_serialize(void* this, Buffer* out_data);
bool circle_deserialize(Circle* this, Buffer* in_data);
