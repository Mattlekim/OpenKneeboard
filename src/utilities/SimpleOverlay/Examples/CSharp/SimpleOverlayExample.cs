using System;
using System.Threading;
using OpenKneeboard.SimpleOverlay;

namespace SimpleOverlayExample
{
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("SimpleOverlay C# Example");
            Console.WriteLine("========================\n");

            try
            {
                // Example 1: Basic overlay creation and display
                BasicExample();

                // Example 2: Overlay with custom configuration
                CustomConfigExample();

                // Example 3: Animated overlay
                AnimatedExample();

                // Example 4: Texture rendering
                TextureExample();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Error: {ex.Message}");
            }

            Console.WriteLine("\nPress any key to exit...");
            Console.ReadKey();
        }

        static void BasicExample()
        {
            Console.WriteLine("Example 1: Basic Overlay");
            Console.WriteLine("-------------------------");

            // Create a simple 800x600 overlay
            using (var overlay = new SimpleOverlay(800, 600))
            {
                Console.WriteLine("Overlay created successfully!");
                Console.WriteLine($"Is Valid: {overlay.IsValid}");

                // Show the overlay
                overlay.Show();
                Console.WriteLine("Overlay is now visible.");

                // Keep it visible for 2 seconds
                Thread.Sleep(2000);

                // Hide the overlay
                overlay.Hide();
                Console.WriteLine("Overlay hidden.\n");
            }
        }

        static void CustomConfigExample()
        {
            Console.WriteLine("Example 2: Custom Configuration");
            Console.WriteLine("--------------------------------");

            // Create overlay with custom configuration
            var config = new SimpleOverlayConfig(
                width: 1024,
                height: 768,
                x: 100,
                y: 100,
                opacity: 0.8f,
                enableVSync: true
            );

            using (var overlay = new SimpleOverlay(config))
            {
                Console.WriteLine($"Custom overlay created at position (100, 100)");
                Console.WriteLine($"Size: 1024x768, Opacity: 0.8");

                overlay.Show();
                Thread.Sleep(2000);

                Console.WriteLine("Overlay hidden.\n");
            }
        }

        static void AnimatedExample()
        {
            Console.WriteLine("Example 3: Animated Overlay");
            Console.WriteLine("---------------------------");

            using (var overlay = new SimpleOverlay(640, 480))
            {
                overlay.Show();
                Console.WriteLine("Animating overlay position and opacity...");

                // Animate position
                for (int i = 0; i < 10; i++)
                {
                    int x = i * 50;
                    int y = i * 30;
                    overlay.SetPosition(x, y);
                    Console.WriteLine($"Position: ({x}, {y})");
                    Thread.Sleep(200);
                }

                // Animate opacity
                for (int i = 10; i >= 0; i--)
                {
                    float opacity = i / 10.0f;
                    overlay.SetOpacity(opacity);
                    Console.WriteLine($"Opacity: {opacity:F2}");
                    Thread.Sleep(200);
                }

                Console.WriteLine("Animation complete.\n");
            }
        }

        static void TextureExample()
        {
            Console.WriteLine("Example 4: Texture Rendering");
            Console.WriteLine("----------------------------");

            const int width = 512;
            const int height = 512;

            using (var overlay = new SimpleOverlay(width, height))
            {
                overlay.Show();

                // Create a simple gradient texture (RGBA format)
                byte[] textureData = new byte[width * height * 4];
                for (int y = 0; y < height; y++)
                {
                    for (int x = 0; x < width; x++)
                    {
                        int index = (y * width + x) * 4;
                        textureData[index + 0] = (byte)(x * 255 / width);     // R
                        textureData[index + 1] = (byte)(y * 255 / height);    // G
                        textureData[index + 2] = 128;                          // B
                        textureData[index + 3] = 255;                          // A
                    }
                }

                Console.WriteLine("Setting gradient texture...");
                overlay.SetTexture(textureData, width, height);

                // Render frames
                Console.WriteLine("Rendering frames...");
                for (int i = 0; i < 60; i++)
                {
                    overlay.Render();
                    Thread.Sleep(16); // ~60 FPS
                }

                Console.WriteLine("Texture rendering complete.\n");
            }
        }
    }
}
