#pragma once
#include "BaseWindow.h"
#include <windows.h>
class App: public BaseWindow<App> {
public:
	int MainLoop();
	HRESULT SetUpGraphics();
	ID2D1Bitmap* LoadImageC(App* app, LPCWSTR pathName);
	BOOL LoadResources();
	LRESULT HandleMessage(UINT uMSG, WPARAM wParam, LPARAM lParam);
	~App();
	void OnPaint();
	void DrawCat();
	void OnStart();
protected:
	virtual PCWSTR ClassName() const;
};