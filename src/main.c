#include <GLFW/glfw3.h>
#include "utils.h"
#include "main_window.h"
#include <debugmalloc.h>

static const size_t INITIAL_WINDOW_WIDTH = 800;
static const size_t INITIAL_WINDOW_HEIGHT = 600;

int main(int argc, char** argv) {
	DISCARD(argc); DISCARD(argv);

#if _DEBUG
	// can't load large enough files with debugmalloc's default limit
	debugmalloc_max_block_size(1024 * 1024 * 1024); // 1 GB
#endif

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
