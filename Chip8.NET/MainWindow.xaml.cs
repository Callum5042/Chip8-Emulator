﻿using System;
using System.Collections.Generic;
using System.Linq;
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

            Emulator = new Chip8Emulator("ROMS/IBM Logo.ch8");
            CompositionTarget.Rendering += UpdateTexture;
        }

        private void UpdateTexture(object? sender, EventArgs e)
        {
            if (Emulator is not null)
            {
                Emulator.Cycle();

                // Render
                var bitmapSource = BitmapSource.Create(1, 1, Chip8Emulator.DisplayWidth, Chip8Emulator.DisplayHeight, PixelFormats.Indexed8, BitmapPalettes.Gray256, Emulator.Display, 2);
                image.Source = bitmapSource;
            }
        }

        private void MenuItem_Load_Click(object sender, RoutedEventArgs e)
        {

        }
    }
}
