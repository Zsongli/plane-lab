#include "shape.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>

int label_resize_callback(ImGuiInputTextCallbackData* data) {
	Shape* this = data->UserData;
	this->label = malloc(data->BufSize);
	if (!this->label) return 1;
	strcpy_s(this->label, data->BufSize, data->Buf);
	free(data->Buf);
	data->Buf = this->label;
	return 0;
}

void shape_draw_properties_window(void* _this) {
	Shape* this = _this;
	if (igInputText("Label", this->label, 16, ImGuiInputTextFlags_CallbackResize, label_resize_callback, this)) {
		bool valid = strlen(this->label) > 0;
		if (valid) {
			free(this->validated_label);
			this->validated_label = strdup(this->label);
		}
		else {
			strcpy_s(this->label, 16, this->validated_label);
		}
	}
	igColorEdit4("Color", (float*)&this->color, ImGuiColorEditFlags_None);
}

void shape_plot(void* this, bool is_selected) {
	DISCARD(this);
	DISCARD(is_selected);
	return;
}

void shape_delete(Shape* this) {
	free(this->validated_label);
	free(this->label);
}

ShapeVTable shape_default_vtable = {
	.draw_properties_window = shape_draw_properties_window,
	.plot = shape_plot,
	.delete = shape_delete,
};

bool shape_new(Shape* this, const char* initial_label, ImVec4 color) {
	this->vtable = &shape_default_vtable;
	this->label = strdup(initial_label);
	this->validated_label = strdup(initial_label);
	if (!this->label) return false;
	this->color = color;
	return true;
}
