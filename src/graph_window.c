#include "graph_window.h"
#include <cimplot.h>
#include "utils.h"
#include <float.h>
#include <assert.h>
#include <imgui_macros.h>
#include <nfd.h>
#include <debugmalloc.h>

bool _graph_window_generate_unique_window_name(char* label, char* separator, void* id, String* out_window_id) {
	string_new(out_window_id, label);
	string_append(out_window_id, separator);
	char ptr[32];
	sprintf_s(ptr, sizeof(ptr), "%p", id);
	string_append(out_window_id, ptr);
	return true;
}

#undef free // debugmalloc is too stupid to handle this case
bool _graph_window_save_as(GraphWindow* this) {
	Graph* graph = &this->graph;

	nfdchar_t* save_path;
	if (NFD_SaveDialog("plab", NULL, &save_path) != NFD_OKAY) return false;

	if (!string_set(&graph->working_file, save_path)) goto fail;
	if (!graph_save_to_file(graph, save_path)) goto fail;
	string_delete(&this->window_name);
	if (!_graph_window_generate_unique_window_name(filename(graph->working_file.data), "###", this, &this->window_name)) goto fail;

	free(save_path);
	return true;

fail:
	free(save_path);
	return false;
}
#define free(P) debugmalloc_free_full((P), "free", __FILE__, __LINE__)

bool _graph_window_save(GraphWindow* this) {
	Graph* graph = &this->graph;
	if (graph->working_file.size > 0)
		return graph_save_to_file(graph, graph->working_file.data);

	return _graph_window_save_as(this);
}

bool graph_window_load_from_file(GraphWindow* this, char* path) {
	if (!graph_load_from_file(&this->graph, path)) return false;
	
	string_delete(&this->window_name);
	if (!_graph_window_generate_unique_window_name(filename(path), "###", this, &this->window_name)) return false;

	return true;
}

bool graph_window_new(GraphWindow* this) {
	graph_new(&this->graph);
	this->dockspace_initialized = false;
	this->should_close = false;
	if (!_graph_window_generate_unique_window_name("(unsaved)", "###", this, &this->window_name)) goto fail_graph;
	if (!_graph_window_generate_unique_window_name("Graph", "##", this, &this->graph_window_name)) goto fail_window_name;
	if (!_graph_window_generate_unique_window_name("Properties", "##", this, &this->properties_window_name)) goto fail_properties;
	if (!_graph_window_generate_unique_window_name("Selector", "##", this, &this->selector_window_name)) goto fail_selector;

	printf("Created graph window with name: %s\n", (char*)this->window_name.data);

	return true;

fail_selector:
	string_delete(&this->properties_window_name);
fail_properties:
	string_delete(&this->graph_window_name);
fail_window_name:
	string_delete(&this->window_name);
fail_graph:
	graph_delete(&this->graph);
	return false;
}

void graph_window_delete(GraphWindow* this) {
	printf("Deleting graph window with name: %s\n", (char*)this->window_name.data);
	string_delete(&this->selector_window_name);
	string_delete(&this->properties_window_name);
	string_delete(&this->graph_window_name);
	string_delete(&this->window_name);
	graph_delete(&this->graph);
}

void _graph_window_draw_graph(GraphWindow* this) {
	if (igBegin(this->graph_window_name.data, NULL, ImGuiWindowFlags_None)) {
		ImPlot_SetNextAxesLimits(-100.0, 100.0, -100.0, 100.0, ImPlotCond_Once);
		if (ImPlot_BeginPlot("##Graph", (ImVec2) { -1, -1 }, ImPlotFlags_NoTitle | ImPlotFlags_NoFrame)) {
			graph_plot_shapes(&this->graph);
			ImPlot_EndPlot();
		}
	}
	igEnd();
}

void _graph_window_draw_properties(GraphWindow* this) {

	if (igBegin(this->properties_window_name.data, NULL, ImGuiWindowFlags_None)) {
		if (!this->graph.selected_shape) igTextDisabled("No shape selected.");
		else shape_draw_properties_window(this->graph.selected_shape);
	}
	igEnd();
}

void _graph_window_draw_selector(GraphWindow* this) {
	const ImGuiStyle* style = igGetStyle();

	if (igBegin(this->selector_window_name.data, NULL, ImGuiWindowFlags_None)) {

		if (linked_list_count(&this->graph.shapes) == 0) {
			igTextDisabled("No shapes added yet.");
			igSeparator();
		}
		else {

			ImVec2 rgn_avail = igGetContentRegionAvail();
			ImVec2 add_button_size = igCalcTextSize("Add new", NULL, false, 0.0f);

			igPushStyleVar_Vec2(ImGuiStyleVar_CellPadding, (ImVec2) { 0.0f, 0.0f });

			if (igBeginTable("##ShapeList", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY, (ImVec2) { -FLT_MIN, rgn_avail.y - add_button_size.y - style->FramePadding.y * 2.0f }, 0.0f)) {

				igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
				igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 20.0f, 0);

				igBeginGroup();

				LinkedListNode* iter = this->graph.shapes.head;
				while (iter != NULL) {
					Shape* shape = iter->value;
					assert(shape != NULL);

					iter = iter->next;

					igTableNextRow(ImGuiTableRowFlags_None, 0.0f);

					igPushID_Ptr(shape);

					igTableSetColumnIndex(0);

					igSetCursorPos((ImVec2) { igGetCursorPosX() + 4.0f, igGetCursorPosY() + style->ItemSpacing.y / 2 }); // applying padding manually
					if (igSelectable_Bool(shape->validated_label.data, shape == this->graph.selected_shape, ImGuiSelectableFlags_None, (ImVec2) { igGetColumnWidth(0) - 4.0f, 0.0f }))
						this->graph.selected_shape = shape;

					ImVec2 selectable_size = igGetItemRectSize();

					igTableSetColumnIndex(1);

					igPushStyleVar_Float(ImGuiStyleVar_FrameRounding, 0.0f);
					igPushStyleVar_Float(ImGuiStyleVar_FrameBorderSize, 0.0f);
					igPushStyleColor_Vec4(ImGuiCol_Button, style->Colors[ImGuiCol_WindowBg]);
					igPushStyleColor_U32(ImGuiCol_ButtonHovered, IM_COL32(225, 66, 66, 150));
					igPushStyleColor_U32(ImGuiCol_ButtonActive, IM_COL32(210, 15, 15, 150));
					if (igButton("x", (ImVec2) { igGetColumnWidth(1), selectable_size.y })) {
						graph_remove_shape(&this->graph, shape);
						this->graph.selected_shape = NULL;
					}
					igPopStyleColor(3);
					igPopStyleVar(2);

					igPopID();
				}
				igEndGroup();

				igEndTable();
			}

			igPopStyleVar(1);
		}

		if (igBeginMenu("Add new", true)) {
			for (ShapeType shape = 0; shape < ShapeType_Count; shape++)
				if (igMenuItem_Bool(shape_type_to_string(shape), NULL, false, true))
					graph_add_default_shape(&this->graph, shape);
			igEndMenu();
		}

	}
	igEnd();
}

void _graph_window_draw_shell(GraphWindow* this, char* label) {

	igPushFont(NULL, 28.0f);
	ImVec2 text_size = igCalcTextSize(label, NULL, false, 0.0f);
	igPopFont();
	ImVec2 avail = igGetContentRegionAvail();
	ImVec2 text_pos = { igGetCursorPosX() + avail.x / 2 - text_size.x / 2, igGetCursorPosY() + avail.y / 2 - text_size.y / 2 };

	igPushStyleColor_U32(ImGuiCol_DockingEmptyBg, IM_COL32_BLACK_TRANS);
	this->dockspace_id = igDockSpace(igGetID_Ptr(this), (ImVec2) { 0.0f, 0.0f }, ImGuiDockNodeFlags_None, NULL);
	igPopStyleColor(1);

	igPushFont(NULL, 28.0f);
	igPushStyleColor_Vec4(ImGuiCol_Text, (ImVec4) { 0.1f, 0.1f, 0.1f, 0.1f });

	igSetCursorPos(text_pos);
	igTextWrapped("%s", label);

	igPopStyleColor(1);
	igPopFont();
}

void graph_window_setup_docking_layout(GraphWindow* this) {
	igDockBuilderRemoveNode(this->dockspace_id);
	igDockBuilderAddNode(this->dockspace_id, ImGuiDockNodeFlags_AutoHideTabBar);

	ImVec2 window_size = igGetWindowSize();
	igDockBuilderSetNodeSize(this->dockspace_id, window_size);

	ImGuiID left, right = this->dockspace_id;
	left = igDockBuilderSplitNode(this->dockspace_id, ImGuiDir_Left, 0.325f, NULL, &right);

	ImGuiID left_top, left_bottom;
	left_top = igDockBuilderSplitNode(left, ImGuiDir_Up, 0.4f, NULL, &left_bottom);

	igDockBuilderDockWindow(this->graph_window_name.data, right);
	igDockBuilderDockWindow(this->selector_window_name.data, left_top);
	igDockBuilderDockWindow(this->properties_window_name.data, left_bottom);

	igDockBuilderFinish(this->dockspace_id);
}

void _graph_window_draw_menu_bar(GraphWindow* this) {
	if (igBeginMenuBar()) {
		if (igBeginMenu("File", true)) {
			if (igMenuItem_Bool("Save", NULL, false, true)) {
				_graph_window_save(this);
			}
			if (igMenuItem_Bool("Save as", NULL, false, true)) {
				_graph_window_save_as(this);
			}
			igSeparator();
			if (igMenuItem_Bool("Close", NULL, false, true)) {
				this->should_close = true;
			}
			igEndMenu();
		}

		if (igBeginMenu("Edit", true)) {
			if (igMenuItem_Bool("Deselect", NULL, false, this->graph.selected_shape)) this->graph.selected_shape = NULL;
			if (igMenuItem_Bool("Delete selected", NULL, false, this->graph.selected_shape)) {

				graph_remove_shape(&this->graph, this->graph.selected_shape);
				this->graph.selected_shape = NULL;
			}
			igEndMenu();
		}

		igEndMenuBar();
	}
}

void graph_window_draw(GraphWindow* this) {
	bool shouldnt_close = !this->should_close;

	igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2) { 0.0f, 0.0f });
	bool show_window = igBegin(this->window_name.data, &shouldnt_close, ImGuiWindowFlags_MenuBar);
	igPopStyleVar(1);

	this->should_close = !shouldnt_close;

	if (show_window) {
		_graph_window_draw_shell(this, this->graph.working_file.size > 0 ? filename(this->graph.working_file.data) : "(unsaved)");
		_graph_window_draw_menu_bar(this);
		_graph_window_draw_selector(this);
		_graph_window_draw_properties(this);
		_graph_window_draw_graph(this);
	}

	igEnd();
}

