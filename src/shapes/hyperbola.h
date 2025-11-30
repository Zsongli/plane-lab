#pragma once
#include "shape.h"
#include <cimplot.h>

typedef struct {
	Shape base;
	ImPlotPoint center;
	ImPlotPoint stretch;
} Hyperbola;

bool hyperbola_new(Hyperbola* this, char* initial_label, ImVec4 initial_color, ImPlotPoint center, ImPlotPoint stretch);
void hyperbola_on_draw_properties_window(void* this);
void hyperbola_on_plot(void* this, bool is_selected);
bool hyperbola_serialize(void* this, Buffer* out_data);
bool hyperbola_deserialize(Hyperbola* this, Buffer* in_data);
