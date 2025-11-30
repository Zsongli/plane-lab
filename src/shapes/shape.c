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
	if (!string_reserve(label, data->BufSize)) return 1;
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
	ImPlot_SetNextErrorBarStyle(this->color, -1, -1); // not sure if there's a better way to set color in the legend, but we don't use error bars for anyway
	if (ImPlot_BeginItem(this->validated_label.data, ImPlotItemFlags_None, ImPlotCol_ErrorBar)) {
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

#pragma pack(push, 1)
typedef struct {
	ImVec4 color;
	uint64_t label_size;
	// char label_data[label_length];
} SerializedBaseShape;
#pragma pack(pop)

bool shape_serialize(void* _this, Buffer* out_data) {
	Shape* this = _this;

	SerializedBaseShape base_shape = {
		.color = this->color,
		.label_size = strlen(this->label.data) + 1
	};

	if (!buffer_reserve(out_data, sizeof(base_shape) + base_shape.label_size)) return false;
	if (!buffer_push_back(out_data, &base_shape, sizeof(base_shape))) return false;
	if (!buffer_push_back(out_data, this->label.data, base_shape.label_size)) return false;

	return true;
}

bool shape_deserialize(Shape* this, Buffer* in_data) {

	SerializedBaseShape base_shape;
	if (!buffer_consume(in_data, sizeof(SerializedBaseShape), &base_shape)) return false;

	char* label = malloc(base_shape.label_size);
	if (!buffer_consume(in_data, base_shape.label_size, label)) {
		free(label);
		return false;
	}

	if (!shape_new(this, label, base_shape.color)) {
		free(label);
		return false;
	}

	free(label);
	return true;
}

ShapeVTable shape_default_vtable = {
	.on_draw_properties_window = shape_on_draw_properties_window,
	.on_plot = shape_on_plot,
	.delete = shape_delete,
	.serialize = shape_serialize
};

bool shape_new(Shape* this, const char* initial_label, ImVec4 color) {
	this->vtable = &shape_default_vtable;
	this->color = color;

	if (!string_new(&this->label, initial_label)) return false;
	if (!string_new(&this->validated_label, initial_label)) {
		string_delete(&this->label);
		return false;
	}

	return true;
}

bool shape_draw_point_tooltip(Shape* this, bool(*value_fn)(void* this, double x, Buffer* out_values)) {
	ImVec2 mouse_pos_screen = igGetMousePos();
	ImPlotPoint mouse_pos_plot = ImPlot_PixelsToPlot_Float(mouse_pos_screen.x, mouse_pos_screen.y, -1, -1);
	ImVec2 plot_pos = ImPlot_GetPlotPos();
	ImVec2 plot_size = ImPlot_GetPlotSize();

	if (mouse_pos_screen.x < plot_pos.x || mouse_pos_screen.x > plot_pos.x + plot_size.x ||
		mouse_pos_screen.y < plot_pos.y || mouse_pos_screen.y > plot_pos.y + plot_size.y) {
		return false;
	}

	Buffer values;
	if (!buffer_new(&values, 0)) return false;
	if (!value_fn(this, mouse_pos_plot.x, &values)) {
		buffer_delete(&values);
		return false;
	}

	double* values_array = values.data;
	size_t value_count = values.size / sizeof(double);

	for (size_t i = 0; i < value_count; i++) {
		double value = values_array[i];
		ImVec2 value_screen = ImPlot_PlotToPixels_double(mouse_pos_plot.x, value, -1, -1);

		if (fabsf(mouse_pos_screen.y - value_screen.y) < 10.0f) {
			if (igBeginTooltip()) {
				igText("(%.3f, %.3f)", mouse_pos_plot.x, value);
				igEndTooltip();

				break; // only show one tooltip at a time
			}
		}
	}
	buffer_delete(&values);

	return true;
}
