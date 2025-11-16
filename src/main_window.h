#pragma once
#include "window_with_imgui.h"
#include "texture.h"
#include "linked_list.h"

typedef struct {
	WindowWithImGui base;

	int icon_image_count;
	GLFWimage* icon_images;

	Texture bg_tex;
	Texture about_tex;
	ImFont* default_font;

	int selected_shape_index;
	LinkedList shapes;
} MainWindow;

bool main_window_new(MainWindow* this, int width, int height, const char* title);
void main_window_delete(MainWindow* this);

void main_window_on_imgui_draw(void* this);
