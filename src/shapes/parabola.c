#include "parabola.h"

ShapeVTable parabola_default_vtable = {
	.on_draw_properties_window = parabola_on_draw_properties_window,
	.on_plot = parabola_on_plot,
	.delete = shape_delete
};

bool parabola_new(Parabola* this, const char* initial_label, ImVec4 initial_color, DVec2 vertex, float scalar) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &parabola_default_vtable;
	
	this->vertex = vertex;
	this->scalar = scalar;
	return true;
}

void parabola_on_draw_properties_window(void* _this) {
	Parabola* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Parabola");
	igPopFont();

}

void parabola_on_plot(void* _this, bool is_selected) {
	Parabola* this = _this;

}
