#pragma once
#include "shape.h"

typedef struct {
	Shape base;
	DVec2 p1, p2;
	DVec2 validated_p1, validated_p2;
} Line;

bool line_new(Line* this, const char* initial_label, ImVec4 color, DVec2 p1, DVec2 p2);

void line_delete(Line* this);
void line_on_draw_properties_window(void* this);
void line_on_plot(void* this, bool is_selected);
