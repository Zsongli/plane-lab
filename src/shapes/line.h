#pragma once
#include "shape.h"
#include <cimplot.h>

typedef struct {
	Shape base;
	ImPlotPoint p1, p2;
} Line;

bool line_new(Line* this, char* initial_label, ImVec4 initial_color, ImPlotPoint p1, ImPlotPoint p2);
void line_on_draw_properties_window(void* this);
void line_on_plot(void* this, bool is_selected);
bool line_serialize(void* this, Buffer* out_data);
bool line_deserialize(Line* this, Buffer* in_data);
