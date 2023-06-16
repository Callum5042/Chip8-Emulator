using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Input;

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
            else if (decode == 0x6)
            {
                SetRegister(opcode);
            }
            else if (decode == 0x7)
            {
                AddAndAssign(opcode);
            }
            else if (decode == 0x8)
            {
                RegisterOperations(opcode);
            }
            else if (decode == 0x9)
            {
                SkipIfRegistersNotEquals(opcode);
            }
            else if (decode == 0xA)
            {
                SetIndexRegister(opcode);
            }
            else if (decode == 0xB)
            {
                JumpToAddressPlusRegister(opcode);
            }
            else if (decode == 0xC)
            {
                SetRegisterToRandom(opcode);
            }
            else if (decode == 0xD)
            {
                Draw(opcode);
            }
            else if (decode == 0xE)
            {
                SkipOnKeyState(opcode);
            }
            else if (decode == 0xF)
            {
                MixtureOfThings(opcode);
            }
        }

        private void MixtureOfThings(short opcode)
        {
            var register = (Register)(opcode & 0x0F00);
            var op = opcode & 0x00FF;

            if (op == 0x15)
            {
                DelayTimer = Registers[register];
            }
            else if (op == 0x18)
            {
                SoundTimer = Registers[register];
            }
            else if (op == 0x1E)
            {
                IndexRegister += Registers[register];
            }
            else if (op == 0x29)
            {
                throw new NotImplementedException();
            }
            else if (op == 0x33)
            {
                throw new NotImplementedException();
            }
            else if (op == 0x55)
            {
                throw new NotImplementedException();
            }
            else if (op == 0x65)
            {
                throw new NotImplementedException();
            }
        }

        private void SkipOnKeyState(short opcode)
        {
            // Skip on key state (Ex
            var register = (Register)(opcode & 0x0F00);
            var key = (Key)Registers[register];
            var op = opcode & 0x00FF;

            if (op == 0x9E)
            {
                if (Keyboard.IsKeyDown(key))
                {
                    ProgramCounter += 2;
                }
            }
            else if (op == 0xA1)
            {
                if (!Keyboard.IsKeyDown(key))
                {
                    ProgramCounter += 2;
                }
            }
        }

        private void Draw(short opcode)
        {
            // Draw sprite at coordinate register x and y with a fixed width of 8 pixels and a height of n pixels (Dxyn)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var registerY = (Register)((opcode & 0x00F0) >> 4);
            var height = opcode & 0x00F;
            const int SpriteWidth = 8;

            // Wrap if going beyond screen boundaries
            byte x = (byte)(Registers[registerX] % DisplayWidth);
            byte y = (byte)(Registers[registerY] % DisplayHeight);

            for (int row = 0; row < height; row++)
            {
                byte spriteData = Memory[IndexRegister + row];

                for (int col = 0; col < SpriteWidth; col++)
                {
                    byte spritePixel = (byte)(spriteData & (0x80 >> col));
                    byte screenPixel = Display[(y + row) * DisplayWidth + (x + col)];

                    if (spritePixel != 0)
                    {
                        if (screenPixel == 0xFF)
                        {
                            Registers[Register.VF] = 1;
                        }

                        Display[(y + row) * DisplayWidth + (x + col)] = 0xFF;
                    }
                }
            }
        }

        private void SetRegisterToRandom(short opcode)
        {
            // Set the register X to random value & nn (Cxnn)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var value = (short)(opcode & 0x00FF);

            var random = new Random();
            var rnd = random.Next(255) & value;
            Registers[registerX] = (byte)rnd;
        }

        private void JumpToAddressPlusRegister(short opcode)
        {
            // Jump to memory address NNN (Bnnn)
            var memoryAddress = (short)(opcode & 0x0FFF);
            ProgramCounter = (short)(memoryAddress + Registers[Register.V0]);
        }

        private void SetIndexRegister(short opcode)
        {
            // Sets the I register to the value NNN (Annn)
            var memoryAddress = (short)(opcode & 0x0FFF);
            IndexRegister = memoryAddress;
        }

        private void SkipIfRegistersNotEquals(short opcode)
        {
            // Gets the register X and register Y (9xy0)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var registerY = (Register)((opcode & 0x00F0) >> 4);

            if (Registers[registerX] != Registers[registerY])
            {
                // Skip next instruction. Opcode is 16 bytes so we increase by 2
                ProgramCounter += 2;
            }
        }

        private void RegisterOperations(short opcode)
        {
            // Get the x and y register (8xyz)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var registerY = (Register)((opcode & 0x0F00) >> 4);
            var op = opcode & 0x000F;

            if (op == 0x0)
            {
                Registers[registerX] = Registers[registerY];
            }
            else if (op == 0x1)
            {
                Registers[registerX] |= Registers[registerY];
            }
            else if (op == 0x2)
            {
                Registers[registerX] &= Registers[registerY];
            }
            else if (op == 0x3)
            {
                Registers[registerX] ^= Registers[registerY];
            }
            else if (op == 0x4)
            {
                var sum = Registers[registerX] + Registers[registerY];
                Registers[registerX] = (byte)sum;

                // If we have overflowed the byte (255) then we set the register VF to 1
                Registers[Register.VF] = (byte)(sum > byte.MaxValue ? 1 : 0);
            }
            else if (op == 0x5)
            {
                var sum = Registers[registerX] - Registers[registerY];
                Registers[registerX] = (byte)sum;

                // If we have underflowed the byte (255) then we set the register VF to 1
                Registers[Register.VF] = (byte)(sum < byte.MinValue ? 1 : 0);
            }
            else if (op == 0x6)
            {
                Registers[registerX] >>= Registers[registerY];
            }
            else if (op == 0x7)
            {
                Registers[registerX] = (byte)(Registers[registerY] - Registers[registerX]);
            }
            else if (op == 0xE)
            {
                Registers[registerX] <<= Registers[registerY];
            }
        }

        private void AddAndAssign(short opcode)
        {
            // Adds NN to the register X (0x7xnn)
            var register = (Register)((opcode & 0x0F00) >> 8);
            var value = (byte)(opcode & 0x00FF);

            Registers[register] += value;
        }

        private void SetRegister(short opcode)
        {
            // Sets the register X to the value NN (0x6xnn)
            var register = (Register)((opcode & 0x0F00) >> 8);
            var value = (byte)(opcode & 0x00FF);

            Registers[register] = value;
        }

        private void SkipIfRegistersEquals(short opcode)
        {
            // Gets the register X and register Y (5xy0)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var registerY = (Register)((opcode & 0x00F0) >> 4);

            if (Registers[registerX] == Registers[registerY])
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
