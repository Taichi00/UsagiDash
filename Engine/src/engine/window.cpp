#include "window.h"
#include "game/game.h"
#include <timeapi.h>
#include <mmsystem.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSKEYDOWN:
		if ((wp == VK_RETURN) && (lp & (1 << 29)))
		{
			Game::Get()->ToggleFullscreen();
		}
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}

Window::Window(const TCHAR* title, const UINT width, const UINT height)
{
#if !_DEBUG
	// �R���\�[�����\��
	FreeConsole();
#endif

	hinstance_ = GetModuleHandle(nullptr);
	if (hinstance_ == nullptr)
	{
		return;
	}

	width_ = width;
	height_ = height;

	// �E�B���h�E�̐ݒ�
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hinstance_, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hinstance_, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = title;
	wc.hIconSm = LoadIcon(hinstance_, IDI_APPLICATION);

	// �E�B���h�E�N���X�̓o�^
	RegisterClassEx(&wc);

	// �E�B���h�E�T�C�Y�̐ݒ�
	RECT rect = {};
	rect.right = static_cast<LONG>(width);
	rect.bottom = static_cast<LONG>(height);

	// �E�B���h�E�T�C�Y�𒲐�
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rect, style, FALSE);

	// �E�B���h�E�̐���
	hwnd_ = CreateWindowEx(
		0,
		title,
		title,
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
		nullptr,
		nullptr,
		hinstance_,
		nullptr
	);

	// �E�B���h�E�̕\��
	ShowWindow(hwnd_, SW_SHOWNORMAL);

	// �E�B���h�E�Ƀt�H�[�J�X����
	SetFocus(hwnd_);

	// ���g���擾
	QueryPerformanceFrequency(&time_freq_);

	// �v���J�n���Ԃ̏�����
	QueryPerformanceCounter(&time_start_);

	// 1�t���[���ɂ������鎞��
	frame_time_ = 1.0 / 60.0;
}

HINSTANCE Window::HInstance()
{
	return hinstance_;
}

HWND Window::HWnd()
{
	return hwnd_;
}

UINT Window::Width()
{
	return width_;
}

UINT Window::Height()
{
	return height_;
}

float Window::AspectRate()
{
	return (float)width_ / (float)height_;
}

double Window::TimeAdjustment()
{
	double frame_time = static_cast<double>(time_end_.QuadPart - time_start_.QuadPart) / static_cast<double>(time_freq_.QuadPart);
	double fps = 0.0;

	// �]�T������΂��̕��҂�
	if (frame_time < frame_time_)
	{
		// sleep���鎞�Ԃ��v�Z
		DWORD sleep_time = static_cast<DWORD>((frame_time_ - frame_time) * 1000);
		timeBeginPeriod(1);
		Sleep(sleep_time);
		timeEndPeriod(1);
		//return 0;
	}

	fps = 1.0 / frame_time;
	printf("%f\n", fps);
	return fps;
}

void Window::TickTime()
{
	/*QueryPerformanceCounter(&time_end_);

	TimeAdjustment();*/

	QueryPerformanceCounter(&time_end_);
	delta_time_ = (double)(time_end_.QuadPart - time_start_.QuadPart) / (double)(time_freq_.QuadPart);
	delta_time_ = std::min(std::max(delta_time_, 0.00001f), 0.05f);

	QueryPerformanceCounter(&time_start_);
}
