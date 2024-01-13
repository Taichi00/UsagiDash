#include "window.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, msg, wp, lp);
}

Window::Window(const TCHAR* title, const UINT width, const UINT height)
{
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
