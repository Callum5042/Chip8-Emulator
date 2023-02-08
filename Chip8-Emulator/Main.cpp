#include <iostream>
#include <Windows.h>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Setup window class
	WNDCLASS wc = {};
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.lpszClassName = L"Chip8-Emulator";

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, L"Error", L"RegisterClass Failed", MB_OK | MB_ICONERROR);
		return -1;
	}

	// Create window
	HWND m_Hwnd = CreateWindow(wc.lpszClassName, L"Chip 8 Emulator", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (m_Hwnd == NULL)
	{
		MessageBox(NULL, L"Error", L"CreateWindow Failed", MB_OK | MB_ICONERROR);
		return -1;
	}

	ShowWindow(m_Hwnd, SW_SHOWNORMAL);

	// Loop
	MSG msg = {};
	bool m_IsRunning = true;
	while (m_IsRunning)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				m_IsRunning = false;
			}

			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	DestroyWindow(m_Hwnd);
	return static_cast<int>(msg.wParam);
}