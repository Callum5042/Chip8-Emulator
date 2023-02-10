#include "Chip8.h"
#include <fstream>
#include <chrono>
#include <random>

namespace
{
	const unsigned int FONTSET_SIZE = 80;
	const unsigned int FONTSET_START_ADDRESS = 0x50;
	const unsigned int START_ADDRESS = 0x200;

	uint8_t fontset[FONTSET_SIZE] =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
}

Chip8::Chip8()
{
	// Initialize PC
	pc = START_ADDRESS;

	// Load fonts into memory
	for (unsigned int i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}
}

void Chip8::LoadROM(char const* filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		std::streampos size = file.tellg();
		char* buffer = new char[size];
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		for (long i = 0; i < size; ++i)
		{
			memory[START_ADDRESS + i] = buffer[i];
		}

		delete[] buffer;
	}
}

void Chip8::Cycle()
{
	// Fetch (opcode is 16 bits so we must read the current program counter and the next program counter)
	opcode = (memory[pc] << 8) | memory[pc + 1];

	// Increment the program counter before we execute anything
	pc += 2;

	// Decode and execute
	uint32_t decode = (opcode & 0xF000) >> 12;

	if (decode == 0x0)
	{
		if (opcode == 0x00E0) // 00E0
		{
			// Clears the screen
			std::memset(video, 0, sizeof(video));
		}
		else if (opcode == 0x00EE) // 00EE
		{
			// Returns from a subroutine (pop the stack)
			--sp;
			pc = stack[sp];
		}
	}
	else if (decode == 0x1) // 1NNN
	{
		// Jumps to address NNN
		pc = opcode & 0x0FFF;
	}
	else if (decode == 0x2) // 2NNN
	{
		// Calls subroutine at NNN (push the stack)
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
	}
	else if (decode == 0x3) // 3XNN
	{
		// Skips the next instruction if VX equals NN (usually the next instruction is a jump to skip a code block)
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t byte = opcode & 0x00FF;

		if (registers[vx_register] == byte)
		{
			pc += 2;
		}
	}
	else if (decode == 0x4) // 4XNN
	{
		// Skips the next instruction if VX does not equal NN (usually the next instruction is a jump to skip a code block)
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t byte = opcode & 0x00FF;

		if (registers[vx_register] != byte)
		{
			pc += 2;
		}
	}
	else if (decode == 0x5) // 5XY0
	{
		// Skips the next instruction if VX equals VY (usually the next instruction is a jump to skip a code block). 
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t vy_register = (opcode & 0x00F0) >> 4;

		if (registers[vx_register] == registers[vy_register])
		{
			pc += 2;
		}
	}
	else if (decode == 0x6) // 6XNN
	{
		// Sets VX register to NN
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t byte = opcode & 0x00FF;

		registers[vx_register] = byte;
	}
	else if (decode == 0x7) // 7XNN
	{
		// Adds NN to VX (carry flag is not changed)
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t byte = opcode & 0x00FF;

		registers[vx_register] += byte;
	}
	else if (decode == 0x8) // 8
	{
		uint32_t val = opcode & 0x000F;

		if (val == 0x0) // 8XY0
		{
			// Sets VX to the value of VY
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] = registers[vy_register];
		}
		else if (val == 0x1) // 8XY1
		{
			// Sets VX to VX or VY. (bitwise OR operation) 
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] |= registers[vy_register];
		}
		else if (val == 0x2) // 8XY2
		{
			// Sets VX to VX and VY. (bitwise AND operation) 
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] &= registers[vy_register];
		}
		else if (val == 0x3) // 8XY3
		{
			// Sets VX to VX xor VY
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] ^= registers[vy_register];
		}
		else if (val == 0x4) // 8XY4
		{
			// Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there is not
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] += registers[vy_register];
		}
		else if (val == 0x5) // 8XY5
		{
			// VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there is not
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] -= registers[vy_register];
		}
		else if (val == 0x6) // 8XY6
		{
			// Stores the least significant bit of VX in VF and then shifts VX to the right by 1
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] >>= 1;
		}
		else if (val == 0x7) // 8XY7
		{
			// Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] = registers[vy_register] - registers[vx_register];
		}
		else if (val == 0xE) // 8XYE
		{
			// Stores the most significant bit of VX in VF and then shifts VX to the left by 1
			uint8_t vx_register = (opcode & 0x0F00) >> 8;
			uint8_t vy_register = (opcode & 0x00F0) >> 4;

			registers[vx_register] <<= 1;
		}
	}
	else if (decode == 0x9) // 9XY0
	{
		// Skips the next instruction if VX does not equal VY. (Usually the next instruction is a jump to skip a code block)
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t vy_register = (opcode & 0x00F0) >> 4;

		if (registers[vx_register] != registers[vy_register])
		{
			pc += 2;
		}
	}
	else if (decode == 0xA) // ANNN
	{
		// Sets I to the address NNN
		index = opcode & 0x0FFF;
	}
	else if (decode == 0xB) // BNNN
	{
		// Jumps to the address NNN plus V0
		pc = (opcode & 0x0FFF) + registers[0];
	}
	else if (decode == 0xC) /// CXNN
	{
		// Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN
		uint8_t vx_register = (opcode & 0x0F00) >> 8;
		uint8_t byte = opcode & 0x00FFu;

		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> dist(0, 255);

		registers[vx_register] = dist(rng) & byte;
	}
	else if (decode == 0xD) // DXYN
	{
		// Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
		// Each row of 8 pixels is read as bit-coded starting from memory location I; I value does not change after the execution of this instruction. 
		// As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
		uint8_t vx = (opcode & 0x0F00) >> 8;
		uint8_t vy = (opcode & 0x00F0) >> 4;
		uint8_t height = opcode & 0x000F;

		// Wrap if going beyond screen boundaries
		uint8_t xPos = registers[vx] % VIDEO_WIDTH;
		uint8_t yPos = registers[vy] % VIDEO_HEIGHT;

		registers[0xF] = 0;

		for (unsigned row = 0; row < height; ++row)
		{
			uint8_t spriteByte = memory[index + row];

			for (unsigned col = 0; col < 8; ++col)
			{
				uint8_t sprite_pixel = spriteByte & (0x80 >> col);
				uint32_t* screen_pixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

				// Sprite pixel is on
				if (sprite_pixel)
				{
					// Screen pixel also on - collision
					if (*screen_pixel == 0xFFFFFFFF)
					{
						registers[0xF] = 1;
					}

					// Effectively XOR with the sprite pixel
					*screen_pixel ^= 0xFFFFFFFF;
				}
			}
		}
	}
	else if (decode == 0xE)
	{
		uint32_t val = (opcode & 0x0F00) >> 8;
		if (val == 0x9E) // EX9E
		{
			// Skips the next instruction if the key stored in VX is pressed (usually the next instruction is a jump to skip a code block)
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			uint8_t key = registers[register_vx];

			if (keypad[key])
			{
				pc += 2;
			}
		}
		else if (val == 0xA1) // EX9E
		{
			// Skips the next instruction if the key stored in VX is not pressed (usually the next instruction is a jump to skip a code block)
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			uint8_t key = registers[register_vx];

			if (!keypad[key])
			{
				pc += 2;
			}
		}
	}
	else if (decode == 0xF)
	{
		uint32_t val = (opcode & 0x00FF);

		if (val == 0x7) // FX07
		{
			// Sets VX to the value of the delay timer
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			registers[register_vx] = delayTimer;
		}
		else if (val == 0x0A) // FX0A
		{
			// A key press is awaited, and then stored in VX (blocking operation, all instruction halted until next key event)
			uint8_t register_vx = (opcode & 0x0F00u) >> 8u;

			if (keypad[0])
			{
				registers[register_vx] = 0;
			}
			else if (keypad[1])
			{
				registers[register_vx] = 1;
			}
			else if (keypad[2])
			{
				registers[register_vx] = 2;
			}
			else if (keypad[3])
			{
				registers[register_vx] = 3;
			}
			else if (keypad[4])
			{
				registers[register_vx] = 4;
			}
			else if (keypad[5])
			{
				registers[register_vx] = 5;
			}
			else if (keypad[6])
			{
				registers[register_vx] = 6;
			}
			else if (keypad[7])
			{
				registers[register_vx] = 7;
			}
			else if (keypad[8])
			{
				registers[register_vx] = 8;
			}
			else if (keypad[9])
			{
				registers[register_vx] = 9;
			}
			else if (keypad[10])
			{
				registers[register_vx] = 10;
			}
			else if (keypad[11])
			{
				registers[register_vx] = 11;
			}
			else if (keypad[12])
			{
				registers[register_vx] = 12;
			}
			else if (keypad[13])
			{
				registers[register_vx] = 13;
			}
			else if (keypad[14])
			{
				registers[register_vx] = 14;
			}
			else if (keypad[15])
			{
				registers[register_vx] = 15;
			}
			else
			{
				pc -= 2;
			}
		}
		else if (val == 0x15) // FX15
		{
			// Sets the delay timer to VX
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			delayTimer = registers[register_vx];
		}
		else if (val == 0x18) // FX18
		{
			// Sets the sound timer to VX
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			soundTimer = registers[register_vx];
		}
		else if (val == 0x1E) // FX1E
		{
			// Adds VX to I. VF is not affected
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			index += registers[register_vx];
		}
		else if (val == 0x29) // FX29
		{
			// Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			uint8_t register_vx = (opcode & 0x0F00u) >> 8u;
			uint8_t digit = registers[register_vx];

			index = FONTSET_START_ADDRESS + (5 * digit);
		}
		else if (val == 0x33) // FX33
		{
			// Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2
			uint8_t register_vx = (opcode & 0x0F00) >> 8;
			uint8_t value = registers[register_vx];

			// Ones-place
			memory[index + 2] = value % 10;
			value /= 10;

			// Tens-place
			memory[index + 1] = value % 10;
			value /= 10;

			// Hundreds-place
			memory[index] = value % 10;
		}
		else if (val == 0x55) // FX55
		{
			// Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified
			uint8_t Vx = (opcode & 0x0F00) >> 8;

			for (uint8_t i = 0; i <= Vx; ++i)
			{
				memory[index + i] = registers[i];
			}
		}
		else if (val == 0x65) // FX65
		{
			// Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset from I is increased by 1 for each value read, but I itself is left unmodified
			uint8_t Vx = (opcode & 0x0F00) >> 8;

			for (uint8_t i = 0; i <= Vx; ++i)
			{
				registers[i] = memory[index + i];
			}
		}
	}

	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}
