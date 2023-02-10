#pragma once

#include <cstdint>
#include <random>


const unsigned int KEY_COUNT = 16;
const unsigned int MEMORY_SIZE = 4096;
const unsigned int REGISTER_COUNT = 16;
const unsigned int STACK_LEVELS = 16;
const unsigned int VIDEO_HEIGHT = 32;
const unsigned int VIDEO_WIDTH = 64;

// https://en.wikipedia.org/wiki/CHIP-8

class Chip8
{
public:
	Chip8();
	void LoadROM(char const* filename);
	void Cycle();

	uint8_t keypad[KEY_COUNT]{};
	uint32_t video[VIDEO_WIDTH * VIDEO_HEIGHT]{};

private:
	uint8_t memory[MEMORY_SIZE]{};
	uint8_t registers[REGISTER_COUNT]{};
	uint16_t index{};
	uint16_t pc{};
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint16_t stack[STACK_LEVELS]{};
	uint8_t sp{};
	uint16_t opcode{};

	std::default_random_engine randGen;
	std::uniform_int_distribution<uint16_t> randByte;
};
