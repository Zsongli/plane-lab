#include <GLFW/glfw3.h>
#include "utils.h"
#include "main_window.h"
#include "data_structures/dynamic_string.h"

#define INITIAL_WINDOW_WIDTH 800
#define INITIAL_WINDOW_HEIGHT 600

int main(int argc, char** argv) {
	DISCARD(argc); DISCARD(argv);

	if (!glfwInit()) {
		perror("Failed to initialize GLFW\n");
		return -1;
	}

	MainWindow window;
	if (!main_window_new(&window, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT, APP_NAME)) {
		perror("Failed to create main window\n");
		glfwTerminate();
		return -1;
	}

	window_run_main_loop((Window*)&window);

	window.base.base.vtable->delete(&window);
	glfwTerminate();
	return 0;
}
