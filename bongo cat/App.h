#pragma once
#include "BaseWindow.h""
#include <windows.h>
class App: public BaseWindow<App> {
public:
	/*HWND hwnd;
	graphics graph;*/
	int MainLoop();
	HRESULT SetUpGraphics();
	ID2D1Bitmap* LoadImageC(App* app, LPCWSTR pathName);
	BOOL LoadResources();
	LRESULT HandleMessage(UINT uMSG, WPARAM wParam, LPARAM lParam);
	//App() {};
	~App();
	void OnPaint();
	void OnStart();
protected:
	virtual PCWSTR ClassName() const;
};