#pragma once
#include <d2d1.h>
// some useful overloads

// error messages
void CErrorMessage(const wchar_t* str);
void ErrorMessage(const wchar_t* str);
void HRError(const wchar_t* str);


static class MouseInput {
public:
	static bool IsPressed();
	static void SetPressed(bool val);
	static D2D1_POINT_2U GetMousePos();
	static D2D1_POINT_2U GetPrev();
	static void SetPrev(D2D1_POINT_2U newPrev);
	static bool IsDragging();
	static void SetDragging(bool val);

private:
	static D2D1_POINT_2U prev;
	static bool isDragging;
	static bool isPressed;

	MouseInput() = delete;
	MouseInput(const MouseInput&) = delete;
	MouseInput& operator=(const MouseInput&) = delete;
};
