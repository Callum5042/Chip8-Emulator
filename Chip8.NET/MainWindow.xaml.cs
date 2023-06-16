using System;
using System.Collections.Generic;
using System.Linq;
using System.Media;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Media3D;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Chip8.NET
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private Chip8Emulator? Emulator { get; set; }

        public MainWindow()
        {
            InitializeComponent();

            // Emulator = new Chip8Emulator("ROMS/IBM Logo.ch8");
            Emulator = new Chip8Emulator("ROMS/breakout.ch8");

            // Emulator = new Chip8Emulator("ROMS/1-chip8-logo.ch8");
            Emulator = new Chip8Emulator("ROMS/4-flags.ch8");

            CompositionTarget.Rendering += MainLoop;
        }

        private void MainLoop(object? sender, EventArgs e)
        {
            if (Emulator is not null)
            {
                // Pause when delay timer is active
                if (Emulator.DelayTimer > 0)
                {
                    Emulator.DelayTimer -= 1;
                    // return;
                }

                // Play sound
                if (Emulator.SoundTimer > 0)
                {
                    Emulator.SoundTimer -= 1;
                    SystemSounds.Beep.Play();
                }

                // Set keypad states
                Emulator.Keypad[0x0] = Keyboard.IsKeyDown(Key.X);
                Emulator.Keypad[0x1] = Keyboard.IsKeyDown(Key.NumPad1);
                Emulator.Keypad[0x2] = Keyboard.IsKeyDown(Key.NumPad2);
                Emulator.Keypad[0x3] = Keyboard.IsKeyDown(Key.NumPad3);
                Emulator.Keypad[0x4] = Keyboard.IsKeyDown(Key.Q);
                Emulator.Keypad[0x5] = Keyboard.IsKeyDown(Key.W);
                Emulator.Keypad[0x6] = Keyboard.IsKeyDown(Key.E);
                Emulator.Keypad[0x7] = Keyboard.IsKeyDown(Key.A);
                Emulator.Keypad[0x8] = Keyboard.IsKeyDown(Key.S);
                Emulator.Keypad[0x9] = Keyboard.IsKeyDown(Key.D);
                Emulator.Keypad[0xA] = Keyboard.IsKeyDown(Key.Z);
                Emulator.Keypad[0xB] = Keyboard.IsKeyDown(Key.C);
                Emulator.Keypad[0xC] = Keyboard.IsKeyDown(Key.NumPad4);
                Emulator.Keypad[0xD] = Keyboard.IsKeyDown(Key.R);
                Emulator.Keypad[0xE] = Keyboard.IsKeyDown(Key.F);

                // Cycle CPU instructions
                Emulator.Cycle();

                // Render
                image.Source = BitmapSource.Create(Chip8Emulator.DisplayWidth, Chip8Emulator.DisplayHeight, 1, 1, PixelFormats.Indexed8, BitmapPalettes.Gray256, Emulator.Display, Chip8Emulator.DisplayWidth);
            }
        }

        private void MenuItem_Load_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
