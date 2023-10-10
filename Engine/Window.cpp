#include "Window.h"

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
	m_hInst = GetModuleHandle(nullptr);
	if (m_hInst == nullptr)
	{
		return;
	}

	m_width = width;
	m_height = height;

	// �E�B���h�E�̐ݒ�
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(m_hInst, IDI_APPLICATION);
	wc.hCursor = LoadCursor(m_hInst, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = title;
	wc.hIconSm = LoadIcon(m_hInst, IDI_APPLICATION);

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
	m_hWnd = CreateWindowEx(
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
		m_hInst,
		nullptr
	);

	// �E�B���h�E�̕\��
	ShowWindow(m_hWnd, SW_SHOWNORMAL);

	// �E�B���h�E�Ƀt�H�[�J�X����
	SetFocus(m_hWnd);
}

HINSTANCE Window::HInstance()
{
	return m_hInst;
}

HWND Window::HWnd()
{
	return m_hWnd;
}

UINT Window::Width()
{
	return m_width;
}

UINT Window::Height()
{
	return m_height;
}

float Window::AspectRate()
{
	return (float)m_width / (float)m_height;
}
