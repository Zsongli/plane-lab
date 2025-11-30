#pragma once
#include "shape.h"
#include <cimplot.h>

typedef struct {
	Shape base;
	ImPlotPoint vertex;
	double scalar;
} Parabola;

bool parabola_new(Parabola* this, const char* initial_label, ImVec4 initial_color, ImPlotPoint vertex, double scalar);
void parabola_on_draw_properties_window(void* this);
void parabola_on_plot(void* this, bool is_selected);
bool parabola_serialize(void* this, Buffer* out_data);
bool parabola_deserialize(Parabola* this, Buffer* in_data);
