#include <iostream>
#include <Windows.h>
#include <fstream>
#include <chrono>
#include <thread>
#include "Chip8.h"

Chip8 emulator;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			// All painting occurs here, between BeginPaint and EndPaint.
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));


			int videoPitch = sizeof(emulator.video[0]) * VIDEO_WIDTH;

			for (int i = 0; i < 64; ++i)
			{
				for (int j = 0; j < 32; ++j)
				{
					int xpos = emulator.video[i];
					int ypos = emulator.video[i];
					SetPixel(hdc, xpos, ypos, RGB(0, 0, 0));
				}
			}

			EndPaint(hwnd, &ps);
			return 0;
		}
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

	// Emulator
	emulator.LoadROM("test_opcode.ch8");

	int videoPitch = sizeof(emulator.video[0]) * VIDEO_WIDTH;

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

		emulator.Cycle();

		RedrawWindow(m_Hwnd, NULL, NULL, RDW_INVALIDATE);

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(16ms);
	}

	DestroyWindow(m_Hwnd);
	return static_cast<int>(msg.wParam);
}