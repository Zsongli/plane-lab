#pragma once
#include "shape.h"

typedef struct {
	Shape base;
	DVec2 vertex;
	float scalar;
} Parabola;

bool parabola_new(Parabola* this, const char* initial_label, ImVec4 initial_color, DVec2 vertex, float scalar);
void parabola_on_draw_properties_window(void* this);
void parabola_on_plot(void* this, bool is_selected);
