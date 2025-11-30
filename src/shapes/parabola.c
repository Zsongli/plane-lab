#include "parabola.h"
#include "shape_types.h"
#include <cimplot.h>
#include <math.h>

ShapeVTable parabola_default_vtable = {
	.on_draw_properties_window = parabola_on_draw_properties_window,
	.on_plot = parabola_on_plot,
	.serialize = parabola_serialize,
	.delete = shape_delete
};

bool parabola_new(Parabola* this, char* initial_label, ImVec4 initial_color, ImPlotPoint vertex, double scalar) {
	if (!shape_new(&this->base, initial_label, initial_color)) return false;
	this->base.vtable = &parabola_default_vtable;

	this->vertex = vertex;
	this->scalar = scalar;
	return true;
}

void parabola_on_draw_properties_window(void* _this) {
	Parabola* this = _this;

	igPushFont(NULL, 22.0f);
	igText("Parabola");
	igPopFont();

	igDragScalarN("Vertex", ImGuiDataType_Double, &this->vertex, 2, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);

	double scalar = this->scalar;
	igDragScalar("Scalar", ImGuiDataType_Double, &scalar, 0.001f, NULL, NULL, "%.3f", ImGuiInputTextFlags_None);

	bool used_text_input = igGetIO_Nil()->WantTextInput;

	bool valid = fabs(scalar) > 0.001;
	if (valid) {
		this->scalar = scalar;
	}
	else if (!used_text_input) {
		if (this->scalar > scalar) this->scalar = scalar - 0.002f; // FIXME: might be buggy if user changes value by more than 0.002 in one frame
		else this->scalar = scalar + 0.002f;
	}

	igSeparator();

	igPushFont(NULL, 22.0f);
	igText("Equation");
	igPopFont();

	if (this->vertex.x >= 0.0 && this->vertex.y >= 0.0) igText("y = %.3f(x - %.3f)^2 + %.3f", this->scalar, this->vertex.x, this->vertex.y);
	else if (this->vertex.x < 0.0 && this->vertex.y >= 0.0) igText("y = %.3f(x + %.3f)^2 + %.3f", this->scalar, -this->vertex.x, this->vertex.y);
	else if (this->vertex.x >= 0.0 && this->vertex.y < 0.0) igText("y = %.3f(x - %.3f)^2 - %.3f", this->scalar, this->vertex.x, -this->vertex.y);
	else igText("y = %.3f(x + %.3f)^2 - %.3f", this->scalar, -this->vertex.x, -this->vertex.y);
}

double _parabola(void* _this, double x) {
	Parabola* this = _this;
	double a = this->scalar;
	double h = this->vertex.x;
	double k = this->vertex.y;
	return a * (x - h) * (x - h) + k;
}

bool _parabola_value_fn(void* _this, double x, Buffer* out_values) {
	Parabola* this = _this;
	double y = _parabola(this, x);
	return buffer_push_back(out_values, &y, sizeof(y));
}

void parabola_on_plot(void* _this, bool is_selected) {
	Parabola* this = _this;

	if (is_selected) {
		ImPlot_DragPoint(0, &this->vertex.x, &this->vertex.y, this->base.color, 5.0f, ImPlotDragToolFlags_None, NULL, NULL, NULL);

		shape_draw_point_tooltip((Shape*)this, _parabola_value_fn);
	}

	ImPlotRect limits = ImPlot_GetPlotLimits(-1, -1);

	ImPlotPoint p1 = { limits.X.Min, _parabola(this, limits.X.Min) };
	ImPlotPoint p3 = { limits.X.Max, _parabola(this, limits.X.Max) };

	double midpoint = (p1.x + p3.x) / 2.0;
	ImPlotPoint p2 = { 2.0 * midpoint - (p1.x + p3.x) / 2.0, 2 * _parabola(this, midpoint) - (p1.y + p3.y) / 2.0 };

	ImVec2 p1_screen = ImPlot_PlotToPixels_double(p1.x, p1.y, -1, -1);
	ImVec2 p2_screen = ImPlot_PlotToPixels_double(p2.x, p2.y, -1, -1);
	ImVec2 p3_screen = ImPlot_PlotToPixels_double(p3.x, p3.y, -1, -1);

	ImDrawList_AddBezierQuadratic(ImPlot_GetPlotDrawList(),
		p1_screen,
		p2_screen,
		p3_screen,
		igGetColorU32_Vec4(this->base.color),
		2.0f,
		0
	);
}

#pragma pack(push, 1)
typedef struct {
	ImPlotPoint vertex;
	double scalar;
} SerializedParabola;
#pragma pack(pop)

bool parabola_serialize(void* _this, Buffer* out_data) {
	Parabola* this = _this;

	uint8_t shape_type = (uint8_t)ShapeType_Parabola;
	buffer_push_back(out_data, &shape_type, sizeof(shape_type));

	Buffer base;
	if (!buffer_new(&base, 0)) return false;
	if (!shape_serialize(this, &base)) goto fail;
	if (!buffer_reserve(out_data, sizeof(shape_type) + base.size + sizeof(SerializedParabola))) goto fail;

	if (!buffer_push_back(out_data, base.data, base.size)) goto fail;
	buffer_delete(&base);

	SerializedParabola parabola_data = {
		.vertex = this->vertex,
		.scalar = this->scalar
	};

	return buffer_push_back(out_data, &parabola_data, sizeof(parabola_data));

fail:
	buffer_delete(&base);
	return false;
}

bool parabola_deserialize(Parabola* this, Buffer* in_data) {
	if (!shape_deserialize(&this->base, in_data)) return false;
	this->base.vtable = &parabola_default_vtable;

	SerializedParabola parabola_data;
	if (!buffer_consume(in_data, sizeof(SerializedParabola), &parabola_data)) return false;

	this->vertex = parabola_data.vertex;
	this->scalar = parabola_data.scalar;

	return true;
}
