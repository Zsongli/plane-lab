#include "line.h"
#include <float.h>
#include <math.h>
#include <cimplot.h>

void line_on_draw_properties_window(void* _this) {
	Line* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Line");
	igPopFont();

	this->p1 = this->validated_p1;
	this->p2 = this->validated_p2;

	bool p1_changed = igDragScalarN("Point 1", ImGuiDataType_Double, &this->p1, 2, 0.001f, NULL, NULL, "%.3lf", ImGuiSliderFlags_None);
	bool p2_changed = igDragScalarN("Point 2", ImGuiDataType_Double, &this->p2, 2, 0.001f, NULL, NULL, "%.3lf", ImGuiSliderFlags_None);

	bool used_text_input = igGetIO_Nil()->WantTextInput;

	// validation and correction to prevent vertical lines
	bool valid = fabs(this->p1.x - this->p2.x) > 0.001;
	if (valid) {
		this->validated_p1 = this->p1;
		this->validated_p2 = this->p2;
	}
	else if (!used_text_input) {
		if (p1_changed) {
			if (this->validated_p1.x > this->p1.x) this->validated_p1.x = this->p1.x - 0.002f;
			else this->validated_p1.x = this->p1.x + 0.002f;
		}
		if (p2_changed) {
			if (this->validated_p2.x > this->p2.x) this->validated_p2.x = this->p2.x - 0.002f;
			else this->validated_p2.x = this->p2.x + 0.002f;
		}
	}

	igSeparator();

	igPushFont(NULL, 22.0f);
	igText("Equation");
	igPopFont();

	double slope = (this->validated_p2.y - this->validated_p1.y) / (this->validated_p2.x - this->validated_p1.x);
	double intercept = this->validated_p1.y - slope * this->validated_p1.x;

	if (intercept < 0.0) igText("y = %.2fx - %.2f", slope, -intercept);
	else igText("y = %.2fx + %.2f", slope, intercept);
}

double _line(Line* this, double x) {
	return this->validated_p1.y + (this->validated_p2.y - this->validated_p1.y) * ((x - this->validated_p1.x) / (this->validated_p2.x - this->validated_p1.x));
}

void line_on_plot(void* _this, bool is_selected) {
	Line* this = _this;

	ImPlotRect limits = ImPlot_GetPlotLimits(-1, -1);

	if (is_selected) {
		ImPlot_DragPoint(0, &this->p1.x, &this->p1.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		ImPlot_DragPoint(1, &this->p2.x, &this->p2.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);

		// validation to prevent vertical lines
		bool valid = fabs(this->p1.x - this->p2.x) > 0.001;
		if (valid) {
			this->validated_p1 = this->p1;
			this->validated_p2 = this->p2;
		}
	}

	double xs[2] = { limits.X.Min, limits.X.Max };
	double ys[2] = { _line(this, xs[0]), _line(this, xs[1]) };

	ImPlot_SetNextLineStyle(this->base.color, 2.0f);
	ImPlot_PlotLine_doublePtrdoublePtr(this->base.validated_label.data, xs, ys, 2, ImPlotLineFlags_None, 0, sizeof(double));
}

ShapeVTable line_default_vtable = {
	.on_draw_properties_window = line_on_draw_properties_window,
	.on_plot = line_on_plot,
	.delete = shape_delete
};

bool line_new(Line* this, const char* initial_label, ImVec4 initial_color, DVec2 initial_p1, DVec2 initial_p2) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &line_default_vtable;
	this->p1 = initial_p1;
	this->p2 = initial_p2;
	this->validated_p1 = initial_p1;
	this->validated_p2 = initial_p2;

	return true;
}
