#pragma once
#include "window/window_with_imgui.h"
#include "resource_management/texture.h"
#include "graph_window.h"

typedef struct {
	WindowWithImGui base;

	Buffer icon_images;

	Texture bg_tex;
	Texture about_tex;
	ImFont* default_font;
	bool show_about_window;

	bool dockspace_initialized;
	LinkedList graph_windows;

} MainWindow;

bool main_window_new(MainWindow* this, size_t width, size_t height, const char* title);
void main_window_delete(void* this);
void main_window_on_imgui_draw(void* this);
