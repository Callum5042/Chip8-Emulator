#include "Window.h"
#include <exception>

namespace
{
	static Window* GetWindow(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Window* window = nullptr;
		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			window = reinterpret_cast<Window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
		}
		else
		{
			window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		return window;
	}

	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		// Application handling
		Window* window = GetWindow(hwnd, msg, wParam, lParam);
		if (window == nullptr)
		{
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		return window->HandleMessage(hwnd, msg, wParam, lParam);
	}
}

Window::Window()
{
}

Window::~Window()
{
	DestroyWindow(m_Hwnd);
}

bool Window::Create(int width, int height)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Setup window class
	WNDCLASS wc = {};
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = L"RoveEngine";

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, L"RegisterClass Failed", L"Error", MB_OK);
		return false;
	}

	// Create window
	m_Hwnd = CreateWindow(wc.lpszClassName, L"Chip-8 Emulator", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, this);
	if (m_Hwnd == NULL)
	{
		MessageBox(NULL, L"CreateWindow Failed", L"Error", MB_OK);
		return false;
	}

	// Show window
	ShowWindow(m_Hwnd, SW_SHOWNORMAL);
	return true;
}

void Window::Poll(bool* quit)
{
	MSG msg = {};
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			*quit = true;
		}

		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
}

LRESULT Window::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
		case WM_KEYUP:
			HandleKeyboardEvent(msg, wParam, lParam);
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Window::GetSize(int* width, int* height)
{
	RECT rect;
	GetClientRect(m_Hwnd, &rect);

	*width = rect.right - rect.left;
	*height = rect.bottom - rect.top;
}

void Window::HandleKeyboardEvent(UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Decode win32 message
	WORD key_flags = HIWORD(lParam);
	bool repeat = (key_flags & KF_REPEAT) == KF_REPEAT;
	bool alt_down = (key_flags & KF_ALTDOWN) == KF_ALTDOWN;
	WORD scan_code = LOBYTE(key_flags);
	BOOL extended_key = (key_flags & KF_EXTENDED) == KF_EXTENDED;

	if (extended_key)
	{
		scan_code = MAKEWORD(scan_code, 0xE0);
	}

	if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
	{
		KeyState[scan_code] = true;
	}
	else if (msg == WM_KEYUP || msg == WM_SYSKEYUP)
	{
		KeyState[scan_code] = false;
	}
}
