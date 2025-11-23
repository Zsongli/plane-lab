#pragma once
#include "window.h"
#include <cimgui.h>
#include <cimplot.h>
#include <stdbool.h>

typedef struct {
	WindowVTable base;
	void (*on_imgui_draw)(void* this);
} WindowWithImGuiVTable;

WindowWithImGuiVTable window_with_imgui_default_vtable;

typedef struct {
	Window base;
	ImGuiContext* imgui_context;
	ImPlotContext* implot_context;
} WindowWithImGui;

bool window_with_imgui_new(WindowWithImGui* this, int width, int height, const char* title);
void window_with_imgui_delete(void* this);

void window_with_imgui_on_draw(void* this);

void window_with_imgui_on_imgui_draw(void* this);
