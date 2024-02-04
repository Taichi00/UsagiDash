#pragma once
#define NOMINMAX
#include <Windows.h>

#pragma comment(lib, "winmm.lib")

class Window
{
public:
	Window(const TCHAR* title, const UINT width, const UINT height);

	HINSTANCE HInstance();
	HWND HWnd();
	UINT Width();
	UINT Height();
	float AspectRate();

	double TimeAdjustment();

private:
	HINSTANCE hinstance_;
	HWND hwnd_ = NULL;
	UINT width_;
	UINT height_;
	LARGE_INTEGER time_freq_;
	LARGE_INTEGER time_start_, time_end_;
	double frame_time_;
};