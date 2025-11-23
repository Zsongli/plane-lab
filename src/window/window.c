#include "window.h"
#include "../utils.h"
#include <stdlib.h>

void window_on_draw(void* this) {
	DISCARD(this);
	return;
}

void window_delete(void* _this) {
	Window* this = _this;

	glfwDestroyWindow(this->glfw_window);
}

WindowVTable window_default_vtable = {
	.on_draw = window_on_draw,
	.delete = window_delete
};

bool window_new(Window* this, int width, int height, const char* title) {
	this->vtable = &window_default_vtable;
	
	this->glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!this->glfw_window) {
		perror("Failed to create GLFW window\n");
		return false;
	}

	return true;
}

void window_run_main_loop(Window* this) {

	while (!glfwWindowShouldClose(this->glfw_window)) {
		glfwPollEvents();

		this->vtable->on_draw(this);
		
		glfwSwapBuffers(this->glfw_window);
	}
}
