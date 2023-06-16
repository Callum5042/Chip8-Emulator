using System;
using System.Collections.Generic;
using System.IO;
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

            foreach (var val in Enum.GetValues<Register>())
            {
                Registers[val] = 0;
            }
        }

        // 4kb of memory
        public byte[] Memory { get; private set; } = new byte[4096];

        // 16 8-bit registers (V0 to VF)
        public IDictionary<Register, byte> Registers { get; private set; } = new Dictionary<Register, byte>();

        // 16 bit I register which is used mainly to store memory addresses
        public ushort IndexRegister { get; private set; }

        // Stack is only used to store the return memory address of 16 bytes is fine
        public ushort[] Stack { get; private set; } = new ushort[12];

        // Pointer to where we are on the stack
        public byte StackPointer { get; private set; }

        // Stores where we are pointing to in memory
        public ushort ProgramCounter { get; private set; }

        // Display is 64x32 and monochrome colour
        public byte[] Display { get; private set; } = new byte[DisplayWidth * DisplayHeight];

        public byte SoundTimer { get; set; }

        public byte DelayTimer { get; set; }

        public bool[] Keypad { get; private set; } = new bool[16];

        public void LoadROM(string filepath)
        {
            // The first 512 bytes are used by the system so we copy the rom into memory with a offset of 512 (0x200)
            var fileBytes = File.ReadAllBytes(filepath);
            Array.Copy(fileBytes, 0, Memory, StartAddress, fileBytes.Length);

            // Set the program counter to the start address
            ProgramCounter = StartAddress;
        }

        private List<ushort> RanCodes = new List<ushort>();

        public void Cycle()
        {
            // Fetch instruction. Opcode is 16 bytes so we must current byte and the next byte in memory
            ushort opcode = (ushort)((Memory[ProgramCounter] << 8) | Memory[ProgramCounter + 1]);

            RanCodes.Add(opcode);

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
            else
            {
                throw new NotImplementedException();
            }
        }

        private void MixtureOfThings(ushort opcode)
        {
            var register = (Register)((opcode & 0x0F00) >> 8);
            var op = opcode & 0x00FF;

            if (op == 0x7)
            {
                Registers[register] = DelayTimer;
            }
            else if (op == 0x15)
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
                // throw new NotImplementedException();
            }
            else if (op == 0x33)
            {
                // Store binary-coded decimal representation of x in memory locations I, I + 1, and I+2
                var value = Registers[register];

                Memory[IndexRegister + 2] = (byte)(value % 10);
                value /= 10;

                Memory[IndexRegister + 1] = (byte)(value % 10);
                value /= 10;

                Memory[IndexRegister] = (byte)(value % 10);
            }
            else if (op == 0x55)
            {
                // Stores from register 0 to register X (including register X) in memory, starting at address I
                for (int i = 0; i <= (int)register; ++i)
                {
                    Memory[IndexRegister + i] = Registers[(Register)i];
                }
            }
            else if (op == 0x65)
            {
                // Fills from register 0 to register X (including register X) with values from memory, starting at address I
                for (int i = 0; i <= (int)register; ++i)
                {
                    Registers[(Register)i] = Memory[IndexRegister + i];
                }
            }
            else
            {
                throw new NotImplementedException();
            }
        }

        private void SkipOnKeyState(ushort opcode)
        {
            // Skip on key state (Ex
            var register = (Register)((opcode & 0x0F00) >> 8);
            var op = opcode & 0x00FF;

            if (op == 0x9E)
            {
                if (Keypad[Registers[register]])
                {
                    ProgramCounter += 2;
                }
            }
            else if (op == 0xA1)
            {
                if (!Keypad[Registers[register]])
                {
                    ProgramCounter += 2;
                }
            }
            else
            {
                throw new NotImplementedException();
            }
        }

        private void Draw(ushort opcode)
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

                        Display[(y + row) * DisplayWidth + (x + col)] ^= 0xFF;
                    }
                }
            }
        }

        private void SetRegisterToRandom(ushort opcode)
        {
            // Set the register X to random value & nn (Cxnn)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var value = (ushort)(opcode & 0x00FF);

            var random = new Random();
            var rnd = random.Next(255) & value;
            Registers[registerX] = (byte)rnd;
        }

        private void JumpToAddressPlusRegister(ushort opcode)
        {
            // Jump to memory address NNN (Bnnn)
            var memoryAddress = (ushort)(opcode & 0x0FFF);
            ProgramCounter = (ushort)(memoryAddress + Registers[Register.V0]);
        }

        private void SetIndexRegister(ushort opcode)
        {
            // Sets the I register to the value NNN (Annn)
            var memoryAddress = (ushort)(opcode & 0x0FFF);
            IndexRegister = memoryAddress;
        }

        private void SkipIfRegistersNotEquals(ushort opcode)
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

        private void RegisterOperations(ushort opcode)
        {
            // Get the x and y register (8xyz)
            var registerX = (Register)((opcode & 0x0F00) >> 8);
            var registerY = (Register)((opcode & 0x00F0) >> 4);
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
                // If we have overflowed the byte (255) then we set the register VF to 1

                //var x = Registers[registerX];
                //var y = Registers[registerY];

                //if (Registers[registerX] > Registers[registerY])
                //{
                //    Registers[Register.VF] = 1;
                //}
                //else
                //{
                //    Registers[Register.VF] = 0;
                //}

                //Registers[registerX] = (byte)(x - y);

                if (Registers[registerX] > Registers[registerY])
                {
                    Registers[registerX] = (byte)(Registers[registerX] - Registers[registerY]);
                    Registers[Register.VF] = 1;
                }
                else
                {
                    Registers[registerX] = (byte)(256 + Registers[registerX] - Registers[registerY]);
                    Registers[Register.VF] = 0;
                }
            }
            else if (op == 0x6)
            {
                var shiftedOut = Registers[registerX] & 0x1;

                Registers[registerX] = Registers[registerY];
                Registers[registerX] >>= 1;

                Registers[Register.VF] = (byte)shiftedOut;
            }
            else if (op == 0x7)
            {
                if (Registers[registerY] > Registers[registerX])
                {
                    Registers[registerX] = (byte)(Registers[registerY] - Registers[registerX]);
                    Registers[Register.VF] = 1;
                }
                else
                {
                    Registers[registerX] = (byte)(256 + Registers[registerY] - Registers[registerX]);
                    Registers[Register.VF] = 0;
                }
            }
            else if (op == 0xE)
            {
                var shiftedOut = (Registers[registerY] & 0x80) >> 7;

                Registers[registerX] = Registers[registerY];
                Registers[registerX] <<= 1;

                Registers[Register.VF] = (byte)shiftedOut;
            }
            else
            {
                throw new NotImplementedException();
            }
        }

        private void AddAndAssign(ushort opcode)
        {
            // Adds NN to the register X (0x7xnn)
            var register = (Register)((opcode & 0x0F00) >> 8);
            var value = (byte)(opcode & 0x00FF);

            Registers[register] += value;
        }

        private void SetRegister(ushort opcode)
        {
            // Sets the register X to the value NN (0x6xnn)
            var register = (Register)((opcode & 0x0F00) >> 8);
            var value = (byte)(opcode & 0x00FF);

            Registers[register] = value;
        }

        private void SkipIfRegistersEquals(ushort opcode)
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

        private void SkipIfNotEquals(ushort opcode)
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

        private void SkipIfEquals(ushort opcode)
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

        private void CallFunction(ushort opcode)
        {
            // Gets the first 12 bits of the opcode (0x2nnn)
            var memoryAddress = (ushort)(opcode & 0x0FFF);

            Stack[StackPointer] = ProgramCounter;
            StackPointer++;

            ProgramCounter = memoryAddress;
        }

        private void JumpToAddress(ushort opcode)
        {
            // Gets the first 12 bits of the opcode (0x1nnn)
            var memoryAddress = (ushort)(opcode & 0x0FFF);
            ProgramCounter = memoryAddress;
        }

        private void ReturnFromFunction()
        {
            // Pop the stack so set the program counter to point to the memory address set in the stack
            StackPointer--;
            ProgramCounter = Stack[StackPointer];
        }

        private void ClearScreen()
        {
            Array.Clear(Display);
        }
    }
}
