#pragma once
#include "shape.h"

typedef struct {
	Shape base;
	DVec2 params;
} Hyperbola;

bool hyperbola_new(Hyperbola* this, const char* initial_label, ImVec4 initial_color, DVec2 params);
void hyperbola_on_draw_properties_window(void* this);
void hyperbola_on_plot(void* this, bool is_selected);
