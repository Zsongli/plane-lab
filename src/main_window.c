#include "main_window.h"
#include "resources/index.h"
#include "utils.h"
#include <imgui_macros.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stb_image.h>
#include "shape.h"
#include "line.h"
#include "ico_file.h"

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

	ImFontConfig font_cfg = {
		.FontDataOwnedByAtlas = false,
		.OversampleH = 0,
		.OversampleV = 0,
		.GlyphMaxAdvanceX = igGET_FLT_MAX(),
		.RasterizerMultiply = 1.0f,
		.RasterizerDensity = 1.0f,
		.EllipsisChar = 0
	};
	this->default_font = ImFontAtlas_AddFontFromMemoryTTF(io->Fonts, RESOURCE_DATA(DROID_SANS_TTF), RESOURCE_SIZE(DROID_SANS_TTF), 20.0f, &font_cfg, NULL);
	if (!this->default_font) return false;

	return true;
}

bool _load_resources(MainWindow* this) {
	if (!texture_new_from_png_data(
		&this->bg_tex,
		RESOURCE_DATA(PLANELAB_LOGO_SILHOUETTE_PNG),
		RESOURCE_SIZE(PLANELAB_LOGO_SILHOUETTE_PNG)
	)) goto fail_bg;

	if (!texture_new_from_png_data(
		&this->about_tex,
		RESOURCE_DATA(PLANELAB_LOGO_PNG),
		RESOURCE_SIZE(PLANELAB_LOGO_PNG)
	)) goto fail_about;

	IcoEntry* entries = ico_file_get_entries(RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO), &this->icon_image_count);
	if (!entries) goto fail_icon;
	this->icon_images = calloc(sizeof(GLFWimage), this->icon_image_count);
	if (!this->icon_images) goto fail_icon_2;
	// this only works if the ico entries are in PNG format
	for (size_t i = 0; i < this->icon_image_count; i++)
	{
		this->icon_images[i].pixels = stbi_load_from_memory(
			RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO) + entries[i].offset,
			entries[i].size,
			&this->icon_images[i].width,
			&this->icon_images[i].height,
			NULL,
			4
		);
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
	for (size_t i = 0; i < this->icon_image_count; i++)
	{
		stbi_image_free(this->icon_images[i].pixels);
	}
	free(this->icon_images);
	texture_delete(&this->about_tex);
	texture_delete(&this->bg_tex);
	ImFontAtlas_RemoveFont(this->base.imgui_context->IO.Fonts, this->default_font);
}

void _draw_about_popup(MainWindow* this) {
	if (igBeginPopupModal("About", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		ImVec2 rgn_avail;
		igGetContentRegionAvail(&rgn_avail);

		ImTextureRef about_tex;
		texture_to_imtextureref(&this->about_tex, &about_tex);
		igImage(about_tex, (ImVec2) { rgn_avail.x, rgn_avail.x* ((float)this->about_tex.height / this->about_tex.width) }, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f });
		igTextWrapped("A simple C application for graphing geometric shapes using ImGui. Made for a university assignment.");
		igDummy((ImVec2) { 0.0f, 10.0f });

#ifdef _DEBUG
		const char* build_config = "Debug";
#else
		const char* build_config = "Release";
#endif
		igText("Build: %s - %s", build_config, __TIMESTAMP__);
		igTextLinkOpenURL("Source code", ""); // TODO

		ImVec2 text_size;
		igCalcTextSize(&text_size, "Close", NULL, false, 0.0f);
		igGetContentRegionAvail(&rgn_avail);

		igSetCursorPos((ImVec2) { igGetCursorPosX() + (rgn_avail.x - text_size.x) * 0.5f, igGetCursorPosY() + 10 });
		if (igButton("Close", (ImVec2) { 0.0f, 0.0f })) {
			igCloseCurrentPopup();
		}
		igEndPopup();
	}
}

void _draw_menu_bar(MainWindow* this) {
	bool open_about_popup = false;
	igBeginMenuBar();
	if (igBeginMenu("File", true)) {
		if (igMenuItem_Bool("Save", "Ctrl+S", NULL, true)) {
			// TODO
		}
		else if (igMenuItem_Bool("Open", "Ctrl+O", NULL, true)) {
			// TODO
		}
		else if (igMenuItem_Bool("Exit", "Alt+F4", NULL, true))
			glfwSetWindowShouldClose(this->base.base.glfw_window, GLFW_TRUE);
		igEndMenu();
	}
	if (igBeginMenu("Edit", true)) {
		if (igMenuItem_Bool("Deselect", "Esc", NULL, true)) {
			this->selected_shape_index = -1;
		}
		igEndMenu();
	}
	if (igBeginMenu("Help", true)) {
		if (igMenuItem_Bool("About", NULL, NULL, true)) {
			open_about_popup = true;
		}
		igEndMenu();
	}
	igEndMenuBar();

	if (open_about_popup)
		igOpenPopup_Str("About", ImGuiPopupFlags_None);

	_draw_about_popup(this);
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

	ImVec2 cursor_pos_before_dockspace;
	igGetCursorPos(&cursor_pos_before_dockspace);
	ImGuiID dockspace_id = igDockSpace(igGetID_Str("DockSpace"), (ImVec2) { 0.0f, 0.0f }, ImGuiDockNodeFlags_None, NULL);
	igSetCursorPos(cursor_pos_before_dockspace);

	float scale = fminf(viewport->WorkSize.x / this->bg_tex.width, viewport->WorkSize.y / this->bg_tex.height);
	ImVec2 image_size = {
		this->bg_tex.width * scale,
		this->bg_tex.height * scale
	};
	ImVec2 image_offset = {
		(viewport->WorkSize.x - image_size.x) * 0.5f,
		(viewport->WorkSize.y - image_size.y) * 0.5f
	};

	igSetCursorPos(image_offset);
	ImTextureRef bg_tex;
	texture_to_imtextureref(&this->bg_tex, &bg_tex);
	igImageWithBg(bg_tex, image_size, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f }, (ImVec4) { 0.0f, 0.0f, 0.0f, 0.0f }, (ImVec4) { 0.1f, 0.1f, 0.1f, 0.1f });

	igEnd();
	igPopStyleColor(1);
	return dockspace_id;
}

void _draw_graph_window(MainWindow* this) {
	igBegin("Graph", NULL, ImGuiWindowFlags_None);
	ImPlot_SetNextAxesLimits(-100.0, 100.0, -100.0, 100.0, ImPlotCond_Once);
	if (ImPlot_BeginPlot("Graph", (ImVec2) { -1, -1 }, ImPlotFlags_NoTitle | ImPlotFlags_NoFrame)) {
		for (size_t i = 0; i < this->shapes.count; i++)
		{
			Shape* shape = linked_list_value_at(&this->shapes, i);

			igPushID_Int(i);
			if (ImPlot_BeginItem(shape->validated_label, ImPlotItemFlags_None, -1)) {
				shape->vtable->plot(shape, this->selected_shape_index == i);
				ImPlot_EndItem();
			}
			igPopID();
		}
		ImPlot_EndPlot();
	}
	igEnd();
}

void _draw_properties_window(MainWindow* this) {
	igBegin("Properties", NULL, ImGuiWindowFlags_None);
	if (this->selected_shape_index == -1) igTextDisabled("No shape selected.");
	else {
		Shape* selected_shape = linked_list_value_at(&this->shapes, this->selected_shape_index);
		selected_shape->vtable->draw_properties_window(selected_shape);
	}
	igEnd();
}

void _draw_selector_window(MainWindow* this) {
	igBegin("Selector", NULL, ImGuiWindowFlags_None);

	ImVec2 rgn_avail;
	igGetContentRegionAvail(&rgn_avail);
	ImVec2 add_button_size;
	igCalcTextSize(&add_button_size, "Add new", NULL, false, 0.0f);
	add_button_size.y += this->base.imgui_context->Style.FramePadding.x * 3.0f;

	if (igBeginListBox("##ShapeList", (ImVec2) { -igGET_FLT_MIN(), rgn_avail.y - add_button_size.y })) {
		if (this->shapes.count == 0) igTextDisabled("No shapes added yet.");

		for (int i = 0; i < this->shapes.count; i++)
		{
			Shape* shape = linked_list_value_at(&this->shapes, i);

			igPushID_Int(i);
			if (igSelectable_Bool(shape->validated_label, this->selected_shape_index == i, ImGuiSelectableFlags_None, (ImVec2) { 0.0f, 0.0f }))
				this->selected_shape_index = i;
			igPopID();
		}
		igEndListBox();
	}
	if (igButton("Add new", (ImVec2) { rgn_avail.x, 0.0f })) {
		Line* line = malloc(sizeof(Line));
		line_new(line, "New Line", (ImVec4) { 0, 0, 0, 1 }, (DVec2) { -1, -1 }, (DVec2) { 1, 1 });
		linked_list_push_back(&this->shapes, line);
		this->selected_shape_index = this->shapes.count - 1;
	}
	igEnd();
}

void main_window_on_imgui_draw(void* _this) {
	MainWindow* this = _this;

	ImGuiID dockspace_id = _draw_shell(this);
	DO_ONCE_BEGIN()
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
	DO_ONCE_END();

	_draw_graph_window(this);
	_draw_properties_window(this);
	_draw_selector_window(this);
}

WindowWithImGuiVTable main_window_vtable = {
	.base = {
		.on_draw = window_with_imgui_on_draw,
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

	glfwSetWindowIcon(this->base.base.glfw_window, this->icon_image_count, this->icon_images);

	this->selected_shape_index = -1;
	linked_list_new(&this->shapes);

	printf("Main window created\n");
	return true;

fail_other:
	window_with_imgui_delete(&this->base);
fail_window:
	return false;
}

void main_window_delete(MainWindow* this) {
	for (size_t i = 0; i < this->shapes.count; i++) {
		Shape* shape = linked_list_value_at(&this->shapes, i);
		shape->vtable->delete(shape);
		free(shape);
	}
	_free_resources(this);
	window_with_imgui_delete(&this->base);
}
