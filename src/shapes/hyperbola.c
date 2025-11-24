#include "hyperbola.h"

ShapeVTable hyperbola_default_vtable = {
	.on_draw_properties_window = hyperbola_on_draw_properties_window,
	.on_plot = hyperbola_on_plot,
	.delete = shape_delete
};

bool hyperbola_new(Hyperbola* this, const char* initial_label, ImVec4 initial_color, DVec2 params) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &hyperbola_default_vtable;

	this->params = params;
	return true;
}

void hyperbola_on_draw_properties_window(void* _this) {
	Hyperbola* this = _this;


	igPushFont(NULL, 22.0f);
	igText("Hyperbola");
	igPopFont();
}

void hyperbola_on_plot(void* _this, bool is_selected) {
	Hyperbola* this = _this;

}
