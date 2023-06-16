using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;

namespace Chip8.NET
{
    public enum Register
    {
        V0,
        V1,
        V2,
        V3,
        V4,
        V5,
        V6,
        V7,
        V8,
        V9,
        VA,
        VB,
        VC,
        VD,
        VE,
        VF
    }

    public class Chip8Emulator
    {
        private const int StartAddress = 0x200;
        public const int DisplayWidth = 64;
        public const int DisplayHeight = 32;

        public Chip8Emulator(string filepath)
        {
            LoadROM(filepath);
        }

        // 4kb of memory
        public byte[] Memory { get; private set; } = new byte[4096];

        // 16 8-bit registers (V0 to VF)
        public IDictionary<Register, byte> Registers { get; private set; } = new Dictionary<Register, byte>();

        // 16 bit I register which is used mainly to store memory addresses
        public short IndexRegister { get; private set; }

        // Stack is only used to store the return memory address of 16 bytes is fine
        public short[] Stack { get; private set; } = new short[12];

        // Pointer to where we are on the stack
        public byte StackPointer { get; private set; }

        // Stores where we are pointing to in memory
        public short ProgramCounter { get; private set; }

        // Display is 64x32 and monochrome colour
        public byte[] Display { get; private set; } = new byte[DisplayWidth * DisplayHeight];

        public byte SoundTimer { get; private set; }

        public byte DelayTimer { get; private set; }

        public void LoadROM(string filepath)
        {
            // The first 512 bytes are used by the system so we copy the rom into memory with a offset of 512 (0x200)
            var fileBytes = File.ReadAllBytes(filepath);
            Array.Copy(fileBytes, 0, Memory, StartAddress, fileBytes.Length);

            // Set the program counter to the start address
            ProgramCounter = StartAddress;
        }

        public void Cycle()
        {
            // Fetch instruction. Opcode is 16 bytes so we must current byte and the next byte in memory
            short opcode = (short)((Memory[ProgramCounter] << 8) | Memory[ProgramCounter + 1]);

            // Increment the program counter before we execute anything
            ProgramCounter += 2;

            // Take first 8 bits
            byte decode = (byte)((opcode & 0xF000) >> 12);

            // Decode & Execute
            if (decode == 0x0)
            {
                if (opcode == 0x00E0)
                {
                    ClearScreen();
                }
                else if (opcode == 0x00EE)
                {
                    ReturnFromFunction();
                }
            }
            else if (decode == 0x1)
            {
                JumpToAddress(opcode);
            }
            else if (decode == 0x2)
            {
                CallFunction(opcode);
            }
            else if (decode == 0x3)
            {
                SkipIfEquals(opcode);
            }
            else if (decode == 0x4)
            {
                SkipIfNotEquals(opcode);
            }
            else if (decode == 0x5)
            {
                SkipIfRegistersEquals(opcode);
            }
        }

        private void SkipIfRegistersEquals(short opcode)
        {
            // Gets the register X and register Y (5xy0)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var registerY = (Register)((opcode & 0x00F0) >> 4);

            if (Registers[registerX] != Registers[registerY])
            {
                // Skip next instruction. Opcode is 16 bytes so we increase by 2
                ProgramCounter += 2;
            }
        }

        private void SkipIfNotEquals(short opcode)
        {
            // Pull out register and value from opcode (4xnn)
            var register = (Register)((opcode & 0x0F00) >> 8);
            var value = opcode & 0x00FF;

            if (Registers[register] != value)
            {
                // Skip next instruction. Opcode is 16 bytes so we increase by 2
                ProgramCounter += 2;
            }
        }

        private void SkipIfEquals(short opcode)
        {
            // Pull out register and value from opcode (3xnn)
            var register = (Register)((opcode & 0x0F00) >> 8);
            var value = opcode & 0x00FF;

            if (Registers[register] == value)
            {
                // Skip next instruction. Opcode is 16 bytes so we increase by 2
                ProgramCounter += 2;
            }
        }

        private void CallFunction(short opcode)
        {
            // Gets the first 12 bits of the opcode (0x2nnn)
            var memoryAddress = (short)(opcode & 0x0FFF);

            Stack[StackPointer] = memoryAddress;
            StackPointer++;
        }

        private void JumpToAddress(short opcode)
        {
            // Gets the first 12 bits of the opcode (0x1nnn)
            var memoryAddress = (short)(opcode & 0x0FFF);
            ProgramCounter = memoryAddress;
        }

        private void ReturnFromFunction()
        {
            // Pop the stack so set the program counter to point to the memory address set in the stack
            ProgramCounter = Stack[StackPointer];
            StackPointer--;
        }

        private void ClearScreen()
        {
            Array.Clear(Display);
        }
    }
}
