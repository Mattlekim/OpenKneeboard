#include "SimpleOpenXROverlay.h"

#include <Windows.h>
#include <d3d11.h>
#include <d3d12.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>

#include <algorithm>
#include <map>
#include <mutex>

using Microsoft::WRL::ComPtr;

namespace SimpleOverlay {

namespace {
  constexpr const char* LIBRARY_VERSION = "1.0.0";
}

// ============================================================================
// Window Finding Logic
// ============================================================================

struct WindowInfo {
  HWND hwnd;
  std::string title;
  std::string processName;
};

static std::vector<WindowInfo> g_EnumeratedWindows;

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
  if (!IsWindowVisible(hwnd)) {
    return TRUE;
  }

  char title[256];
  if (GetWindowTextA(hwnd, title, sizeof(title)) == 0) {
    return TRUE;
  }

  DWORD processId = 0;
  GetWindowThreadProcessId(hwnd, &processId);

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
  char processName[MAX_PATH] = "";
  if (hProcess) {
    DWORD size = MAX_PATH;
    QueryFullProcessImageNameA(hProcess, 0, processName, &size);
    CloseHandle(hProcess);
  }

  WindowInfo info;
  info.hwnd = hwnd;
  info.title = title;
  info.processName = processName;
  g_EnumeratedWindows.push_back(info);

  return TRUE;
}

static HWND FindWindowBySpec(const WindowMatchSpec& spec) {
  g_EnumeratedWindows.clear();
  EnumWindows(EnumWindowsProc, 0);

  for (const auto& window : g_EnumeratedWindows) {
    bool titleMatch = spec.windowTitle.empty();
    bool processMatch = spec.processName.empty();

    if (!spec.windowTitle.empty()) {
      if (spec.exactMatch) {
        titleMatch = (window.title == spec.windowTitle);
      } else {
        titleMatch = (window.title.find(spec.windowTitle) != std::string::npos);
      }
    }

    if (!spec.processName.empty()) {
      if (spec.exactMatch) {
        processMatch = (window.processName.find(spec.processName) != std::string::npos);
      } else {
        processMatch = (window.processName.find(spec.processName) != std::string::npos);
      }
    }

    if (titleMatch && processMatch) {
      return window.hwnd;
    }
  }

  return nullptr;
}

// ============================================================================
// Graphics API Detection and Management
// ============================================================================

class GraphicsManager {
public:
  GraphicsManager() : mAPI(GraphicsAPI::Unknown) {}

  bool Initialize() {
    // Try to detect D3D11
    if (DetectD3D11()) {
      mAPI = GraphicsAPI::D3D11;
      return true;
    }

    // Try to detect D3D12
    if (DetectD3D12()) {
      mAPI = GraphicsAPI::D3D12;
      return true;
    }

    // Try to detect Vulkan
    if (DetectVulkan()) {
      mAPI = GraphicsAPI::Vulkan;
      return true;
    }

    return false;
  }

  GraphicsAPI GetAPI() const { return mAPI; }

  bool CaptureWindow(HWND hwnd) {
    if (!hwnd) return false;

    switch (mAPI) {
      case GraphicsAPI::D3D11:
        return CaptureD3D11(hwnd);
      case GraphicsAPI::D3D12:
        return CaptureD3D12(hwnd);
      case GraphicsAPI::Vulkan:
        return CaptureVulkan(hwnd);
      default:
        return false;
    }
  }

private:
  bool DetectD3D11() {
    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      0,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      &mD3D11Device,
      &featureLevel,
      &mD3D11Context);

    return SUCCEEDED(hr);
  }

  bool DetectD3D12() {
    HRESULT hr = D3D12CreateDevice(
      nullptr,
      D3D_FEATURE_LEVEL_11_0,
      IID_PPV_ARGS(&mD3D12Device));

    return SUCCEEDED(hr);
  }

  bool DetectVulkan() {
    // Simplified Vulkan detection
    // In a real implementation, you would check for Vulkan loader
    HMODULE vulkanLib = LoadLibraryA("vulkan-1.dll");
    if (vulkanLib) {
      FreeLibrary(vulkanLib);
      return true;
    }
    return false;
  }

  bool CaptureD3D11(HWND hwnd) {
    if (!mD3D11Device) return false;

    // Get window dimensions
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    // Create texture for capture
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

    HRESULT hr = mD3D11Device->CreateTexture2D(&desc, nullptr, &mCaptureTexture);
    return SUCCEEDED(hr);
  }

  bool CaptureD3D12(HWND hwnd) {
    // D3D12 capture implementation
    return mD3D12Device != nullptr;
  }

  bool CaptureVulkan(HWND hwnd) {
    // Vulkan capture implementation
    return false;
  }

  GraphicsAPI mAPI;
  ComPtr<ID3D11Device> mD3D11Device;
  ComPtr<ID3D11DeviceContext> mD3D11Context;
  ComPtr<ID3D11Texture2D> mCaptureTexture;
  ComPtr<ID3D12Device> mD3D12Device;
};

// ============================================================================
// ApplicationOverlay Implementation
// ============================================================================

class ApplicationOverlay::Impl {
public:
  Impl()
    : mInitialized(false),
      mVisible(true),
      mTargetWindow(nullptr) {}

  bool Initialize() {
    if (mInitialized) return true;

    if (!mGraphics.Initialize()) {
      NotifyError("Failed to initialize graphics system");
      return false;
    }

    // Initialize OpenXR (simplified)
    // In a real implementation, you would create XrInstance, XrSession, etc.

    mInitialized = true;
    return true;
  }

  void Shutdown() {
    if (!mInitialized) return;

    mTargetWindow = nullptr;
    mInitialized = false;
  }

  bool SetTargetWindow(const WindowMatchSpec& spec) {
    HWND hwnd = FindWindowBySpec(spec);
    if (!hwnd) {
      NotifyError("Window not found matching specification");
      return false;
    }

    mTargetWindow = hwnd;
    return mGraphics.CaptureWindow(hwnd);
  }

  void SetConfig(const OverlayConfig& config) {
    std::lock_guard<std::mutex> lock(mConfigMutex);
    mConfig = config;
  }

  const OverlayConfig& GetConfig() const {
    return mConfig;
  }

  void Update() {
    if (!mInitialized || !mVisible || !mTargetWindow) {
      return;
    }

    // Update overlay rendering
    // In a real implementation, you would:
    // 1. Capture the target window content
    // 2. Update the OpenXR overlay quad
    // 3. Apply configuration settings
  }

  bool IsActive() const {
    return mInitialized && mTargetWindow != nullptr;
  }

  void SetVisible(bool visible) {
    mVisible = visible;
  }

  GraphicsAPI GetGraphicsAPI() const {
    return mGraphics.GetAPI();
  }

  void SetErrorCallback(std::function<void(const std::string&)> callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mErrorCallback = callback;
  }

private:
  void NotifyError(const std::string& message) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    if (mErrorCallback) {
      mErrorCallback(message);
    }
  }

  bool mInitialized;
  bool mVisible;
  HWND mTargetWindow;
  OverlayConfig mConfig;
  GraphicsManager mGraphics;
  
  std::mutex mConfigMutex;
  std::mutex mCallbackMutex;
  std::function<void(const std::string&)> mErrorCallback;
};

// ============================================================================
// ApplicationOverlay Public Interface
// ============================================================================

ApplicationOverlay::ApplicationOverlay()
  : pImpl(std::make_unique<Impl>()) {}

ApplicationOverlay::~ApplicationOverlay() {
  if (pImpl) {
    pImpl->Shutdown();
  }
}

bool ApplicationOverlay::Initialize() {
  return pImpl->Initialize();
}

void ApplicationOverlay::Shutdown() {
  pImpl->Shutdown();
}

bool ApplicationOverlay::SetTargetWindow(const WindowMatchSpec& spec) {
  return pImpl->SetTargetWindow(spec);
}

void ApplicationOverlay::SetConfig(const OverlayConfig& config) {
  pImpl->SetConfig(config);
}

const OverlayConfig& ApplicationOverlay::GetConfig() const {
  return pImpl->GetConfig();
}

void ApplicationOverlay::Update() {
  pImpl->Update();
}

bool ApplicationOverlay::IsActive() const {
  return pImpl->IsActive();
}

void ApplicationOverlay::SetVisible(bool visible) {
  pImpl->SetVisible(visible);
}

GraphicsAPI ApplicationOverlay::GetGraphicsAPI() const {
  return pImpl->GetGraphicsAPI();
}

void ApplicationOverlay::SetErrorCallback(std::function<void(const std::string&)> callback) {
  pImpl->SetErrorCallback(callback);
}

// ============================================================================
// Utility Functions
// ============================================================================

std::unique_ptr<ApplicationOverlay> LoadOverlay(
  const std::string& windowTitle,
  const OverlayConfig& config) {
  WindowMatchSpec spec(windowTitle);
  return LoadOverlay(spec, config);
}

std::unique_ptr<ApplicationOverlay> LoadOverlay(
  const WindowMatchSpec& spec,
  const OverlayConfig& config) {
  auto overlay = std::make_unique<ApplicationOverlay>();
  
  if (!overlay->Initialize()) {
    return nullptr;
  }

  overlay->SetConfig(config);
  
  if (!overlay->SetTargetWindow(spec)) {
    return nullptr;
  }

  return overlay;
}

std::vector<std::string> GetAvailableWindows() {
  g_EnumeratedWindows.clear();
  EnumWindows(EnumWindowsProc, 0);

  std::vector<std::string> titles;
  titles.reserve(g_EnumeratedWindows.size());
  
  for (const auto& window : g_EnumeratedWindows) {
    if (!window.title.empty()) {
      titles.push_back(window.title);
    }
  }

  return titles;
}

const char* GetVersion() {
  return LIBRARY_VERSION;
}

} // namespace SimpleOverlay
