#pragma once
#include "window/window_with_imgui.h"
#include "resource_management/texture.h"
#include "data_structures/linked_list.h"
#include "shapes/shape.h"

typedef struct {
	WindowWithImGui base;

	size_t icon_image_count;
	GLFWimage* icon_images;

	Texture bg_tex;
	Texture about_tex;
	ImFont* default_font;
	bool show_about_window;

	Shape* selected_shape;
	LinkedList shapes;
} MainWindow;

bool main_window_new(MainWindow* this, int width, int height, const char* title);
void main_window_delete(void* this);

void main_window_on_imgui_draw(void* this);
