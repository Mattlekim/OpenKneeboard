# SimpleOverlay - OpenXR Window Overlay Library

A lightweight C++ library for displaying application windows as overlays in OpenXR-compatible VR experiences.

## Overview

SimpleOverlay provides an easy-to-use API for capturing desktop application windows and rendering them as overlays in Virtual Reality. The library automatically detects the graphics API (Direct3D 11, Direct3D 12, or Vulkan) and handles all the complexity of window capture and OpenXR integration.

## Features

- **Automatic Graphics API Detection**: Supports D3D11, D3D12, and Vulkan
- **Flexible Window Matching**: Find windows by title or process name with exact or substring matching
- **Configurable Overlay Positioning**: Control size, distance, offset, and opacity
- **Head-Locked or World-Locked**: Option to follow user's gaze or remain fixed in space
- **Simple C++ API**: Easy to integrate into existing applications
- **Thread-Safe**: Safe to use from multiple threads

## Requirements

- Windows 10 or later
- OpenXR Runtime (SteamVR, Oculus, Windows Mixed Reality, etc.)
- C++17 compatible compiler
- CMake 3.15 or later
- DirectX 11/12 SDK or Vulkan SDK

## Building

### Prerequisites

```bash
# Install dependencies (example using vcpkg)
vcpkg install openxr-loader
```

### Build Steps

```bash
# Clone the repository
git clone https://github.com/Mattlekim/OpenKneeboard.git
cd OpenKneeboard/SimpleOverlay

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . --config Release
```

## Usage

### Basic Example

```cpp
#include "SimpleOpenXROverlay.h"

using namespace SimpleOverlay;

int main() {
    // Configure overlay properties
    OverlayConfig config;
    config.width = 1.5f;        // 1.5 meters wide
    config.height = 1.0f;       // 1.0 meter tall
    config.distance = 2.0f;     // 2 meters from user
    config.opacity = 0.9f;      // 90% opacity
    
    // Load overlay for a specific window
    auto overlay = LoadOverlay("Notepad", config);
    
    if (!overlay) {
        printf("Failed to create overlay\n");
        return 1;
    }
    
    // Main loop
    while (true) {
        overlay->Update();
        // Your application logic here
    }
    
    return 0;
}
```

### Advanced Window Matching

```cpp
// Match by exact window title
WindowMatchSpec spec;
spec.windowTitle = "My Application - Document1";
spec.exactMatch = true;

auto overlay = LoadOverlay(spec);
```

```cpp
// Match by process name
WindowMatchSpec spec;
spec.processName = "chrome.exe";
spec.windowTitle = "YouTube";  // Also match title substring
spec.exactMatch = false;

auto overlay = LoadOverlay(spec);
```

### Dynamic Configuration

```cpp
auto overlay = LoadOverlay("Calculator");

// Change overlay properties at runtime
OverlayConfig newConfig;
newConfig.followGaze = true;     // Make overlay follow head movement
newConfig.offsetY = 0.5f;        // Offset upward

overlay->SetConfig(newConfig);
```

### Error Handling

```cpp
auto overlay = std::make_unique<ApplicationOverlay>();

// Set error callback
overlay->SetErrorCallback([](const std::string& error) {
    printf("Overlay Error: %s\n", error.c_str());
});

if (!overlay->Initialize()) {
    printf("Failed to initialize overlay\n");
    return 1;
}

WindowMatchSpec spec("MyWindow");
if (!overlay->SetTargetWindow(spec)) {
    printf("Window not found\n");
    return 1;
}
```

### Listing Available Windows

```cpp
auto windows = GetAvailableWindows();

printf("Available windows:\n");
for (const auto& title : windows) {
    printf("  - %s\n", title.c_str());
}
```

## API Reference

### Classes

#### `ApplicationOverlay`

Main class for managing window overlays in VR.

**Methods:**
- `bool Initialize()` - Initialize the overlay system
- `void Shutdown()` - Cleanup and shutdown
- `bool SetTargetWindow(const WindowMatchSpec& spec)` - Set the window to overlay
- `void SetConfig(const OverlayConfig& config)` - Update overlay configuration
- `const OverlayConfig& GetConfig() const` - Get current configuration
- `void Update()` - Update overlay (call each frame)
- `bool IsActive() const` - Check if overlay is active
- `void SetVisible(bool visible)` - Show/hide overlay
- `GraphicsAPI GetGraphicsAPI() const` - Get detected graphics API
- `void SetErrorCallback(std::function<void(const std::string&)> callback)` - Set error handler

### Structures

#### `WindowMatchSpec`

Specifies how to find target windows.

**Fields:**
- `std::string windowTitle` - Window title to match
- `std::string processName` - Process name to match
- `bool exactMatch` - Require exact match (default: false)

#### `OverlayConfig`

Configuration for overlay appearance and behavior.

**Fields:**
- `float width` - Width in meters (default: 1.0)
- `float height` - Height in meters (default: 0.75)
- `float distance` - Distance from user in meters (default: 1.5)
- `float offsetX` - Horizontal offset (default: 0.0)
- `float offsetY` - Vertical offset (default: 0.0)
- `bool followGaze` - Follow head orientation (default: false)
- `bool alwaysVisible` - Always show overlay (default: true)
- `float opacity` - Opacity 0.0-1.0 (default: 1.0)

### Functions

- `std::unique_ptr<ApplicationOverlay> LoadOverlay(const std::string& windowTitle, const OverlayConfig& config = {})` - Load overlay by window title
- `std::unique_ptr<ApplicationOverlay> LoadOverlay(const WindowMatchSpec& spec, const OverlayConfig& config = {})` - Load overlay with match spec
- `std::vector<std::string> GetAvailableWindows()` - Get list of available windows
- `const char* GetVersion()` - Get library version string

## Integration with OpenKneeboard

This library is designed to work seamlessly with OpenKneeboard, providing a simple way to overlay kneeboard content or other application windows in VR flight simulators and other OpenXR applications.

```cpp
// Example: Overlay OpenKneeboard in VR
OverlayConfig config;
config.width = 0.8f;
config.height = 1.0f;
config.distance = 1.2f;
config.offsetX = -0.5f;  // Position to the left

auto kneeboard = LoadOverlay("OpenKneeboard", config);
```

## Performance Considerations

- **Window Capture**: Capturing window content has a performance cost. Consider reducing capture frequency for static content.
- **Graphics API**: D3D11 and D3D12 typically provide better performance than Vulkan for window capture on Windows.
- **Resolution**: Higher resolution overlays consume more GPU resources. Balance quality with performance needs.

## Troubleshooting

### Overlay Not Visible
- Ensure OpenXR runtime is active
- Check that target window exists and is visible
- Verify overlay distance and position settings
- Confirm `SetVisible(true)` has been called

### Window Not Found
- Use `GetAvailableWindows()` to list available windows
- Try substring matching instead of exact matching
- Check if window title changes dynamically

### Performance Issues
- Reduce overlay resolution
- Lower update frequency
- Check graphics API detection (`GetGraphicsAPI()`)

## License

This library is part of the OpenKneeboard project. See the main repository for license information.

## Contributing

Contributions are welcome! Please submit pull requests to the main OpenKneeboard repository.

## Version History

### 1.0.0 (Initial Release)
- Basic overlay functionality
- D3D11, D3D12, and Vulkan support
- Window matching by title and process
- Configurable overlay positioning
- Error callback system
