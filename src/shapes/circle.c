#include "circle.h"
#include <cimplot.h>
#include <imgui_macros.h>
#include <math.h>

ShapeVTable circle_default_vtable = {
	.on_draw_properties_window = circle_on_draw_properties_window,
	.on_plot = circle_on_plot,
	.delete = shape_delete
};

bool circle_new(Circle* this, const char* initial_label, ImVec4 initial_color, DVec2 center, double radius) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &circle_default_vtable;

	this->center = center;
	this->radius = radius;
	this->validated_radius = radius;
	return true;
}

void circle_on_draw_properties_window(void* _this) {
	Circle* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Circle");
	igPopFont();

	this->radius = this->validated_radius;

	igDragScalarN("Center", ImGuiDataType_Double, &this->center, 2, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);
	igDragScalar("Radius", ImGuiDataType_Double, &this->radius, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);

	// TODO: there really should be a better way for validation, this feels too restrictive when zoomed in enough
	bool valid = this->radius >= 0.001;
	if (valid) {
		this->validated_radius = this->radius;
	}
	else {
		this->radius = this->validated_radius;
	}
}

void circle_on_plot(void* _this, bool is_selected) {
	Circle* this = _this;

	if (is_selected) {
		ImPlot_DragPoint(0, &this->center.x, &this->center.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		
		double y = this->center.y;
		double x = this->center.x + this->validated_radius;
		// TODO: didn't have time to implement a dragpoint with fixed y value so it just drags freely for now but only the x value is used
		ImPlot_DragPoint(1, &x, &y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		this->validated_radius = fmax(fabs(x - this->center.x), 0.001);

		ImDrawList_AddLine(ImPlot_GetPlotDrawList(),
			ImPlot_PlotToPixels_double(this->center.x, this->center.y, -1, -1),
			ImPlot_PlotToPixels_double(this->center.x + this->validated_radius, this->center.y, -1, -1),
			igGetColorU32_Vec4(this->base.color), 2.0f
		);
	}
	
	ImVec2 center = ImPlot_PlotToPixels_double(this->center.x, this->center.y, -1, -1);
	ImVec2 max_x = ImPlot_PlotToPixels_double(this->center.x + this->validated_radius, this->center.y, -1, -1);
	ImVec2 max_y = ImPlot_PlotToPixels_double(this->center.x, this->center.y + this->validated_radius, -1, -1);
	ImVec2 radius = {
		max_x.x -center.x,
		max_y.y - center.y
	};

	ImVec4 fill_color = this->base.color;
	fill_color.w *= 0.25f;

	ImDrawList_AddEllipseFilled(ImPlot_GetPlotDrawList(), center, radius, igGetColorU32_Vec4(fill_color), 0.0f, 0);
	ImDrawList_AddEllipse(ImPlot_GetPlotDrawList(), center, radius, igGetColorU32_Vec4(this->base.color), 0.0f, 0, 2.0f);
}
