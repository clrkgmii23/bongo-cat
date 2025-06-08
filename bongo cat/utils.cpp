#include "utils.h" 

#include <Windows.h>
// Error message with a crash
void CErrorMessage(const wchar_t* str) {
	MessageBox(NULL, str, L"Crash Error Message", MB_ICONERROR);
	PostQuitMessage(0);
}
// An Error Message
void ErrorMessage(const wchar_t* str) {
	MessageBox(NULL, str, L"Error Message", MB_OK);
}
void HRError(const wchar_t* str) {
	DWORD error = GetLastError();
	wchar_t buffer[256];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, 256, NULL);
	MessageBox(NULL, buffer, str, MB_OK | MB_ICONERROR);
}

// mouse class implementation
D2D1_POINT_2U MouseInput::prev = D2D1::Point2U(0,0);
bool MouseInput::isDragging = false;
bool MouseInput::isPressed = false;

bool MouseInput::IsPressed() {
	return MouseInput::isPressed;
}

void MouseInput::SetPressed(bool val) {
	MouseInput::isPressed = val;
}
D2D1_POINT_2U MouseInput::GetPrev(){
	return prev;
}
void MouseInput::SetPrev(D2D1_POINT_2U newPrev) {
	prev = newPrev;
}
bool MouseInput::IsDragging() {
	return isDragging;
}

void MouseInput::SetDragging(bool val) {
	isDragging = val;
}

D2D1_POINT_2U MouseInput::GetMousePos() {
	POINT pt;
	GetCursorPos(&pt);
	return D2D1::Point2U(pt.x,pt.y);
}