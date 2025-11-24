#pragma once
#include "shape.h"

typedef struct {
	Shape base;
	DVec2 center;
	double radius;
	double validated_radius;
} Circle;

bool circle_new(Circle* this, const char* initial_label, ImVec4 initial_color, DVec2 center, double radius);
void circle_on_draw_properties_window(void* this);
void circle_on_plot(void* this, bool is_selected);
