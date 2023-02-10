#include "Chip8.h"
#include "Renderer.h"
#include "Shader.h"
#include "Model.h"
#include "Window.h"
#include <chrono>
#include <iostream>
#include <string>

//void KeyDown(SDL_Keycode keycode, uint8_t* keys)
//{

//		case SDLK_w:
//			keys[5] = 1;
//			break;
//
//		case SDLK_e:
//			keys[6] = 1;
//			break;
//
//		case SDLK_a:
//			keys[7] = 1;
//			break;
//
//		case SDLK_s:
//			keys[8] = 1;
//			break;
//
//		case SDLK_d:
//			keys[9] = 1;
//			break;
//
//		case SDLK_z:
//			keys[0xA] = 1;
//			break;
//
//		case SDLK_c:
//			keys[0xB] = 1;
//			break;
//
//		case SDLK_4:
//			keys[0xC] = 1;
//			break;
//
//		case SDLK_r:
//			keys[0xD] = 1;
//			break;
//
//		case SDLK_f:
//			keys[0xE] = 1;
//			break;
//
//		case SDLK_v:
//			keys[0xF] = 1;
//			break;
//	}
//}
//
//void KeyUp(SDL_Keycode keycode, uint8_t* keys)
//{
//	switch (keycode)
//	{
//		case SDLK_x:
//			keys[0] = 0;
//			break;
//
//		case SDLK_1:
//			keys[1] = 0;
//			break;
//
//		case SDLK_2:
//			keys[2] = 0;
//			break;
//
//		case SDLK_3:
//			keys[3] = 0;
//			break;
//
//		case SDLK_q:
//			keys[4] = 0;
//			break;
//
//		case SDLK_w:
//			keys[5] = 0;
//			break;
//
//		case SDLK_e:
//			keys[6] = 0;
//			break;
//
//		case SDLK_a:
//			keys[7] = 0;
//			break;
//
//		case SDLK_s:
//			keys[8] = 0;
//			break;
//
//		case SDLK_d:
//			keys[9] = 0;
//			break;
//
//		case SDLK_z:
//			keys[0xA] = 0;
//			break;
//
//		case SDLK_c:
//			keys[0xB] = 0;
//			break;
//
//		case SDLK_4:
//			keys[0xC] = 0;
//			break;
//
//		case SDLK_r:
//			keys[0xD] = 0;
//			break;
//
//		case SDLK_f:
//			keys[0xE] = 0;
//			break;
//
//		case SDLK_v:
//			keys[0xF] = 0;
//			break;
//	}
//}

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
	shader.LoadPixelShader("D:\\Sources\\Chip8-Emulator\\bin\\x64-Debug\\PixelShader.cso");
	shader.LoadVertexShader("D:\\Sources\\Chip8-Emulator\\bin\\x64-Debug\\VertexShader.cso");
	shader.Use();

	// Model
	DX::Model model(&renderer);
	model.Create();

	// Emulation core
	Chip8 chip8;
	//chip8.LoadROM("IBM Logo.ch8");
	//chip8.LoadROM("chip8-test-suite.ch8");
	chip8.LoadROM("breakout.ch8");
	int video_pitch = sizeof(chip8.VideoMemory[0]) * VIDEO_WIDTH; 

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
		model.UpdateTexture(chip8.VideoMemory.data(), video_pitch);
		model.Render();
		renderer.Present();
	}

	return 0;
}
