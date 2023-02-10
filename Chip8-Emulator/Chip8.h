#pragma once

// https://en.wikipedia.org/wiki/CHIP-8
#include <cstdint>
#include <array>
#include <stack>

const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

class Chip8
{
public:
	Chip8();

	// Load the ROM into memory
	void LoadROM(char const* filename);

	// Cycle through the CPU
	void Cycle();

	// Keypad
	uint8_t keypad[KEY_COUNT] = {};

	// VRAM
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT] = {};

private:

	// RAM
	std::array<uint8_t, MEMORY_SIZE> m_Memory;

	// Registers
	std::array<uint8_t, REGISTER_COUNT> m_Registers;

	// Index register
	uint16_t m_IndexRegister = 0;

	// Program counter
	uint16_t m_ProgramCounter = 0;

	// Stacks
	std::stack<uint16_t> m_Stack;

	// Delay timer
	uint8_t m_DelayTimer = 0;

	// Sound timer
	uint8_t m_SoundTimer = 0;
};
