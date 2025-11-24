#include "main_window.h"
#include "resources/index.h"
#include "utils.h"
#include <imgui_macros.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stb_image.h>
#include "shapes/shape.h"
#include "shapes/line.h"
#include "shapes/circle.h"
#include "resource_management/ico_file.h"
#include <float.h>
#include <debugmalloc.h>

enum ShapeType {
	ShapeType_Line,
	ShapeType_Circle,
	ShapeType_Parabola,
	ShapeType_Hyperbola
};

void _initialize_imgui_io_config(MainWindow* this) {
	ImGuiIO* const io = &this->base.imgui_context->IO;

	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io->ConfigDpiScaleFonts = true;
	io->ConfigDpiScaleViewports = true;
	io->LogFilename = NULL;
	io->IniFilename = NULL;
}

bool _initialize_imgui_style(MainWindow* this) {
	ImGuiContext* imgui_context = this->base.imgui_context;
	ImGuiStyle* style = &imgui_context->Style;
	ImGuiIO* io = &imgui_context->IO;

	igStyleColorsLight(style);
	style->FrameRounding = 4.0f;
	style->FrameBorderSize = 1.0f;

	const ImFontConfig font_cfg = {
		.FontDataOwnedByAtlas = false,
		.OversampleH = 0,
		.OversampleV = 0,
		.GlyphMaxAdvanceX = FLT_MAX,
		.RasterizerMultiply = 1.0f,
		.RasterizerDensity = 1.0f,
		.EllipsisChar = 0
	};

	assert(RESOURCE_SIZE(DROID_SANS_TTF) < INT_MAX);
	// we can assume imgui won't modify the font data and cast away the const
	this->default_font = ImFontAtlas_AddFontFromMemoryTTF(io->Fonts, (void*)RESOURCE_DATA(DROID_SANS_TTF), (int)RESOURCE_SIZE(DROID_SANS_TTF), 20.0f, &font_cfg, NULL);
	if (!this->default_font) return false;

	return true;
}

bool _load_resources(MainWindow* this) {
	if (!texture_new_from_encoded(
		&this->bg_tex,
		RESOURCE_DATA(PLANELAB_LOGO_SILHOUETTE_PNG),
		RESOURCE_SIZE(PLANELAB_LOGO_SILHOUETTE_PNG)
	)) goto fail_bg;

	if (!texture_new_from_encoded(
		&this->about_tex,
		RESOURCE_DATA(PLANELAB_LOGO_PNG),
		RESOURCE_SIZE(PLANELAB_LOGO_PNG)
	)) goto fail_about;

	IcoEntry* entries = ico_file_get_entries(RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO), &this->icon_image_count);
	if (!entries) goto fail_icon;
	this->icon_images = calloc(sizeof(GLFWimage), this->icon_image_count);
	if (!this->icon_images) goto fail_icon_2;

	for (size_t i = 0; i < this->icon_image_count; i++) {
		if (ico_entry_is_png(entries[i], RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO))) {
			assert(entries[i].size < INT_MAX);
			// stbi can read pngs directly
			this->icon_images[i].pixels = stbi_load_from_memory(
				(uint8_t*)RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO) + entries[i].offset,
				(int)entries[i].size,
				&this->icon_images[i].width,
				&this->icon_images[i].height,
				NULL,
				4
			);
		}
		else {
			// unfortunately lots of copying is happening here
			size_t bmp_size;
			void* bmp_data = ico_entry_bmp_to_real_bmp(entries[i], RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO), &bmp_size); // convert to a format that stbi can read without complaining
			if (!bmp_data) goto fail_icon_3;

			assert(bmp_size < INT_MAX);
			this->icon_images[i].pixels = stbi_load_from_memory(
				bmp_data,
				(int)bmp_size,
				&this->icon_images[i].width,
				&this->icon_images[i].height,
				NULL,
				4
			);
			free(bmp_data);
		}
		if (!this->icon_images[i].pixels) goto fail_icon_3;
	}
	free(entries);

	return true;

fail_icon_3:
	free(this->icon_images);
fail_icon_2:
	free(entries);
fail_icon:
	texture_delete(&this->about_tex);
fail_about:
	texture_delete(&this->bg_tex);
fail_bg:
	return false;
}

void _free_resources(MainWindow* this) {
	for (size_t i = 0; i < this->icon_image_count; i++) {
		stbi_image_free(this->icon_images[i].pixels);
	}
	free(this->icon_images);
	texture_delete(&this->about_tex);
	texture_delete(&this->bg_tex);
	ImFontAtlas_RemoveFont(this->base.imgui_context->IO.Fonts, this->default_font);
}

void _draw_about_window(MainWindow* this) {
	igBegin("About", &this->show_about_window, ImGuiWindowFlags_AlwaysAutoResize);

	ImVec2 rgn_avail = igGetContentRegionAvail();

	igImage(texture_to_imtextureref(&this->about_tex), (ImVec2) { rgn_avail.x, rgn_avail.x* ((float)this->about_tex.height / this->about_tex.width) }, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f });

	igSeparator();
	igDummy((ImVec2) { 0.0f, 10.0f });

	igTextWrapped("A simple C application for graphing geometric shapes using ImGui and ImPlot. Made for a university assignment.");

	igDummy((ImVec2) { 0.0f, 10.0f });
	igSeparator();
	igDummy((ImVec2) { 0.0f, 10.0f });

#ifdef _DEBUG
	const char* build_config = "Debug";
#else
	const char* build_config = "Release";
#endif
	igText("Build: %s - %s", build_config, __TIMESTAMP__);
	igTextLinkOpenURL("Source code", "https://github.com/Zsongli/plane-lab");

	igEnd();
}

void _remove_shape(MainWindow* this, Shape* shape) {
	shape->vtable->delete(shape);
	free(shape);

	if (shape == this->shapes.head->value) linked_list_remove_at(&this->shapes, 0);
	else {
		LinkedListNode* iter = this->shapes.head;
		while (iter->next && iter->next->value != shape) iter = iter->next;
		linked_list_remove_after(&this->shapes, iter);
	}
}

void _draw_menu_bar(MainWindow* this) {
	igBeginMenuBar();
	if (igBeginMenu("File", true)) {
		if (igMenuItem_Bool("Save", "Ctrl+S", false, true)) {
			// TODO
		}
		else if (igMenuItem_Bool("Open", "Ctrl+O", false, true)) {
			// TODO
		}
		else if (igMenuItem_Bool("Exit", "Alt+F4", false, true))
			glfwSetWindowShouldClose(this->base.base.glfw_window, GLFW_TRUE);
		igEndMenu();
	}
	if (igBeginMenu("Edit", true)) {
		if (igMenuItem_Bool("Deselect", "Esc", false, this->selected_shape)) this->selected_shape = NULL;
		if (igMenuItem_Bool("Delete selected", "Del", false, this->selected_shape)) {
			_remove_shape(this, this->selected_shape);
			this->selected_shape = NULL;
		}
		igEndMenu();
	}
	if (igBeginMenu("Help", true)) {
		if (igMenuItem_Bool("About", NULL, false, true)) this->show_about_window = true;
		igEndMenu();
	}
	igEndMenuBar();

}

void _draw_background_image(MainWindow* this, ImVec2 viewport_size) {
	float scale = fminf(viewport_size.x / this->bg_tex.width, viewport_size.y / this->bg_tex.height);
	ImVec2 image_size = {
		this->bg_tex.width * scale,
		this->bg_tex.height * scale
	};
	ImVec2 image_offset = {
		(viewport_size.x - image_size.x) * 0.5f,
		(viewport_size.y - image_size.y) * 0.5f
	};
	igSetCursorPos(image_offset);
	igImageWithBg(texture_to_imtextureref(&this->bg_tex), image_size, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f }, (ImVec4) { 0.0f, 0.0f, 0.0f, 0.0f }, (ImVec4) { 0.1f, 0.1f, 0.1f, 0.1f });
}

ImGuiID _draw_shell(MainWindow* this) { // this function is based on ImGui::DockSpaceOverViewport
	igPushStyleColor_Vec4(ImGuiCol_DockingEmptyBg, (ImVec4) { 0.0f, 0.0f, 0.0f, 0.0f });
	ImGuiViewport* viewport = igGetMainViewport();
	igSetNextWindowPos(viewport->WorkPos, ImGuiCond_None, (ImVec2) { 0.0f, 0.0f });
	igSetNextWindowSize(viewport->WorkSize, ImGuiCond_None);
	igSetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags host_window_flags = 0;
	host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

	char label[32];
	sprintf_s(label, IM_ARRAYSIZE(label), "WindowOverViewport_%08X", viewport->ID);

	igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
	igPushStyleVar_Float(ImGuiStyleVar_WindowBorderSize, 0.0f);
	igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, (ImVec2) { 0.0f, 0.0f });
	igBegin(label, NULL, host_window_flags);
	igPopStyleVar(3);

	_draw_menu_bar(this);

	ImVec2 cursor_pos_before_dockspace = igGetCursorPos();
	ImGuiID dockspace_id = igDockSpace(igGetID_Str("DockSpace"), (ImVec2) { 0.0f, 0.0f }, ImGuiDockNodeFlags_None, NULL);
	igSetCursorPos(cursor_pos_before_dockspace);

	_draw_background_image(this, viewport->WorkSize);

	igEnd();
	igPopStyleColor(1);
	return dockspace_id;
}

void _draw_graph_window(MainWindow* this) {
	igBegin("Graph", NULL, ImGuiWindowFlags_None);
	ImPlot_SetNextAxesLimits(-100.0, 100.0, -100.0, 100.0, ImPlotCond_Once);
	if (ImPlot_BeginPlot("Graph", (ImVec2) { -1, -1 }, ImPlotFlags_NoTitle | ImPlotFlags_NoFrame)) {
		LinkedListNode* iter = this->shapes.head;
		while (iter) {
			Shape* shape = iter->value;
			iter = iter->next;
			igPushID_Ptr(shape);
			shape_plot(shape, shape == this->selected_shape);
			igPopID();
		}
		ImPlot_EndPlot();
	}
	igEnd();
}

void _draw_properties_window(MainWindow* this) {
	igBegin("Properties", NULL, ImGuiWindowFlags_None);
	if (!this->selected_shape) igTextDisabled("No shape selected.");
	else shape_draw_properties_window(this->selected_shape);
	igEnd();
}

void _draw_selector_window(MainWindow* this) {
	const ImGuiStyle* style = &this->base.imgui_context->Style;

	igBegin("Selector", NULL, ImGuiWindowFlags_None);

	ImVec2 rgn_avail = igGetContentRegionAvail();
	ImVec2 add_button_size = igCalcTextSize("Add new", NULL, false, 0.0f);

	igPushStyleVar_Vec2(ImGuiStyleVar_CellPadding, (ImVec2) { 0.0f, 0.0f });

	if (igBeginTable("##ShapeList", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY, (ImVec2) { -FLT_MIN, rgn_avail.y - add_button_size.y - style->FramePadding.y * 2.0f }, 0.0f)) {
		if (this->shapes.count == 0) igTextDisabled("No shapes added yet.");

		igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
		igTableSetupColumn(NULL, ImGuiTableColumnFlags_WidthFixed, 20.0f, 0);

		igBeginGroup();

		LinkedListNode* iter = this->shapes.head;
		while (iter) {
			igTableNextRow(ImGuiTableRowFlags_None, 0.0f);

			Shape* shape = iter->value;
			iter = iter->next;

			igPushID_Ptr(shape);

			igTableSetColumnIndex(0);

			igSetCursorPos((ImVec2){ igGetCursorPosX()+4.0f, igGetCursorPosY() + style->ItemSpacing.y / 2 }); // applying padding manually
			if (igSelectable_Bool(shape->validated_label.data, shape == this->selected_shape, ImGuiSelectableFlags_None, (ImVec2) { igGetColumnWidth(0) - 4.0f, 0.0f }))
				this->selected_shape = shape;

			ImVec2 selectable_size = igGetItemRectSize();

			igTableSetColumnIndex(1);

			igPushStyleVar_Float(ImGuiStyleVar_FrameRounding, 0.0f);
			igPushStyleVar_Float(ImGuiStyleVar_FrameBorderSize, 0.0f);
			igPushStyleColor_Vec4(ImGuiCol_Button, style->Colors[ImGuiCol_WindowBg]);
			igPushStyleColor_U32(ImGuiCol_ButtonHovered, IM_COL32(225, 66, 66, 150));
			igPushStyleColor_U32(ImGuiCol_ButtonActive, IM_COL32(210, 15, 15, 150));
			if (igButton("x", (ImVec2) { igGetColumnWidth(1), selectable_size.y })) {
				_remove_shape(this, shape);
				this->selected_shape = NULL;
			}
			igPopStyleColor(3);
			igPopStyleVar(2);


			igPopID();
		}
		igPopStyleVar(1);

		igEndGroup();

		igEndTable();
	}
	if (igBeginMenu("Add new", true)) {
		if (igMenuItem_Bool("Line", NULL, false, true)) {
			Line* _line = malloc(sizeof(Line));
			line_new(_line, "New Line", (ImVec4) { 0, 0, 0, 1 }, (DVec2) { -1, -1 }, (DVec2) { 1, 1 });
			linked_list_push_back(&this->shapes, _line);
		}
		else if (igMenuItem_Bool("Circle", NULL, false, true)) {
			Circle* _circle = malloc(sizeof(Circle));
			circle_new(_circle, "New Circle", (ImVec4) { 0, 0, 0, 1 }, (DVec2) { 0, 0 }, 1.0);
			linked_list_push_back(&this->shapes, _circle);
		}
		igEndMenu();
	}
	//if (igButton("Add new", (ImVec2) { rgn_avail.x, add_button_size.y })) {
	//	//igOpenPopup_Str("AddShapePopup", ImGuiPopupFlags_None);
	//	/*Line* _line = malloc(sizeof(Line));
	//	line_new(_line, "New Line", (ImVec4) { 0, 0, 0, 1 }, (DVec2) { -1, -1 }, (DVec2) { 1, 1 });
	//	linked_list_push_back(&this->shapes, _line);*/
	//	Circle* _circle = malloc(sizeof(Circle));
	//	circle_new(_circle, "New Circle", (ImVec4) { 0, 0, 0, 1 }, (DVec2) { 0, 0 }, 1.0);
	//	linked_list_push_back(&this->shapes, _circle);
	//}
	igEnd();
}

void main_window_on_imgui_draw(void* _this) {
	MainWindow* this = _this;

	ImGuiID dockspace_id = _draw_shell(this);

	static bool once = true;
	if (once) {
		once = false;

		// set up default docking layout
		igDockBuilderRemoveNode(dockspace_id);
		igDockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_None);
		igDockBuilderSetNodeSize(dockspace_id, this->base.imgui_context->IO.DisplaySize);
		ImGuiID left, right = dockspace_id;
		left = igDockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.25f, NULL, &right);
		ImGuiID left_top, left_bottom;
		left_top = igDockBuilderSplitNode(left, ImGuiDir_Up, 0.4f, NULL, &left_bottom);

		igDockBuilderDockWindow("Graph", right);
		igDockBuilderDockWindow("Selector", left_top);
		igDockBuilderDockWindow("Properties", left_bottom);

		igDockBuilderFinish(dockspace_id);
	}

#if _DEBUG
	igShowDemoWindow(NULL);
#endif
	_draw_selector_window(this);
	_draw_properties_window(this);
	_draw_graph_window(this);
	if (this->show_about_window) _draw_about_window(this);
}

void main_window_delete(void* _this) {
	MainWindow* this = _this;

	LinkedListNode* iter = this->shapes.head;
	while (iter) {
		Shape* shape = iter->value;
		iter = iter->next;
		shape->vtable->delete(shape);
		free(shape);
	}
	linked_list_delete(&this->shapes);

	_free_resources(this);
	window_with_imgui_delete(&this->base);

	puts("Deleted main window");
}

WindowWithImGuiVTable main_window_vtable = {
	.base = {
		.on_draw = window_with_imgui_on_draw,
		.delete = main_window_delete,
	},
	.on_imgui_draw = main_window_on_imgui_draw,
};

bool main_window_new(MainWindow* this, int width, int height, const char* title) {

	if (!window_with_imgui_new(&this->base, width, height, title)) {
		perror("Failed to create base window with ImGui\n");
		goto fail_window;
	}
	this->base.base.vtable = (WindowVTable*)&main_window_vtable;

	_initialize_imgui_io_config(this);
	if (!_initialize_imgui_style(this)) {
		perror("Failed to initialize ImGui style\n");
		goto fail_other;
	}
	if (!_load_resources(this)) {
		perror("Failed to load resources\n");
		goto fail_other;
	}

	assert(this->icon_image_count < INT_MAX);
	glfwSetWindowIcon(this->base.base.glfw_window, (int)this->icon_image_count, this->icon_images);

	this->selected_shape = NULL;
	linked_list_new(&this->shapes);

	this->show_about_window = false;

	printf("Main window created\n");
	return true;

fail_other:
	window_with_imgui_delete(&this->base);
fail_window:
	return false;
}
