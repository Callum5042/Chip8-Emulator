#include "Chip8.h"
#include <chrono>
#include <iostream>
#include <string>
#include <SDL.h>
#include "Renderer.h"
#include "Shader.h"
#include "Model.h"

void KeyDown(SDL_Keycode keycode, uint8_t* keys)
{
	switch (keycode)
	{
		case SDLK_x:
			keys[0] = 1;
			break;

		case SDLK_1:
			keys[1] = 1;
			break;

		case SDLK_2:
			keys[2] = 1;
			break;

		case SDLK_3:
			keys[3] = 1;
			break;

		case SDLK_q:
			keys[4] = 1;
			break;

		case SDLK_w:
			keys[5] = 1;
			break;

		case SDLK_e:
			keys[6] = 1;
			break;

		case SDLK_a:
			keys[7] = 1;
			break;

		case SDLK_s:
			keys[8] = 1;
			break;

		case SDLK_d:
			keys[9] = 1;
			break;

		case SDLK_z:
			keys[0xA] = 1;
			break;

		case SDLK_c:
			keys[0xB] = 1;
			break;

		case SDLK_4:
			keys[0xC] = 1;
			break;

		case SDLK_r:
			keys[0xD] = 1;
			break;

		case SDLK_f:
			keys[0xE] = 1;
			break;

		case SDLK_v:
			keys[0xF] = 1;
			break;
	}
}

void KeyUp(SDL_Keycode keycode, uint8_t* keys)
{
	switch (keycode)
	{
		case SDLK_x:
			keys[0] = 0;
			break;

		case SDLK_1:
			keys[1] = 0;
			break;

		case SDLK_2:
			keys[2] = 0;
			break;

		case SDLK_3:
			keys[3] = 0;
			break;

		case SDLK_q:
			keys[4] = 0;
			break;

		case SDLK_w:
			keys[5] = 0;
			break;

		case SDLK_e:
			keys[6] = 0;
			break;

		case SDLK_a:
			keys[7] = 0;
			break;

		case SDLK_s:
			keys[8] = 0;
			break;

		case SDLK_d:
			keys[9] = 0;
			break;

		case SDLK_z:
			keys[0xA] = 0;
			break;

		case SDLK_c:
			keys[0xB] = 0;
			break;

		case SDLK_4:
			keys[0xC] = 0;
			break;

		case SDLK_r:
			keys[0xD] = 0;
			break;

		case SDLK_f:
			keys[0xE] = 0;
			break;

		case SDLK_v:
			keys[0xF] = 0;
			break;
	}
}

int main(int argc, char** argv)
{
	// Initalise SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "SDL_Init failed", nullptr);
		return -1;
	}

	int video_scale = 10;
	int width = VIDEO_WIDTH * video_scale;
	int height = VIDEO_HEIGHT * video_scale;

	// Create window
	SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == nullptr)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "SDL_CreateWindow failed", nullptr);
		return -1;
	}

	// Create renderer
	DX::Renderer renderer(window);
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
	chip8.LoadROM("test_opcode.ch8");
	int video_pitch = sizeof(chip8.video[0]) * VIDEO_WIDTH;

	// Message loop
	bool quit = false;
	while (!quit)
	{
		SDL_Event event = {};
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;

				case SDL_KEYDOWN:
					KeyDown(event.key.keysym.sym, chip8.keypad);
					break;

				case SDL_KEYUP:
					KeyUp(event.key.keysym.sym, chip8.keypad);
					break;

				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_RESIZED)
					{
						renderer.Resize(event.window.data1, event.window.data2);
					}
					break;
			}
		}

		// Execute instructions
		chip8.Cycle();

		// Update
		renderer.Clear();

		model.UpdateTexture(chip8.video, video_pitch);
		model.Render();

		renderer.Present();
	}

	return 0;
}
