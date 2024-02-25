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
	void TickTime();

	double DeltaTime() const { return delta_time_; }

private:
	HINSTANCE hinstance_;
	HWND hwnd_ = NULL;
	UINT width_ = 640u;
	UINT height_ = 480u;
	LARGE_INTEGER time_freq_ = {};
	LARGE_INTEGER time_start_ = {}, time_end_ = {};
	double frame_time_ = 1.0 / 60.0;
	float delta_time_ = 0.0f;
};