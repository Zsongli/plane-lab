#include "line.h"
#include <float.h>
#include <math.h>
#include <cimplot.h>
#include "shape_types.h"
#include <stdlib.h>
#include <string.h>
#include <debugmalloc.h>

void line_on_draw_properties_window(void* _this) {
	Line* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Line");
	igPopFont();

	ImPlotPoint p1 = this->p1, p2 = this->p2;

	bool p1_changed = igDragScalarN("Point 1", ImGuiDataType_Double, &p1, 2, 0.001f, NULL, NULL, "%.3lf", ImGuiSliderFlags_None);
	bool p2_changed = igDragScalarN("Point 2", ImGuiDataType_Double, &p2, 2, 0.001f, NULL, NULL, "%.3lf", ImGuiSliderFlags_None);

	bool used_text_input = igGetIO_Nil()->WantTextInput;

	// validation and correction to prevent vertical lines
	bool valid = fabs(p1.x - p2.x) >= 0.001;
	if (valid) {
		this->p1 = p1;
		this->p2 = p2;
	}
	else if (!used_text_input) {
		if (p1_changed) {
			if (this->p1.x > p1.x) this->p1.x = p1.x - 0.002f;
			else this->p1.x = p1.x + 0.002f;
		}
		if (p2_changed) {
			if (this->p2.x > p2.x) this->p2.x = p2.x - 0.002f;
			else this->p2.x = p2.x + 0.002f;
		}
	}

	igSeparator();

	igPushFont(NULL, 22.0f);
	igText("Equation");
	igPopFont();

	double slope = (this->p2.y - this->p1.y) / (this->p2.x - this->p1.x);
	double intercept = this->p1.y - slope * this->p1.x;

	if (intercept < 0.0) igText("y = %.3fx - %.3f", slope, -intercept);
	else igText("y = %.3fx + %.3f", slope, intercept);
}

double _line(void* _this, double x) {
	Line* this = _this;
	return this->p1.y + (this->p2.y - this->p1.y) * ((x - this->p1.x) / (this->p2.x - this->p1.x));
}

bool _line_value_fn(void* _this, double x, Buffer* out_values) {
	Line* this = _this;
	double y = _line(this, x);
	return buffer_push_back(out_values, &y, sizeof(y));
}

void line_on_plot(void* _this, bool is_selected) {
	Line* this = _this;

	ImPlotRect limits = ImPlot_GetPlotLimits(-1, -1);

	if (is_selected) {
		ImPlotPoint p1 = this->p1, p2 = this->p2;
		ImPlot_DragPoint(0, &p1.x, &p1.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		ImPlot_DragPoint(1, &p2.x, &p2.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);

		// validation to prevent vertical lines
		bool valid = fabs(p1.x - p2.x) > 0.001;
		if (valid) {
			this->p1 = p1;
			this->p2 = p2;
		}

		shape_draw_point_tooltip((Shape*)this, _line_value_fn);
	}

	double xs[2] = { limits.X.Min, limits.X.Max };
	double ys[2] = { _line(this, xs[0]), _line(this, xs[1]) };

	ImPlot_SetNextLineStyle(this->base.color, 2.0f);
	ImPlot_PlotLine_doublePtrdoublePtr(this->base.validated_label.data, xs, ys, 2, ImPlotLineFlags_None, 0, sizeof(double));
}


#pragma pack(push, 1)
typedef struct {
	ImPlotPoint p1, p2;
} SerializedLine;
#pragma pack(pop)

bool line_serialize(void* _this, Buffer* out_data) {
	Line* this = _this;

	uint8_t shape_type = (uint8_t)ShapeType_Line;
	buffer_push_back(out_data, &shape_type, sizeof(shape_type));

	Buffer base;
	if (!buffer_new(&base, 0)) return false;
	if (!shape_serialize(this, &base)) goto fail;
	if (!buffer_reserve(out_data, sizeof(shape_type) + base.size + sizeof(SerializedLine))) goto fail;

	if (!buffer_push_back(out_data, base.data, base.size)) goto fail;
	buffer_delete(&base);

	SerializedLine line_data = {
		.p1 = this->p1,
		.p2 = this->p2
	};

	return buffer_push_back(out_data, &line_data, sizeof(line_data));

fail:
	buffer_delete(&base);
	return false;

}

ShapeVTable line_default_vtable = {
	.on_draw_properties_window = line_on_draw_properties_window,
	.on_plot = line_on_plot,
	.delete = shape_delete,
	.serialize = line_serialize
};

bool line_deserialize(Line* this, Buffer* in_data) {
	if (!shape_deserialize(&this->base, in_data)) return false;
	this->base.vtable = &line_default_vtable;

	SerializedLine line_data;
	if (!buffer_consume(in_data, sizeof(SerializedLine), &line_data)) return false;

	this->p1 = line_data.p1;
	this->p2 = line_data.p2;

	return true;
}

bool line_new(Line* this, char* initial_label, ImVec4 initial_color, ImPlotPoint p1, ImPlotPoint p2) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &line_default_vtable;
	this->p1 = p1;
	this->p2 = p2;

	return true;
}
