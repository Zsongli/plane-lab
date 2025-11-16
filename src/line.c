#include "line.h"
#include <cimplot.h>

void line_draw_properties_window(void* _this) {
	Line* this = _this;
	shape_draw_properties_window(&this->base);
	float p1[2] = { this->p1.x, this->p1.y };
	float p2[2] = { this->p2.x, this->p2.y };
	igInputFloat2("Point 1", p1, NULL, ImGuiInputTextFlags_None);
	igInputFloat2("Point 2", p2, NULL, ImGuiInputTextFlags_None);
	this->p1.x = (double)p1[0];
	this->p1.y = (double)p1[1];
	this->p2.x = (double)p2[0];
	this->p2.y = (double)p2[1];
}

double line(Line* this, double x) {
	return this->p1.y + (this->p2.y - this->p1.y) * ((x - this->p1.x) / (this->p2.x - this->p1.x));
}

void line_plot(void* _this, bool is_selected) {
	Line* this = _this;

	ImPlot_SetNextLineStyle(this->base.color, 2.0f);

	ImPlotRect limits = ImPlot_GetPlotLimits(-1, -1);

	double xs[2] = { limits.X.Min, limits.X.Max };
	double ys[2] = { line(this, xs[0]), line(this, xs[1]) };
	ImPlot_PlotLine_doublePtrdoublePtr(this->base.validated_label, xs, ys, 2, ImPlotLineFlags_None, 0, sizeof(double));
	if (is_selected) {
		ImPlot_DragPoint("Point 1", &this->p1.x, &this->p1.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		ImPlot_DragPoint("Point 2", &this->p2.x, &this->p2.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
	}
}

ShapeVTable line_default_vtable = {
	.draw_properties_window = line_draw_properties_window,
	.plot = line_plot,
	.delete = shape_delete
};

bool line_new(Line* this, const char* initial_label, ImVec4 color, DVec2 p1, DVec2 p2) {
	if (!shape_new(&this->base, initial_label, color)) return false;
	this->base.vtable = &line_default_vtable;
	this->p1 = p1;
	this->p2 = p2;
}
