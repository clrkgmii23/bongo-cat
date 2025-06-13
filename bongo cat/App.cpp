#include "App.h"

#include <Windows.h>
#include <windowsx.h>
#include <wincodec.h>

#include "BaseWindow.h"
#include "utils.h"
#include "Cat.h"

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

	// no idea what any of this is:
	if (graph.hdcMem) DeleteDC(graph.hdcMem);
	if (graph.hBitmap) DeleteObject(graph.hBitmap);
	CoUninitialize();
}
HRESULT App::SetUpGraphics() {
	if (graph.pRenderTarget) {
		graph.pRenderTarget->Release();
		graph.pRenderTarget = nullptr; 
	}
	if (graph.pFactory) {
		graph.pFactory->Release();
		graph.pFactory = nullptr;
	}
	
	if (graph.hBitmap) {
		DeleteObject(graph.hBitmap);
		graph.hBitmap = nullptr;
	}
	if (graph.hdcMem) {
		DeleteDC(graph.hdcMem);
		graph.hdcMem = nullptr;
	}

	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &graph.pFactory);
	if (FAILED(hr)) return hr;

	RECT rc;
	if (!graph.hwnd) {
		return E_FAIL;
	}
	GetClientRect(graph.hwnd, &rc);
	graph.clientRect = D2D1::RectF((FLOAT)rc.left, (FLOAT)rc.top, (FLOAT)rc.right, (FLOAT)rc.bottom);
	graph.width = rc.right - rc.left;
	graph.height = rc.bottom - rc.top;

	// If window is minimized or has zero size, we can't create resources, idk why :(
	if (graph.width == 0 || graph.height == 0) {
		return S_OK;
	}

	// basically all the code from here is not even mine :)
	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = graph.width;
	bmi.bmiHeader.biHeight = -graph.height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = nullptr;
	HDC hdcScreen = GetDC(NULL);
	graph.hdcMem = CreateCompatibleDC(hdcScreen);
	if (!graph.hdcMem) {
		ReleaseDC(NULL, hdcScreen);
		return HRESULT_FROM_WIN32(GetLastError());
	}
	graph.hBitmap = CreateDIBSection(graph.hdcMem, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	if (!graph.hBitmap) {
		DeleteDC(graph.hdcMem); graph.hdcMem = nullptr;
		ReleaseDC(NULL, hdcScreen);
		return HRESULT_FROM_WIN32(GetLastError());
	}
	SelectObject(graph.hdcMem, graph.hBitmap);
	ReleaseDC(NULL, hdcScreen);

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
	);
	// create the pRenderTarget
	hr = graph.pFactory->CreateDCRenderTarget(&props, &graph.pRenderTarget);
	if (FAILED(hr)) {
		// clean up if failed
		if (graph.hdcMem) { DeleteDC(graph.hdcMem); graph.hdcMem = nullptr; }
		if (graph.hBitmap) { DeleteObject(graph.hBitmap); graph.hBitmap = nullptr; }
	}
	else {
		// what???
		((ID2D1DCRenderTarget*)graph.pRenderTarget)->BindDC(graph.hdcMem, &rc);
	}
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
	case WM_SIZE: {
		// apparently this will get of the error when starting the app
		if (!graph.hwnd) {
			return 0;
		}

		HRESULT hr = SetUpGraphics(); // this feels wrong but it works
		if (FAILED(hr)) {
			HRError(L"Failed to resetup graphics on WM_SIZE");
		}

		InvalidateRect(graph.hwnd, NULL, TRUE); // request a repaint after resize for no jitter
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(graph.hwnd, &ps);
		OnPaint();
		HRESULT hr = 0;
		if (FAILED(hr)) {
			// for some reason this will always fail at first, so let's just comment out the error
			//HRError(L"End Draw is Failllinnng!! :((");
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
		if (isOnLeft(wParam)) {
			graph.cat.ChangeState(LEFT_PAW, false);
		}
		else {
			graph.cat.ChangeState(RIGHT_PAW, false);
		}
		OnPaint();
		return 0;
	}
	case WM_KEYUP: {
		if (isOnLeft(wParam)) {
			graph.cat.ChangeState(LEFT_PAW, true);
		}
		else {
			graph.cat.ChangeState(RIGHT_PAW, true);
		}
		OnPaint();
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

	graph.cat =	 Cat();
}

// seperate function for drawing the cat
// it also has tons of smelly magic numbers
void App::DrawCat() {
	// draw body
	if (!graph.resources[0]) return;
	D2D1_RECT_F sourceRec = D2D1::RectF(0, 225, 887, 597);
	D2D1_RECT_F targetRec = D2D1::RectF(0, 0, 0, 0);
	DrawBodyPart(sourceRec, graph.clientRect);
	//// draw left paw
	//// open
	switch (graph.cat.isLeftPaw()) {
		case true : {
			sourceRec = D2D1::RectF(0, 0, 105, 109);
			targetRec = D2D1::RectF(190, 78, 295, 187);
			DrawBodyPart(sourceRec, targetRec);
			break;
		}
		case false: {
			sourceRec = D2D1::RectF(123, 0, 248, 63);
			targetRec = D2D1::RectF(194, 145, 315, 210);
			DrawBodyPart(sourceRec, targetRec);
			break;
		}
	}
	// closeed paw
	switch (graph.cat.isRightPaw()) {
		case true: {
			sourceRec = D2D1::RectF(0, 109, 92, 224);
			targetRec = D2D1::RectF(550, 160, 642, 275);
			DrawBodyPart(sourceRec, targetRec);
			break;
		}
		case false: {
			sourceRec = D2D1::RectF(125, 111, 231, 174);

			targetRec = D2D1::RectF(537, 232, 645, 295);
			DrawBodyPart(sourceRec, targetRec);
			break;
		}
	}
}

void App::DrawBodyPart(D2D1_RECT_F sourceRec, D2D1_RECT_F targetRec) {
	D2D1_RECT_F newTargetRec = targetRec * graph.sizeMultiplier;
	graph.pRenderTarget->DrawBitmap(
		graph.resources[0],
		newTargetRec,
		1.0f,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		sourceRec
	);
}


void App::OnPaint() {
	RECT rc;
 	GetClientRect(graph.hwnd, &rc);

	// Bind HDC to render target
	((ID2D1DCRenderTarget*)graph.pRenderTarget)->BindDC(graph.hdcMem, &rc);

	graph.pRenderTarget->BeginDraw();

	// start of draw
	graph.pRenderTarget->Clear(D2D1::ColorF(0,0));
	DrawCat();


	// end of draw
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