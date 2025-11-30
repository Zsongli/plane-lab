#include "circle.h"
#include <cimplot.h>
#include <imgui_macros.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "shape_types.h"
#include <debugmalloc.h>

ShapeVTable circle_default_vtable = {
	.on_draw_properties_window = circle_on_draw_properties_window,
	.on_plot = circle_on_plot,
	.delete = shape_delete,
	.serialize = circle_serialize
};

bool circle_new(Circle* this, const char* initial_label, ImVec4 initial_color, ImPlotPoint center, double radius) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &circle_default_vtable;

	this->center = center;
	this->radius = radius;
	this->radius_tool_pos = (ImPlotPoint){ center.x + radius, center.y };
	return true;
}

void circle_on_draw_properties_window(void* _this) {
	Circle* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Circle");
	igPopFont();

	double radius = this->radius;

	igDragScalarN("Center", ImGuiDataType_Double, &this->center, 2, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);
	igDragScalar("Radius", ImGuiDataType_Double, &radius, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);

	// TODO: there really should be a better way for validation, this feels too restrictive when zoomed in enough
	bool valid = radius >= 0.001;
	if (valid) {
		this->radius = radius;

		double radius_tool_rot = atan2(this->radius_tool_pos.y - this->center.y, this->radius_tool_pos.x - this->center.x);
		this->radius_tool_pos.x = this->center.x + this->radius * cos(radius_tool_rot);
		this->radius_tool_pos.y = this->center.y + this->radius * sin(radius_tool_rot);
	}

	igSeparator();

	igPushFont(NULL, 22.0f);
	igText("Equation");
	igPopFont();

	if (this->center.x >= 0.0 && this->center.y >= 0.0) igText(" (x - %.3f)^2 + (y - %.3f)^2 = %.3f^2", this->center.x, this->center.y, this->radius);
	else if (this->center.x < 0.0 && this->center.y >= 0.0) igText(" (x + %.3f)^2 + (y - %.3f)^2 = %.3f^2", -this->center.x, this->center.y, this->radius);
	else if (this->center.x >= 0.0 && this->center.y < 0.0) igText(" (x - %.3f)^2 + (y + %.3f)^2 = %.3f^2", this->center.x, -this->center.y, this->radius);
	else igText(" (x + %.3f)^2 + (y + %.3f)^2 = %.3f^2", -this->center.x, -this->center.y, this->radius);
}

bool _circle_value_fn(void* _this, double x, Buffer* out_values) {
	Circle* this = _this;

	double dx = x - this->center.x;
	double radius_sqr = this->radius * this->radius;
	double dx_sqr = dx * dx;
	if (dx_sqr > radius_sqr) return true;

	double dy = sqrt(radius_sqr - dx_sqr);
	double y1 = this->center.y + dy;
	double y2 = this->center.y - dy;
	
	if (!buffer_push_back(out_values, &y1, sizeof(y1))) return false;
	if (!buffer_push_back(out_values, &y2, sizeof(y2))) return false;
	
	return true;
}

void circle_on_plot(void* _this, bool is_selected) {
	Circle* this = _this;

	if (is_selected) {
		ImPlotPoint prev_center = this->center;
		ImPlot_DragPoint(0, &this->center.x, &this->center.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);
		// move radius tool along with center
		this->radius_tool_pos.x += this->center.x - prev_center.x;
		this->radius_tool_pos.y += this->center.y - prev_center.y;

		ImPlotPoint prev_radius_tool_pos = this->radius_tool_pos;
		ImPlot_DragPoint(1, &this->radius_tool_pos.x, &this->radius_tool_pos.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);

		double dx = this->radius_tool_pos.x - this->center.x;
		double dy = this->radius_tool_pos.y - this->center.y;
		double distance_sqr = dx * dx + dy * dy;
		bool valid = distance_sqr >= 0.001 * 0.001;
		if (valid) {
			this->radius = sqrt(distance_sqr);
		}
		else {
			this->radius_tool_pos = prev_radius_tool_pos;
		}

		ImDrawList_AddLine(ImPlot_GetPlotDrawList(),
			ImPlot_PlotToPixels_double(this->center.x, this->center.y, -1, -1),
			ImPlot_PlotToPixels_double(this->radius_tool_pos.x, this->radius_tool_pos.y, -1, -1),
			igGetColorU32_Vec4(this->base.color), 2.0f
		);

		shape_draw_point_tooltip((Shape*)this, _circle_value_fn);
	}

	ImVec2 center = ImPlot_PlotToPixels_double(this->center.x, this->center.y, -1, -1);
	ImVec2 max_x = ImPlot_PlotToPixels_double(this->center.x + this->radius, this->center.y, -1, -1);
	ImVec2 max_y = ImPlot_PlotToPixels_double(this->center.x, this->center.y + this->radius, -1, -1);
	ImVec2 radius = {
		max_x.x - center.x,
		max_y.y - center.y
	};

	ImVec4 fill_color = this->base.color;
	fill_color.w *= 0.25f;

	ImDrawList_AddEllipseFilled(ImPlot_GetPlotDrawList(), center, radius, igGetColorU32_Vec4(fill_color), 0.0f, 0);
	ImDrawList_AddEllipse(ImPlot_GetPlotDrawList(), center, radius, igGetColorU32_Vec4(this->base.color), 0.0f, 0, 2.0f);
}

#pragma pack(push, 1)
typedef struct {
	ImPlotPoint center;
	double radius;
} SerializedCircle;
#pragma pack(pop)

bool circle_serialize(void* _this, Buffer* out_data) {
	Circle* this = _this;

	uint8_t shape_type = (uint8_t)ShapeType_Circle;
	buffer_push_back(out_data, &shape_type, sizeof(shape_type));

	Buffer base;
	if (!buffer_new(&base, 0)) return false;
	if (!shape_serialize(this, &base)) goto fail;

	if (!buffer_reserve(out_data, sizeof(shape_type) + base.size + sizeof(SerializedCircle))) goto fail;
	if (!buffer_push_back(out_data, base.data, base.size)) goto fail;
	buffer_delete(&base);

	SerializedCircle circle_data = {
		.center = this->center,
		.radius = this->radius
	};
	return buffer_push_back(out_data, &circle_data, sizeof(circle_data));

fail:
	buffer_delete(&base);
	return false;
}

bool circle_deserialize(Circle* this, Buffer* in_data) {
	if (!shape_deserialize(&this->base, in_data)) return false;
	this->base.vtable = &circle_default_vtable;

	SerializedCircle circle_data;
	if (!buffer_consume(in_data, sizeof(SerializedCircle), &circle_data)) return false;

	this->center = circle_data.center;
	this->radius = circle_data.radius;
	// TODO: add some validation
	this->radius_tool_pos = (ImPlotPoint){ this->center.x + this->radius, this->center.y };

	return true;
}