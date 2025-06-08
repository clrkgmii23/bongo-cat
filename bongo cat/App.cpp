#include "App.h"

#include <Windows.h>
#include <windowsx.h>
#include <wincodec.h>

#include "BaseWindow.h"
#include "utils.h"

PCWSTR App::ClassName() const { return L"APP WINDOW"; }
int App::MainLoop() {
	MSG msg = {};
	while (GetMessage(&msg, nullptr, NULL, NULL)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
App::~App() {
	// for images!!
	for (size_t i = 0; i < RESOURCE_COUNT; i++)
	{
		if (graph.resources[i]) {
			graph.resources[i]->Release();
			graph.resources[i] = nullptr;
		}
	}

	if (graph.pRenderTarget)
		graph.pRenderTarget->Release();
	if (graph.pFactory)
		graph.pFactory->Release();
	if(graph.hdcMem) DeleteDC(graph.hdcMem);
	if (graph.hBitmap) DeleteObject(graph.hBitmap);
	CoUninitialize();
}
// initialise both direct2d and some graphics variables
HRESULT App::SetUpGraphics() {
	if (graph.pFactory) {
		graph.pFactory->Release();
		graph.pFactory = nullptr;
	}
	if (graph.pRenderTarget) {
		graph.pRenderTarget->Release();
		graph.pRenderTarget = nullptr;
	}

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &graph.pFactory);
	if (FAILED(hr)) return hr;

	// Get window size
	RECT rc;
	GetClientRect(graph.hwnd, &rc);
	graph.clientRect = D2D1::RectF((FLOAT)rc.left, (FLOAT)rc.top, (FLOAT)rc.right, (FLOAT)rc.bottom);
	graph.width = rc.right - rc.left;
	graph.height = rc.bottom - rc.top;

	// Create DIBSection (offscreen bitmap with alpha)
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = graph.width;
	bmi.bmiHeader.biHeight = -graph.height; // top-down bitmap
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = nullptr;
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = CreateDIBSection(hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	SelectObject(hdcMem, hBitmap);
	ReleaseDC(NULL, hdcScreen);

	graph.hdcMem = hdcMem;
	graph.hBitmap = hBitmap;

	// Create Direct2D render target for the HDC
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);

	hr = graph.pFactory->CreateDCRenderTarget(&props, (ID2D1DCRenderTarget**)&graph.pRenderTarget);
	return hr;

}

ID2D1Bitmap* App::LoadImageC(App* app, LPCWSTR pathName) {
	HRESULT hr;
	// make image factory
	IWICImagingFactory* pImageFactory;
	hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pImageFactory)
	);

	IWICBitmapDecoder* pDecoder = NULL;
	pImageFactory->CreateDecoderFromFilename(
		pathName,          // Image file path
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);

	IWICBitmapFrameDecode* pFrame = NULL;

	pDecoder->GetFrame(0, &pFrame);

	IWICFormatConverter* pConverter = NULL;

	pImageFactory->CreateFormatConverter(&pConverter);

	pConverter->Initialize(
		pFrame,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.0f,
		WICBitmapPaletteTypeMedianCut
	);
	ID2D1Bitmap* pBitmap = NULL;

	app->graph.pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, &pBitmap);
	// release stuff
	if (pConverter)pConverter->Release();
	if (pFrame) pFrame->Release();
	if (pDecoder) pDecoder->Release();
	if (pImageFactory) pImageFactory->Release();
	// return the actual image bitmap
	return pBitmap;
}

BOOL App::LoadResources() {
	if (graph.resources[0]) {
		graph.resources[0]->Release();
		graph.resources[0] = nullptr;
	}
	graph.resources[0] = LoadImageC(this, L"bongo_cat.png");
	return graph.resources[0] ? TRUE : FALSE;
}

LRESULT App::HandleMessage(UINT uMSG, WPARAM wParam, LPARAM lParam) {
	switch (uMSG) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE: {
		SendMessage(hwnd, WM_DESTROY, NULL, NULL);
		return 0;
	}
	case WM_SETCURSOR: {
		POINT pt;
		GetCursorPos(&pt);
		RECT clientRect;
		GetClientRect(graph.hwnd, &clientRect);
		ScreenToClient(graph.hwnd, &pt);
		if (PtInRect(&clientRect, pt)) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return 0;
		}
		break;
	}
	// resize window, update variables assosiated with it
	case WM_SIZE: {
		// get new render target, and update screensize variables
		RECT rc;
		GetClientRect(graph.hwnd, &rc);
		graph.clientRect = D2D1::Rect(rc.left, rc.top, rc.right, rc.bottom);
		D2D1_SIZE_U screenSize = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
		graph.width = screenSize.width;
		graph.height = screenSize.height;

		graph.pRenderTarget->Resize(screenSize);

		InvalidateRect(graph.hwnd, NULL, TRUE);
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(graph.hwnd, &ps);
		graph.pRenderTarget->BeginDraw();
		OnPaint();
		HRESULT hr = graph.pRenderTarget->EndDraw();
		if (FAILED(hr)) {
			HRError(L"End Draw is Failllinnng!! :((");
			if (FAILED(SetUpGraphics())) {
				HRError(L"SetUpGraphics FAILED in paint!!!");
			}
		}
		EndPaint(graph.hwnd, &ps);
		return 0;
	}
	//input
						//  right click
	case WM_RBUTTONDOWN: {
		// start drag
		MouseInput::SetPrev(MouseInput::GetMousePos());
		SetCapture(hwnd);
		MouseInput::SetDragging(true);
		return 0;
	}
	case WM_MOUSEMOVE: {
		// moving using right click
		if (!MouseInput::IsDragging()) return 0;
		auto current = MouseInput::GetMousePos();
		auto prev = MouseInput::GetPrev();
		RECT windowRect;
		GetWindowRect(hwnd, &windowRect);
		int deltaX = current.x - prev.x;
		int deltaY = current.y - prev.y;
		SetWindowPos(graph.hwnd, HWND_TOP, windowRect.left + deltaX,
						windowRect.top + deltaY, 0,0, SWP_NOSIZE);
		MouseInput::SetPrev(current);
		return 0;
	}
	case WM_RBUTTONUP: {
		// stop drag
		ReleaseCapture();
		MouseInput::SetDragging(false);
		return 0;
	}
						// keyboard
	case WM_KEYDOWN: {
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		return 0;
	}

	default:
		return DefWindowProc(hwnd, uMSG, wParam, lParam);
	}
	return TRUE;
}

// most useful code here
void App::OnStart() {
	LoadResources();
	OnPaint(); // <-- Manually force drawing before UpdateWindow
	InvalidateRect(graph.hwnd, NULL, FALSE);
	UpdateWindow(graph.hwnd);


}
void App::OnPaint() {
	RECT rc;
	GetClientRect(graph.hwnd, &rc);

	// Bind HDC to render target
	((ID2D1DCRenderTarget*)graph.pRenderTarget)->BindDC(graph.hdcMem, &rc);

	graph.pRenderTarget->BeginDraw();
	graph.pRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0)); // transparent background

	if (graph.resources[0]) {
		graph.pRenderTarget->DrawBitmap(
			graph.resources[0],
			graph.clientRect,
			1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
		);
	}

	HRESULT hr = graph.pRenderTarget->EndDraw();
	if (FAILED(hr)) {
		SetUpGraphics();
		return;
	}

	// Push bitmap to screen with alpha
	POINT ptSrc = { 0, 0 };
	SIZE sizeWnd = { graph.width, graph.height };
	POINT ptWnd = {};
	ClientToScreen(graph.hwnd, &ptWnd);

	BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
	HDC hdcScreen = GetDC(NULL);
	UpdateLayeredWindow(
		graph.hwnd, hdcScreen, &ptWnd, &sizeWnd,
		graph.hdcMem, &ptSrc, 0, &blend, ULW_ALPHA
	);
	ReleaseDC(NULL, hdcScreen);
}