# SimpleOverlay

A lightweight overlay library for Windows with C# bindings.

## Overview

SimpleOverlay provides a simple API for creating overlay windows with Direct3D 11 rendering. It includes:

- Native C/C++ library built as a DLL
- C# wrapper with P/Invoke bindings
- Full example applications

## Building

### Native Library (C++)

```bash
cmake -B build
cmake --build build --config Release
```

This will produce `SimpleOverlay.dll` in the `build/bin` directory.

### C# Library

```bash
cd src/utilities/SimpleOverlay/SimpleOverlay.CSharp
dotnet build
```

## Usage

### C# Example

```csharp
using OpenKneeboard.SimpleOverlay;

// Create an overlay
var config = new SimpleOverlayConfig(
    width: 800,
    height: 600,
    x: 0,
    y: 0,
    opacity: 1.0f,
    enableVSync: true
);

using (var overlay = new SimpleOverlay(config))
{
    // Show the overlay
    overlay.Show();
    
    // Update position
    overlay.SetPosition(100, 100);
    
    // Update opacity
    overlay.SetOpacity(0.8f);
    
    // Set texture data (RGBA format)
    byte[] textureData = new byte[800 * 600 * 4];
    // ... fill texture data ...
    overlay.SetTexture(textureData, 800, 600);
    
    // Render a frame
    overlay.Render();
    
    // Hide the overlay
    overlay.Hide();
}
```

### Running Examples

```bash
cd src/utilities/SimpleOverlay/Examples/CSharp
dotnet run
```

Make sure `SimpleOverlay.dll` is in the same directory as the executable or in your PATH.

## API Reference

### SimpleOverlay Class

#### Constructor
- `SimpleOverlay(SimpleOverlayConfig config)` - Creates overlay with custom configuration
- `SimpleOverlay(int width, int height)` - Creates overlay with default settings

#### Methods
- `void Show()` - Shows the overlay window
- `void Hide()` - Hides the overlay window
- `void SetPosition(int x, int y)` - Updates overlay position
- `void SetOpacity(float opacity)` - Updates overlay opacity (0.0 to 1.0)
- `void Render()` - Renders a frame
- `void SetTexture(byte[] textureData, int width, int height)` - Sets RGBA texture data

#### Properties
- `bool IsValid` - Returns true if the overlay is valid and not disposed

### SimpleOverlayConfig Struct

- `int Width` - Overlay width in pixels
- `int Height` - Overlay height in pixels
- `int X` - Initial X position
- `int Y` - Initial Y position
- `float Opacity` - Initial opacity (0.0 to 1.0)
- `int EnableVSync` - Enable VSync (1 = enabled, 0 = disabled)

## Requirements

- Windows 10 or later
- .NET 6.0 or later (for C# bindings)
- Direct3D 11 compatible GPU

## License

See the main OpenKneeboard LICENSE file for licensing information.
