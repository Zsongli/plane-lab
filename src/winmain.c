#ifdef _WIN32

#include "utils.h"
#include <stdio.h>
#include <windows.h>

int main(int argc, char** argv);

static void after_main(void) {
	printf("Press Enter to exit...\n");
	char c = getchar();
	DISCARD(c);
	FreeConsole();

}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
	DISCARD(hInstance);
	DISCARD(hPrevInstance);
	DISCARD(lpCmdLine);
	DISCARD(nShowCmd);

#ifdef _DEBUG
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	atexit(after_main); // this is needed instead of just putting it at the end of wWinMain, because debugmalloc prints its leaks after wWinMain returns
#endif

	return main(__argc, __argv);
}

#endif