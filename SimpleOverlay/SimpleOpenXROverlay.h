#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace SimpleOverlay {

// Forward declarations
struct OverlayConfig;
class ApplicationOverlay;

/**
 * Specification for matching windows by title or process name
 */
struct WindowMatchSpec {
  std::string windowTitle;      // Match by window title (empty to ignore)
  std::string processName;      // Match by process name (empty to ignore)
  bool exactMatch = false;      // Require exact match vs. substring match
  
  WindowMatchSpec() = default;
  WindowMatchSpec(const std::string& title, const std::string& process = "")
    : windowTitle(title), processName(process) {}
};

/**
 * Configuration for overlay behavior
 */
struct OverlayConfig {
  float width = 1.0f;           // Width in meters in VR space
  float height = 0.75f;         // Height in meters in VR space
  float distance = 1.5f;        // Distance from user in meters
  float offsetX = 0.0f;         // Horizontal offset
  float offsetY = 0.0f;         // Vertical offset
  bool followGaze = false;      // Follow user's head orientation
  bool alwaysVisible = true;    // Always show overlay
  float opacity = 1.0f;         // Overlay opacity (0.0 - 1.0)
};

/**
 * Graphics API types supported by the overlay
 */
enum class GraphicsAPI {
  Unknown,
  D3D11,
  D3D12,
  Vulkan,
  OpenGL
};

/**
 * Main overlay class that captures and displays window content in VR
 */
class ApplicationOverlay {
public:
  ApplicationOverlay();
  ~ApplicationOverlay();

  // Prevent copying
  ApplicationOverlay(const ApplicationOverlay&) = delete;
  ApplicationOverlay& operator=(const ApplicationOverlay&) = delete;

  /**
   * Initialize the overlay system
   * @return true if initialization succeeded
   */
  bool Initialize();

  /**
   * Shutdown and cleanup the overlay system
   */
  void Shutdown();

  /**
   * Set the window to capture and display
   * @param spec Window matching specification
   * @return true if window was found and capture started
   */
  bool SetTargetWindow(const WindowMatchSpec& spec);

  /**
   * Update overlay configuration
   * @param config New configuration settings
   */
  void SetConfig(const OverlayConfig& config);

  /**
   * Get current configuration
   */
  const OverlayConfig& GetConfig() const;

  /**
   * Update overlay (call once per frame)
   */
  void Update();

  /**
   * Check if overlay is currently active
   */
  bool IsActive() const;

  /**
   * Set overlay visibility
   */
  void SetVisible(bool visible);

  /**
   * Get detected graphics API
   */
  GraphicsAPI GetGraphicsAPI() const;

  /**
   * Set callback for error notifications
   */
  void SetErrorCallback(std::function<void(const std::string&)> callback);

private:
  class Impl;
  std::unique_ptr<Impl> pImpl;
};

/**
 * Load and initialize an overlay for a specific window
 * @param windowTitle Title of the window to overlay
 * @param config Overlay configuration
 * @return Pointer to initialized overlay or nullptr on failure
 */
std::unique_ptr<ApplicationOverlay> LoadOverlay(
  const std::string& windowTitle,
  const OverlayConfig& config = OverlayConfig());

/**
 * Load and initialize an overlay using window match specification
 * @param spec Window matching specification
 * @param config Overlay configuration
 * @return Pointer to initialized overlay or nullptr on failure
 */
std::unique_ptr<ApplicationOverlay> LoadOverlay(
  const WindowMatchSpec& spec,
  const OverlayConfig& config = OverlayConfig());

/**
 * Get list of available windows for overlay
 * @return Vector of window titles
 */
std::vector<std::string> GetAvailableWindows();

/**
 * Get library version string
 */
const char* GetVersion();

} // namespace SimpleOverlay
