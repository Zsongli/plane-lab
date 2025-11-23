#include "shape.h"
#include "../utils.h"
#include <string.h>
#include <stdlib.h>
#include <debugmalloc.h>
#include <cimplot.h>
#include <imgui_macros.h>

int label_resize_callback(ImGuiInputTextCallbackData* data) {
	if (data->EventFlag != ImGuiInputTextFlags_CallbackResize) return 0;

	String* label = data->UserData;
	if(!string_resize(label, data->BufSize)) return 1;
	data->Buf = label->data;

	return 0;
}

void shape_on_draw_properties_window(void* _this) {
	DISCARD(_this);
}

void shape_draw_properties_window(Shape* this) {
	igPushFont(NULL, 22.0f);
	igText("Base shape");
	igPopFont();

	if (igInputText("Label", this->label.data, this->label.capacity, ImGuiInputTextFlags_CallbackResize, label_resize_callback, &this->label)) {
		bool valid = strlen(this->label.data) > 0;
		if (valid) {
			string_set(&this->validated_label, this->label.data);
		}
		else {
			string_set(&this->label, this->validated_label.data);
		}
	}
	igColorEdit4("Color", &this->color.x, ImGuiColorEditFlags_None);

	igSeparator();

	this->vtable->on_draw_properties_window(this);
}

void shape_plot(Shape* this, bool is_selected) {
	if (ImPlot_BeginItem(this->validated_label.data, ImPlotItemFlags_None, -1)) {
		this->vtable->on_plot(this, is_selected);
		ImPlot_EndItem();
	}
}

void shape_on_plot(void* _this, bool is_selected) {
	DISCARD(_this);
	DISCARD(is_selected);
}

void shape_delete(Shape* this) {
	string_delete(&this->validated_label);
	string_delete(&this->label);
}

ShapeVTable shape_default_vtable = {
	.on_draw_properties_window = shape_on_draw_properties_window,
	.on_plot = shape_on_plot,
	.delete = shape_delete,
};

bool shape_new(Shape* this, const char* initial_label, ImVec4 color) {
	this->vtable = &shape_default_vtable;
	this->color = color;
	
	if(!string_new(&this->label, initial_label)) return false;
	if (!string_new(&this->validated_label, initial_label)) {
		string_delete(&this->label);
		return false;
	}
	
	return true;
}
