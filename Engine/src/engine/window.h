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
	HINSTANCE hinstance_;
	HWND hwnd_ = NULL;
	UINT width_;
	UINT height_;
};