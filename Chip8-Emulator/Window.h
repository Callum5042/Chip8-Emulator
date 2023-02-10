#pragma once

#include <Windows.h>
#include <map>

class Window
{
public:
	Window();
	~Window();

	// Create window
	bool Create(int width, int height);

	// Poll messages
	void Poll(bool* quit);

	// Get Win32 handle
	inline HWND GetHwnd() { return m_Hwnd; }

	// Handle messages
	LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Get window size
	void GetSize(int* width, int* height);

	// Keys
	std::map<WORD, bool> KeyState;

private:
	HWND m_Hwnd = NULL;
	void HandleKeyboardEvent(UINT msg, WPARAM wParam, LPARAM lParam);
};