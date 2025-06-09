#pragma once
#include <Windows.h>
#include <d2d1.h>
#include "utils.h"

#define RESOURCE_COUNT 10
// basic graphic elements for window
struct graphics {
	HWND hwnd;
	ID2D1Factory* pFactory;
	ID2D1DCRenderTarget* pRenderTarget;
	D2D1_RECT_F clientRect;

	HDC hdcMem;
	HBITMAP hBitmap;

	int width;
	int height;

	ID2D1Bitmap* resources[RESOURCE_COUNT]; // buffer to store up to 10 images B). awful design!
};

template <class DERIVED>
class BaseWindow {
public:
	graphics graph;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		DERIVED* pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

	}
	BaseWindow() : hwnd(NULL), graph({}) {};

	BOOL Create(LPCWSTR title, DWORD style,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int width = CW_USEDEFAULT,
		int height = CW_USEDEFAULT,
		HWND parent = NULL,
		HMENU menu = NULL) {
		WNDCLASS window = { 0 };
		window.lpfnWndProc = DERIVED::WindowProc;
		window.hInstance = GetModuleHandle(NULL);
		window.lpszClassName = ClassName();
		window.hCursor = LoadCursor(GetModuleHandle(NULL), IDC_HAND);

		if (!RegisterClass(&window)) {
			HRError(L"RegisterClass Failed :(");
			return FALSE;
		}
		hwnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, ClassName(), title, style,x, y,
			width, height, parent, menu, GetModuleHandle(NULL), this);
		graph.hwnd = hwnd;
		if (!hwnd) {
			HRError(L"CreateWindowEx Failed >:(");
			return FALSE;
		}
		return (hwnd ? TRUE : FALSE);
	};

	HWND Window() const { return hwnd; }
protected:
	virtual PCWSTR ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	HWND hwnd;
};