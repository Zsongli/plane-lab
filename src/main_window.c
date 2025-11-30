#include "main_window.h"
#include "resources/index.h"
#include <imgui_macros.h>
#include <stdlib.h>
#include "resource_management/ico_file.h"
#include <float.h>
#include <math.h>
#include <assert.h>
#include <nfd.h>
#include <stb_image.h>
#include <debugmalloc.h>

void _main_window_initialize_imgui_io_config(MainWindow* this) {
	ImGuiIO* const io = &this->base.imgui_context->IO;

	io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io->ConfigDpiScaleFonts = true;
	io->ConfigDpiScaleViewports = true;
	io->LogFilename = NULL;
	io->IniFilename = NULL;
}

bool _main_window_initialize_imgui_style(MainWindow* this) {
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

	assert(RESOURCE_SIZE(DROID_SANS_TTF) <= INT_MAX);
	// we can assume imgui won't modify the font data and cast away the const
	this->default_font = ImFontAtlas_AddFontFromMemoryTTF(io->Fonts, (void*)RESOURCE_DATA(DROID_SANS_TTF), (int)RESOURCE_SIZE(DROID_SANS_TTF), 20.0f, &font_cfg, NULL);
	if (!this->default_font) return false;

	return true;
}

bool _main_window_load_resources(MainWindow* this) {
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

	if (!buffer_new(&this->icon_images, 0)) goto fail_icon;
	if (!ico_file_load_icons(
		RESOURCE_DATA(PLANELAB_LOGO_ICON_ICO),
		RESOURCE_SIZE(PLANELAB_LOGO_ICON_ICO),
		&this->icon_images
	)) goto fail_icon_2;

	return true;

fail_icon_2:
	buffer_delete(&this->icon_images);
fail_icon:
	texture_delete(&this->about_tex);
fail_about:
	texture_delete(&this->bg_tex);
fail_bg:
	return false;
}

void _main_window_free_resources(MainWindow* this) {
	const size_t icon_image_count = this->icon_images.size / sizeof(GLFWimage);
	GLFWimage* icon_images = (GLFWimage*)this->icon_images.data;
	for (size_t i = 0; i < icon_image_count; i++) stbi_image_free(icon_images[i].pixels);
	buffer_delete(&this->icon_images);

	texture_delete(&this->about_tex);
	texture_delete(&this->bg_tex);
	ImFontAtlas_RemoveFont(this->base.imgui_context->IO.Fonts, this->default_font);
}

void _main_window_draw_about_window(MainWindow* this) {
	if (igBegin("About", &this->show_about_window, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImVec2 avail = igGetContentRegionAvail();
		if (igGetScrollMaxY() == 0.0f)
			avail.x -= this->base.imgui_context->Style.ScrollbarSize;
		igImage(texture_to_imtextureref(&this->about_tex), (ImVec2) { avail.x, avail.x* ((float)this->about_tex.height / this->about_tex.width) }, (ImVec2) { 0.0f, 0.0f }, (ImVec2) { 1.0f, 1.0f });

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

	}
	igEnd();
}

bool _main_window_add_graph_window(MainWindow* this, char* path) {
	GraphWindow* new_window = malloc(sizeof(GraphWindow));

	if (!graph_window_new(new_window)) goto fail_new_window;
	if (path != NULL && !graph_window_load_from_file(new_window, path)) goto fail_load;
	if (!linked_list_push_back(&this->graph_windows, new_window)) goto fail_load;

	return true;

fail_load:
	graph_window_delete(new_window);
fail_new_window:
	free(new_window);
	return false;
}

#if _DEBUG
#undef free // debugmalloc is too stupid to handle this case
#endif
bool _main_window_load_graph_window_from_file(MainWindow* this) {
	nfdchar_t* path = NULL;
	if (NFD_OpenDialog("plab", NULL, &path) != NFD_OKAY) return false;
	bool result = _main_window_add_graph_window(this, path);
	free(path);
	return result;
}
#if _DEBUG
#define free(P) debugmalloc_free_full((P), "free", __FILE__, __LINE__)
#endif


void _main_window_draw_menu_bar(MainWindow* this) {
	if (igBeginMenuBar()) {
		if (igBeginMenu("File", true)) {
			if (igMenuItem_Bool("New", NULL, false, true)) {
				if (!_main_window_add_graph_window(this, NULL)) {
					perror("Failed to create new graph window");
				}
			}
			if (igMenuItem_Bool("Open", NULL, false, true)) {
				if (!_main_window_load_graph_window_from_file(this)) {
					perror("Failed to load graph window from file");
				}
			}
			igSeparator();
			if (igMenuItem_Bool("Exit", NULL, false, true))
				glfwSetWindowShouldClose(this->base.base.glfw_window, GLFW_TRUE);
			igEndMenu();
		}
		if (igBeginMenu("View", true)) {
			if (igMenuItem_Bool("Reset window layout", NULL, false, true)) {
				this->dockspace_initialized = false;
				for (LinkedListNode* iter = this->graph_windows.head; iter != NULL; iter = iter->next) {
					GraphWindow* graph_window = iter->value;
					graph_window->dockspace_initialized = false;
				}
			}
			igEndMenu();
		}
		if (igBeginMenu("Help", true)) {
			if (igMenuItem_Bool("About", NULL, false, true)) this->show_about_window = true;
			igEndMenu();
		}

		igEndMenuBar();
	}
}

void _main_window_draw_background_image(MainWindow* this, ImVec2 viewport_size) {
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

ImGuiID _main_window_draw_shell(MainWindow* this) { // this function is based on ImGui::DockSpaceOverViewport
	igPushStyleColor_U32(ImGuiCol_DockingEmptyBg, IM_COL32_BLACK_TRANS);
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

	_main_window_draw_menu_bar(this);

	ImVec2 cursor_pos_before_dockspace = igGetCursorPos();
	ImGuiID dockspace_id = igDockSpace(igGetID_Str("DockSpace"), (ImVec2) { 0.0f, 0.0f }, ImGuiDockNodeFlags_None, NULL);
	igSetCursorPos(cursor_pos_before_dockspace);

	_main_window_draw_background_image(this, viewport->WorkSize);

	igEnd();
	igPopStyleColor(1);
	return dockspace_id;
}

void main_window_on_imgui_draw(void* _this) {
	MainWindow* this = _this;

	ImGuiID dockspace_id = _main_window_draw_shell(this);

	if (!this->dockspace_initialized) {

		igDockBuilderRemoveNode(dockspace_id);
		igDockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
	}

#if _DEBUG
	igShowDemoWindow(NULL);
#endif

	LinkedListNode* iter = this->graph_windows.head;
	while (iter != NULL) {
		GraphWindow* graph_window = iter->value;
		assert(graph_window != NULL);

		iter = iter->next;

		if (graph_window->should_close) {
			if (graph_window == this->graph_windows.head->value) linked_list_remove_head(&this->graph_windows);
			else linked_list_remove_after(linked_list_find_preceding(&this->graph_windows, graph_window));

			graph_window_delete(graph_window);
			free(graph_window);

			continue;
		}

		if (!graph_window->dockspace_initialized) {
			igDockBuilderDockWindow(graph_window->window_name.data, dockspace_id);
		}

		graph_window_draw(graph_window);

		if (!graph_window->dockspace_initialized) {
			graph_window->dockspace_initialized = true;
			graph_window_setup_docking_layout(graph_window);
		}
	}

	if (!this->dockspace_initialized) {
		this->dockspace_initialized = true;
		igDockBuilderFinish(dockspace_id);
	}

	if (this->show_about_window) _main_window_draw_about_window(this);
}

void main_window_delete(void* _this) {
	MainWindow* this = _this;

	for (LinkedListNode* iter = this->graph_windows.head; iter != NULL; iter = iter->next) {
		GraphWindow* graph_window = iter->value;
		graph_window_delete(graph_window);
		free(graph_window);
	}
	linked_list_delete(&this->graph_windows);

	_main_window_free_resources(this);
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

bool main_window_new(MainWindow* this, size_t width, size_t height, const char* title) {

	if (!window_with_imgui_new(&this->base, width, height, title)) {
		perror("Failed to create base window with ImGui\n");
		goto fail_window;
	}
	this->base.base.vtable = (WindowVTable*)&main_window_vtable;

	_main_window_initialize_imgui_io_config(this);
	if (!_main_window_initialize_imgui_style(this)) {
		perror("Failed to initialize ImGui style\n");
		goto fail_other;
	}
	if (!_main_window_load_resources(this)) {
		perror("Failed to load resources\n");
		goto fail_other;
	}

	const size_t icon_image_count = this->icon_images.size / sizeof(GLFWimage);
	assert(icon_image_count <= INT_MAX);
	glfwSetWindowIcon(this->base.base.glfw_window, (int)icon_image_count, (GLFWimage*)this->icon_images.data);

	this->show_about_window = false;
	this->dockspace_initialized = false;

	linked_list_new(&this->graph_windows);

	printf("Main window created\n");
	return true;

fail_other:
	window_with_imgui_delete(&this->base);
fail_window:
	return false;
}
