#include "Chip8.h"
#include "Renderer.h"
#include "Shader.h"
#include "Model.h"
#include "Window.h"
#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
	int video_scale = 10;
	int width = VIDEO_WIDTH * video_scale;
	int height = VIDEO_HEIGHT * video_scale;

	// Create window
	Window window;
	if (!window.Create(width, height))
		return -1;

	// Create renderer
	DX::Renderer renderer(&window);
	renderer.Create();

	// Create shader
	DX::Shader shader(&renderer);
	shader.LoadPixelShader(L"PixelShader.hlsl");
	shader.LoadVertexShader(L"VertexShader.hlsl");
	shader.Use();

	// Model
	DX::Model model(&renderer);
	model.Create();

	// Emulation core
	Chip8 chip8;
	//chip8.LoadROM("IBM Logo.ch8");
	//chip8.LoadROM("chip8-test-suite.ch8");
	chip8.LoadROM("IBM Logo.ch8");
	int video_pitch = sizeof(chip8.VideoBuffer[0]) * VIDEO_WIDTH;

	// Message loop
	bool quit = false;
	while (!quit)
	{
		// Poll window messages
		window.Poll(&quit);

		// Inputs
		chip8.Keypad[0x0] = window.KeyState[MapVirtualKeyW('X', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x1] = window.KeyState[MapVirtualKeyW('1', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x2] = window.KeyState[MapVirtualKeyW('2', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x3] = window.KeyState[MapVirtualKeyW('3', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x4] = window.KeyState[MapVirtualKeyW('Q', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x5] = window.KeyState[MapVirtualKeyW('W', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x6] = window.KeyState[MapVirtualKeyW('E', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x7] = window.KeyState[MapVirtualKeyW('A', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x8] = window.KeyState[MapVirtualKeyW('S', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0x9] = window.KeyState[MapVirtualKeyW('D', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0xA] = window.KeyState[MapVirtualKeyW('Z', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0xB] = window.KeyState[MapVirtualKeyW('C', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0xC] = window.KeyState[MapVirtualKeyW('4', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0xD] = window.KeyState[MapVirtualKeyW('R', MAPVK_VK_TO_VSC)];
		chip8.Keypad[0xE] = window.KeyState[MapVirtualKeyW('F', MAPVK_VK_TO_VSC)];

		// Execute instructions
		chip8.Cycle();

		// Update screen
		renderer.Clear();
		model.UpdateTexture(chip8.VideoBuffer.data(), video_pitch);
		model.Render();
		renderer.Present();
	}

	return 0;
}
