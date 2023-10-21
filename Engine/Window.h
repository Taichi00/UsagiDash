#pragma once
#define NOMINMAX
#include <Windows.h>

class Window
{
public:
	Window(const TCHAR* title, const UINT width, const UINT height);

	HINSTANCE HInstance();
	HWND HWnd();
	UINT Width();
	UINT Height();
	float AspectRate();

private:
	HINSTANCE m_hInst;
	HWND m_hWnd = NULL;
	UINT m_width;
	UINT m_height;
};