#include "hyperbola.h"
#include "shape_types.h"
#include <limits.h>
#include <assert.h>
#include <math.h>

ShapeVTable hyperbola_default_vtable = {
	.on_draw_properties_window = hyperbola_on_draw_properties_window,
	.on_plot = hyperbola_on_plot,
	.serialize = hyperbola_serialize,
	.delete = shape_delete
};

bool hyperbola_new(Hyperbola* this, const char* initial_label, ImVec4 initial_color, ImPlotPoint center, ImPlotPoint stretch) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &hyperbola_default_vtable;

	this->center = center;
	this->stretch = stretch;
	return true;
}

void hyperbola_on_draw_properties_window(void* _this) {
	Hyperbola* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Hyperbola");
	igPopFont();

	igDragScalarN("Center", ImGuiDataType_Double, &this->center, 2, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);

	ImPlotPoint stretch = this->stretch;
	igDragScalarN("Stretch", ImGuiDataType_Double, &stretch, 2, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);
	bool used_text_input = igGetIO_Nil()->WantTextInput;

	bool x_valid = fabs(stretch.x) >= 0.001;
	bool y_valid = fabs(stretch.y) >= 0.001;
	if (x_valid && y_valid) {
		this->stretch = stretch;
	}
	else if (!used_text_input) {
		if (!x_valid) {
			if (this->stretch.x > stretch.x) this->stretch.x = stretch.x - 0.002f;
			else this->stretch.x = stretch.x + 0.002f;
		}

		if (!y_valid) {
			if (this->stretch.y > stretch.y) this->stretch.y = stretch.y - 0.002f;
			else this->stretch.y = stretch.y + 0.002f;
		}
	}

	igSeparator();
	igPushFont(NULL, 22.0f);
	igText("Equation");
	igPopFont();

	igText("(y - %.3f)^2 / %.3f^2 - (x - %.3f)^2 / %.3f^2 = 1", this->center.y, this->stretch.y, this->center.x, this->stretch.x);
}

void _hyperbola(Hyperbola* this, double x, double* out_y1, double* out_y2) {
	double h = this->center.x;
	double k = this->center.y;
	double a = this->stretch.x;
	double b = this->stretch.y;
	// (y - k)^2 / b^2 - (x - h)^2 / a^2 = 1 rearranged
	double sqrt_term = (x - h) * (x - h) / (a * a) - 1.0;
	if (sqrt_term < 0.001) { // try to avoid NaN
		*out_y1 = k;
		*out_y2 = k;
	}
	else {
		*out_y1 = k + b * sqrt(sqrt_term);
		*out_y2 = k - b * sqrt(sqrt_term);
	}
}

bool _hyperbola_value_fn(void* _this, double x, Buffer* out_values) {
	Hyperbola* this = _this;

	if (fabs(x - this->center.x) < fabs(this->stretch.x)) return true; // no real values

	double y1, y2;
	_hyperbola(this, x, &y1, &y2);

	if (!buffer_push_back(out_values, &y1, sizeof(y1))) return false;
	if (!buffer_push_back(out_values, &y2, sizeof(y2))) return false;

	return true;
}

void hyperbola_on_plot(void* _this, bool is_selected) {
	Hyperbola* this = _this;

	if (is_selected) {
		ImPlot_DragPoint(0, &this->center.x, &this->center.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		shape_draw_point_tooltip((Shape*)this, _hyperbola_value_fn);
	}

	ImPlotRect limits = ImPlot_GetPlotLimits(-1, -1);
	double resolution = 500.0;
	double step = (limits.X.Max - limits.X.Min) / resolution;

	// right branch
	Buffer right_upper_points;
	if (!buffer_new(&right_upper_points, 0)) return;
	Buffer right_lower_points;
	if (!buffer_new(&right_lower_points, 0)) goto fail_right_lower_points;

	for (double x = this->center.x + fabs(this->stretch.x); x <= limits.X.Max; x += step) {
		double y1, y2;
		_hyperbola(this, x, &y1, &y2);

		ImVec2 p1 = ImPlot_PlotToPixels_double(x, y1, -1, -1);
		ImVec2 p2 = ImPlot_PlotToPixels_double(x, y2, -1, -1);
		buffer_push_back(&right_upper_points, &p1, sizeof(p1));
		buffer_push_back(&right_lower_points, &p2, sizeof(p2));
	}

	// left branch
	Buffer left_upper_points;
	if (!buffer_new(&left_upper_points, 0)) goto fail_left_upper_points;
	Buffer left_lower_points;
	if (!buffer_new(&left_lower_points, 0)) goto fail_left_lower_points;

	for (double x = this->center.x - fabs(this->stretch.x); x >= limits.X.Min; x -= step) {
		double y1, y2;
		_hyperbola(this, x, &y1, &y2);

		ImVec2 p1 = ImPlot_PlotToPixels_double(x, y1, -1, -1);
		ImVec2 p2 = ImPlot_PlotToPixels_double(x, y2, -1, -1);
		buffer_push_back(&left_upper_points, &p1, sizeof(p1));
		buffer_push_back(&left_lower_points, &p2, sizeof(p2));
	}

	ImVec2* right_lower_points_array = (ImVec2*)right_lower_points.data;
	size_t right_lower_points_count = right_lower_points.size / sizeof(ImVec2);
	assert(right_lower_points_count < INT_MAX);

	ImVec2* right_upper_points_array = (ImVec2*)right_upper_points.data;
	size_t right_upper_points_count = right_upper_points.size / sizeof(ImVec2);
	assert(right_upper_points_count < INT_MAX);

	ImVec2* left_lower_points_array = (ImVec2*)left_lower_points.data;
	size_t left_lower_points_count = left_lower_points.size / sizeof(ImVec2);
	assert(left_lower_points_count < INT_MAX);

	ImVec2* left_upper_points_array = (ImVec2*)left_upper_points.data;
	size_t left_upper_points_count = left_upper_points.size / sizeof(ImVec2);
	assert(left_upper_points_count < INT_MAX);

	// draw approximation lines
	ImDrawList_AddPolyline(ImPlot_GetPlotDrawList(),
		right_lower_points_array,
		(int)right_lower_points_count,
		igGetColorU32_Vec4(this->base.color),
		false,
		2.0f
	);

	ImDrawList_AddPolyline(ImPlot_GetPlotDrawList(),
		right_upper_points_array,
		(int)right_upper_points_count,
		igGetColorU32_Vec4(this->base.color),
		false,
		2.0f
	);

	ImDrawList_AddPolyline(ImPlot_GetPlotDrawList(),
		left_lower_points_array,
		(int)left_lower_points_count,
		igGetColorU32_Vec4(this->base.color),
		false,
		2.0f
	);

	ImDrawList_AddPolyline(ImPlot_GetPlotDrawList(),
		left_upper_points_array,
		(int)left_upper_points_count,
		igGetColorU32_Vec4(this->base.color),
		false,
		2.0f
	);

	buffer_delete(&left_lower_points);
fail_left_lower_points:
	buffer_delete(&left_upper_points);
fail_left_upper_points:
	buffer_delete(&right_lower_points);
fail_right_lower_points:
	buffer_delete(&right_upper_points);
}

#pragma pack(push, 1)
typedef struct {
	ImPlotPoint center;
	ImPlotPoint stretch;
} SerializedHyperbola;
#pragma pack(pop)

bool hyperbola_serialize(void* _this, Buffer* out_data) {
	Hyperbola* this = _this;


	uint8_t shape_type = (uint8_t)ShapeType_Hyperbola;
	buffer_push_back(out_data, &shape_type, sizeof(shape_type));

	Buffer base;
	if (!buffer_new(&base, 0)) return false;
	if (!shape_serialize(this, &base)) goto fail;
	if (!buffer_reserve(out_data, sizeof(shape_type) + base.size + sizeof(SerializedHyperbola))) goto fail;

	if (!buffer_push_back(out_data, base.data, base.size)) goto fail;
	buffer_delete(&base);

	SerializedHyperbola parabola_data = {
		.center = this->center,
		.stretch = this->stretch
	};

	return buffer_push_back(out_data, &parabola_data, sizeof(parabola_data));

fail:
	buffer_delete(&base);
	return false;

}

bool hyperbola_deserialize(Hyperbola* this, Buffer* in_data) {
	if (!shape_deserialize(&this->base, in_data)) return false;
	this->base.vtable = &hyperbola_default_vtable;

	SerializedHyperbola hyperbola_data;
	if (!buffer_consume(in_data, sizeof(SerializedHyperbola), &hyperbola_data)) return false;

	this->center = hyperbola_data.center;
	this->stretch = hyperbola_data.stretch;

	return true;

}
