# SimpleOverlay

A lightweight, high-performance overlay library for Windows with Direct3D 11 rendering and complete C# bindings.

## Overview

SimpleOverlay provides a simple yet powerful API for creating overlay windows with Direct3D 11 rendering. It includes:

- **Native C/C++ library** - Built as a DLL with full D3D11 implementation
- **C# wrapper** - Complete P/Invoke bindings with managed wrapper
- **Thread-safe operation** - Protected with mutexes for concurrent access
- **RAII resource management** - Proper cleanup with COM smart pointers
- **Example applications** - Ready-to-run demos

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Application Layer                        │
│  (C# Example App or OpenKneeboard Integration)              │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      │ P/Invoke
                      │
┌─────────────────────▼───────────────────────────────────────┐
│              C# Wrapper (Managed)                            │
│  • SimpleOverlay class                                       │
│  • SimpleOverlayConfig struct                                │
│  • IDisposable implementation                                │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      │ DllImport
                      │
┌─────────────────────▼───────────────────────────────────────┐
│          SimpleOverlay.dll (Native C++)                      │
│                                                              │
│  State Management:                                           │
│  • Global overlay vector with mutex                          │
│  • Per-overlay state with mutex                              │
│                                                              │
│  D3D11 Resources:                                            │
│  • ID3D11Device                                              │
│  • ID3D11DeviceContext                                       │
│  • ID3D11Texture2D                                           │
│  • ID3D11RenderTargetView                                    │
│  • ID3D11ShaderResourceView                                  │
│  • IDXGISwapChain1                                           │
│                                                              │
│  Win32 Resources:                                            │
│  • HWND (layered, topmost window)                            │
│  • Window class registration                                 │
│  • Message pump handling                                     │
└─────────────────────┬───────────────────────────────────────┘
                      │
                      │ Win32 API / D3D11 API
                      │
┌─────────────────────▼───────────────────────────────────────┐
│              Windows Graphics Stack                          │
│  • Direct3D 11                                               │
│  • DXGI                                                      │
│  • Desktop Window Manager (DWM)                              │
└──────────────────────────────────────────────────────────────┘
```

## Building

### Quick Build (Recommended)

Use the provided PowerShell script to build everything:

```powershell
cd src/utilities/SimpleOverlay
.\build.ps1
```

This will:
1. Configure CMake for Visual Studio 2022 x64
2. Build both Release and Debug configurations
3. Build the C# wrapper library
4. Build the C# example application
5. Copy the DLL to the example output directory

#### Build Script Options

```powershell
# Build only Release configuration
.\build.ps1 -Configuration Release

# Skip CMake build (if already built)
.\build.ps1 -SkipCMake

# Skip C# wrapper build
.\build.ps1 -SkipCSharp

# Skip example build
.\build.ps1 -SkipExample

# Build only Debug configuration
.\build.ps1 -Configuration Debug
```

### Manual Build

#### Native Library (C++)

```bash
# From SimpleOverlay directory
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cmake --build build --config Debug
```

This will produce `SimpleOverlay.dll` in the `build/bin/Release` and `build/bin/Debug` directories.

#### C# Library

```bash
cd SimpleOverlay.CSharp
dotnet build --configuration Release
```

#### C# Example

```bash
cd Examples/CSharp
dotnet build --configuration Release

# Copy native DLL to output directory
copy ..\..\build\bin\Release\SimpleOverlay.dll bin\Release\net6.0\
```

## Usage

### C# Quick Start

```csharp
using OpenKneeboard.SimpleOverlay;

// Create a simple overlay
using (var overlay = new SimpleOverlay(800, 600))
{
    overlay.Show();
    
    // Your rendering loop here
    overlay.Render();
    
    overlay.Hide();
}
```

### C# Advanced Example

```csharp
using OpenKneeboard.SimpleOverlay;
using System.Threading;

// Create overlay with custom configuration
var config = new SimpleOverlayConfig(
    width: 1024,
    height: 768,
    x: 100,
    y: 100,
    opacity: 0.9f,
    enableVSync: true
);

using (var overlay = new SimpleOverlay(config))
{
    // Show the overlay
    overlay.Show();
    
    // Update position
    overlay.SetPosition(200, 200);
    
    // Update opacity
    overlay.SetOpacity(0.8f);
    
    // Create and set texture data (RGBA format)
    int width = 1024;
    int height = 768;
    byte[] textureData = new byte[width * height * 4];
    
    // Fill with gradient
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 4;
            textureData[index + 0] = (byte)(x * 255 / width);  // R
            textureData[index + 1] = (byte)(y * 255 / height); // G
            textureData[index + 2] = 128;                       // B
            textureData[index + 3] = 255;                       // A
        }
    }
    
    overlay.SetTexture(textureData, width, height);
    
    // Render loop
    for (int i = 0; i < 60; i++)
    {
        overlay.Render();
        Thread.Sleep(16); // ~60 FPS
    }
    
    overlay.Hide();
}
```

### Native C API Example

```cpp
#include "SimpleOverlay.h"

// Create overlay
SimpleOverlayConfig config = {};
config.width = 800;
config.height = 600;
config.x = 0;
config.y = 0;
config.opacity = 1.0f;
config.enableVSync = 1;

SimpleOverlayHandle handle;
SimpleOverlayResult result = SimpleOverlay_Create(&config, &handle);
if (result != SIMPLEOVERLAY_SUCCESS) {
    // Handle error
    return;
}

// Show overlay
SimpleOverlay_Show(handle);

// Set texture data
std::vector<unsigned char> textureData(800 * 600 * 4);
// ... fill texture data ...
SimpleOverlay_SetTexture(handle, textureData.data(), 800, 600);

// Render
SimpleOverlay_Render(handle);

// Clean up
SimpleOverlay_Destroy(handle);
```

### Running Examples

```bash
cd Examples/CSharp
dotnet run
```

The example application demonstrates:
1. Basic overlay creation and display
2. Custom configuration
3. Animated position and opacity
4. Texture rendering with gradients

## API Reference

### C# API

#### SimpleOverlay Class

**Constructors:**
- `SimpleOverlay(SimpleOverlayConfig config)` - Creates overlay with custom configuration
- `SimpleOverlay(int width, int height)` - Creates overlay with default settings

**Methods:**
- `void Show()` - Shows the overlay window
- `void Hide()` - Hides the overlay window
- `void SetPosition(int x, int y)` - Updates overlay position
- `void SetOpacity(float opacity)` - Updates overlay opacity (0.0 to 1.0)
- `void Render()` - Renders a frame
- `void SetTexture(byte[] textureData, int width, int height)` - Sets RGBA texture data
- `void Dispose()` - Releases native resources

**Properties:**
- `bool IsValid` - Returns true if the overlay is valid and not disposed

#### SimpleOverlayConfig Struct

- `int Width` - Overlay width in pixels
- `int Height` - Overlay height in pixels
- `int X` - Initial X position
- `int Y` - Initial Y position
- `float Opacity` - Initial opacity (0.0 to 1.0)
- `int EnableVSync` - Enable VSync (1 = enabled, 0 = disabled)

### Native C API

All functions return `SimpleOverlayResult` status code.

**Functions:**
- `SimpleOverlay_Create(config, outHandle)` - Initialize and create overlay instance
- `SimpleOverlay_Destroy(handle)` - Destroy overlay and free resources
- `SimpleOverlay_Show(handle)` - Show the overlay window
- `SimpleOverlay_Hide(handle)` - Hide the overlay window
- `SimpleOverlay_SetPosition(handle, x, y)` - Update overlay position
- `SimpleOverlay_SetOpacity(handle, opacity)` - Update overlay opacity
- `SimpleOverlay_Render(handle)` - Render a frame
- `SimpleOverlay_SetTexture(handle, textureData, width, height)` - Update texture data

**Result Codes:**
- `SIMPLEOVERLAY_SUCCESS (0)` - Operation succeeded
- `SIMPLEOVERLAY_ERROR_INVALID_HANDLE (-1)` - Invalid overlay handle
- `SIMPLEOVERLAY_ERROR_INVALID_PARAMETER (-2)` - Invalid parameter
- `SIMPLEOVERLAY_ERROR_INITIALIZATION_FAILED (-3)` - Initialization failed
- `SIMPLEOVERLAY_ERROR_RENDER_FAILED (-4)` - Render operation failed
- `SIMPLEOVERLAY_ERROR_NOT_FOUND (-5)` - Resource not found

## Implementation Details

### D3D11 Rendering

The library uses Direct3D 11 for high-performance GPU-accelerated rendering:

- **Device Creation**: `D3D11CreateDevice` with hardware acceleration
- **Texture Management**: `ID3D11Texture2D` with `BIND_SHADER_RESOURCE | BIND_RENDER_TARGET`
- **Shader Resources**: `ID3D11ShaderResourceView` for texture access
- **Render Targets**: `ID3D11RenderTargetView` for rendering operations
- **Swap Chain**: `IDXGISwapChain1` with flip presentation model
- **Resource Updates**: `UpdateSubresource` for efficient texture updates

### Window Management

Overlay windows are created with specific styles for proper overlay behavior:

- **Window Style**: `WS_POPUP` (borderless, no chrome)
- **Extended Style**: `WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE`
- **Layered Attributes**: `SetLayeredWindowAttributes` for opacity control
- **Always on Top**: Maintained via `HWND_TOPMOST` positioning

### Thread Safety

All operations are protected by mutexes:

- **Global Mutex**: Protects the overlay list for concurrent create/destroy
- **Per-Overlay Mutex**: Protects individual overlay state modifications
- **Lock Guards**: RAII-style locking with `std::lock_guard`

### Resource Management

The library follows RAII principles for automatic cleanup:

- **COM Smart Pointers**: `Microsoft::WRL::ComPtr` for D3D11 objects
- **Unique Pointers**: `std::unique_ptr` for overlay state
- **Automatic Cleanup**: Resources released when overlay is destroyed
- **No Manual Release**: All COM objects cleaned up automatically

## Troubleshooting

### DLL Not Found

**Problem**: Application fails to start with "SimpleOverlay.dll not found"

**Solutions**:
1. Copy `SimpleOverlay.dll` to the application directory
2. Add the DLL location to your PATH
3. Use the build script which copies the DLL automatically

### D3D11 Initialization Failed

**Problem**: `SIMPLEOVERLAY_ERROR_INITIALIZATION_FAILED` on create

**Solutions**:
1. Ensure you have a D3D11-compatible GPU
2. Update your graphics drivers
3. Check if D3D11 runtime is installed (comes with Windows 10+)
4. Try running as administrator

### Overlay Not Visible

**Problem**: Overlay created successfully but not visible

**Solutions**:
1. Call `Show()` after creating the overlay
2. Check if another window is covering the overlay
3. Verify the position is on-screen
4. Check opacity is not set to 0

### Texture Not Displaying

**Problem**: Texture set but not visible when rendered

**Solutions**:
1. Ensure texture dimensions match overlay dimensions
2. Verify texture data is in RGBA format (4 bytes per pixel)
3. Call `Render()` after `SetTexture()`
4. Check alpha channel values (should be 255 for opaque)

### Build Errors

**Problem**: CMake or build fails

**Solutions**:
1. Ensure Visual Studio 2022 is installed
2. Install Windows 10 SDK (version 10.0.20348.0 or later)
3. Verify CMake version is 3.20 or later
4. For C# builds, ensure .NET 6.0 SDK is installed

## Integration with OpenKneeboard

SimpleOverlay can be integrated into OpenKneeboard for overlay rendering:

1. **Include the header**: `#include "SimpleOverlay.h"`
2. **Link the library**: Add `SimpleOverlay` to target dependencies
3. **Create overlay**: Call `SimpleOverlay_Create` during initialization
4. **Update rendering**: Use `SimpleOverlay_SetTexture` to update content
5. **Render frames**: Call `SimpleOverlay_Render` in your render loop
6. **Cleanup**: Call `SimpleOverlay_Destroy` on shutdown

### Example Integration

```cpp
// During initialization
SimpleOverlayConfig config = {800, 600, 0, 0, 1.0f, 1};
SimpleOverlayHandle overlay;
SimpleOverlay_Create(&config, &overlay);
SimpleOverlay_Show(overlay);

// In render loop
std::vector<unsigned char> frameData = GetFrameData();
SimpleOverlay_SetTexture(overlay, frameData.data(), 800, 600);
SimpleOverlay_Render(overlay);

// On shutdown
SimpleOverlay_Destroy(overlay);
```

## Requirements

- **Operating System**: Windows 10 or later (19H1 or newer recommended)
- **Graphics API**: Direct3D 11 compatible GPU
- **Compiler**: Visual Studio 2022 or compatible C++20 compiler
- **.NET Runtime**: .NET 6.0 or later (for C# bindings)
- **Build Tools**: CMake 3.20+, MSBuild

## Performance Considerations

- **VSync**: Enable VSync for smooth rendering at display refresh rate
- **Texture Updates**: Only call `SetTexture` when content changes
- **Render Frequency**: Match your application's frame rate
- **Multiple Overlays**: Each overlay has its own D3D11 device and resources

## License

See the main OpenKneeboard LICENSE file for licensing information.

## Contributing

Contributions are welcome! Please ensure:
1. Code follows existing style conventions
2. All builds pass (Debug and Release)
3. Examples continue to work
4. Documentation is updated for API changes
