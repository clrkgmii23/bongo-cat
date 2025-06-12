#include <Windows.h>
#include <d2d1.h>
#pragma comment(lib,"d2d1.lib")

#include "App.h"
#include "utils.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
	if (FAILED(CoInitialize(NULL))) {
		CErrorMessage(L"Could Initialize COM");
	}


	App app;
	if (!app.Create(L"Cool Ass Title", WS_POPUP,0,0, 887, 373)) {
		return -1;
	}
	if (FAILED(app.SetUpGraphics())) {
		HRError(L"SetUpGraphics Failure");
	}

	ShowWindow(app.graph.hwnd, nCmdShow);

	app.OnStart();

	return app.MainLoop();
}