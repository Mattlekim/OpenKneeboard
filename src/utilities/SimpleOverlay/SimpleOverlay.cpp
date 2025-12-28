#include "SimpleOverlay.h"

#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <memory>
#include <mutex>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace {

// Window class name for overlay windows
const wchar_t* OVERLAY_WINDOW_CLASS = L"SimpleOverlayWindowClass";

// Global state management
struct OverlayState {
  SimpleOverlayConfig config;
  
  // D3D11 resources
  ComPtr<ID3D11Device> device;
  ComPtr<ID3D11DeviceContext> context;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11ShaderResourceView> shaderResourceView;
  ComPtr<ID3D11RenderTargetView> renderTargetView;
  ComPtr<IDXGISwapChain1> swapChain;
  
  // Window handle
  HWND hwnd = nullptr;
  
  // State
  bool isVisible = false;
  
  // Thread safety
  std::mutex mutex;
};

// Global list of overlays with mutex protection
std::vector<std::unique_ptr<OverlayState>> g_overlays;
std::mutex g_overlaysMutex;

// Window procedure for overlay windows
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CLOSE:
      // Don't destroy the window, just hide it
      ShowWindow(hwnd, SW_HIDE);
      return 0;
    
    case WM_DESTROY:
      return 0;
    
    default:
      return DefWindowProc(hwnd, msg, wParam, lParam);
  }
}

// Register window class (one-time initialization)
bool RegisterOverlayWindowClass() {
  static bool registered = false;
  static std::mutex registerMutex;
  
  std::lock_guard<std::mutex> lock(registerMutex);
  
  if (registered) {
    return true;
  }
  
  WNDCLASSEXW wc = {};
  wc.cbSize = sizeof(WNDCLASSEXW);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = OverlayWndProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.lpszClassName = OVERLAY_WINDOW_CLASS;
  
  if (!RegisterClassExW(&wc)) {
    return false;
  }
  
  registered = true;
  return true;
}

// Initialize D3D11 device and resources
bool InitializeD3D11(OverlayState* state) {
  HRESULT hr;
  
  // Create D3D11 device
  D3D_FEATURE_LEVEL featureLevels[] = {
    D3D_FEATURE_LEVEL_11_1,
    D3D_FEATURE_LEVEL_11_0,
  };
  
  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
  
  D3D_FEATURE_LEVEL featureLevel;
  hr = D3D11CreateDevice(
    nullptr,                    // Use default adapter
    D3D_DRIVER_TYPE_HARDWARE,
    nullptr,
    createDeviceFlags,
    featureLevels,
    ARRAYSIZE(featureLevels),
    D3D11_SDK_VERSION,
    &state->device,
    &featureLevel,
    &state->context
  );
  
  if (FAILED(hr)) {
    return false;
  }
  
  // Create texture
  D3D11_TEXTURE2D_DESC texDesc = {};
  texDesc.Width = state->config.width;
  texDesc.Height = state->config.height;
  texDesc.MipLevels = 1;
  texDesc.ArraySize = 1;
  texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texDesc.SampleDesc.Count = 1;
  texDesc.SampleDesc.Quality = 0;
  texDesc.Usage = D3D11_USAGE_DEFAULT;
  texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
  texDesc.CPUAccessFlags = 0;
  texDesc.MiscFlags = 0;
  
  hr = state->device->CreateTexture2D(&texDesc, nullptr, &state->texture);
  if (FAILED(hr)) {
    return false;
  }
  
  // Create shader resource view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = texDesc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels = 1;
  srvDesc.Texture2D.MostDetailedMip = 0;
  
  hr = state->device->CreateShaderResourceView(
    state->texture.Get(),
    &srvDesc,
    &state->shaderResourceView
  );
  
  if (FAILED(hr)) {
    return false;
  }
  
  // Create render target view
  D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
  rtvDesc.Format = texDesc.Format;
  rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  rtvDesc.Texture2D.MipSlice = 0;
  
  hr = state->device->CreateRenderTargetView(
    state->texture.Get(),
    &rtvDesc,
    &state->renderTargetView
  );
  
  if (FAILED(hr)) {
    return false;
  }
  
  return true;
}

// Create overlay window
bool CreateOverlayWindow(OverlayState* state) {
  if (!RegisterOverlayWindowClass()) {
    return false;
  }
  
  // Create a layered, topmost, transparent window
  state->hwnd = CreateWindowExW(
    WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
    OVERLAY_WINDOW_CLASS,
    L"SimpleOverlay",
    WS_POPUP,
    state->config.x,
    state->config.y,
    state->config.width,
    state->config.height,
    nullptr,
    nullptr,
    GetModuleHandle(nullptr),
    nullptr
  );
  
  if (!state->hwnd) {
    return false;
  }
  
  // Set layered window attributes for opacity
  BYTE alpha = static_cast<BYTE>(state->config.opacity * 255.0f);
  SetLayeredWindowAttributes(state->hwnd, 0, alpha, LWA_ALPHA);
  
  return true;
}

// Create swap chain for the window
bool CreateSwapChain(OverlayState* state) {
  HRESULT hr;
  
  // Get DXGI device
  ComPtr<IDXGIDevice> dxgiDevice;
  hr = state->device.As(&dxgiDevice);
  if (FAILED(hr)) {
    return false;
  }
  
  // Get DXGI adapter
  ComPtr<IDXGIAdapter> dxgiAdapter;
  hr = dxgiDevice->GetAdapter(&dxgiAdapter);
  if (FAILED(hr)) {
    return false;
  }
  
  // Get DXGI factory
  ComPtr<IDXGIFactory2> dxgiFactory;
  hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);
  if (FAILED(hr)) {
    return false;
  }
  
  // Create swap chain
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.Width = state->config.width;
  swapChainDesc.Height = state->config.height;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.Flags = 0;
  
  hr = dxgiFactory->CreateSwapChainForHwnd(
    state->device.Get(),
    state->hwnd,
    &swapChainDesc,
    nullptr,
    nullptr,
    &state->swapChain
  );
  
  if (FAILED(hr)) {
    return false;
  }
  
  return true;
}

// Find overlay by handle
OverlayState* FindOverlay(SimpleOverlayHandle handle) {
  std::lock_guard<std::mutex> lock(g_overlaysMutex);
  
  for (auto& overlay : g_overlays) {
    if (overlay.get() == handle) {
      return overlay.get();
    }
  }
  
  return nullptr;
}

} // namespace

// C API Implementation

extern "C" {

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_Create(const SimpleOverlayConfig* config, SimpleOverlayHandle* outHandle) {
  if (!config || !outHandle) {
    return SIMPLEOVERLAY_ERROR_INVALID_PARAMETER;
  }
  
  // Validate configuration
  if (config->width <= 0 || config->height <= 0) {
    return SIMPLEOVERLAY_ERROR_INVALID_PARAMETER;
  }
  
  if (config->opacity < 0.0f || config->opacity > 1.0f) {
    return SIMPLEOVERLAY_ERROR_INVALID_PARAMETER;
  }
  
  // Create overlay state
  auto state = std::make_unique<OverlayState>();
  state->config = *config;
  
  // Initialize D3D11
  if (!InitializeD3D11(state.get())) {
    return SIMPLEOVERLAY_ERROR_INITIALIZATION_FAILED;
  }
  
  // Create window
  if (!CreateOverlayWindow(state.get())) {
    return SIMPLEOVERLAY_ERROR_INITIALIZATION_FAILED;
  }
  
  // Create swap chain
  if (!CreateSwapChain(state.get())) {
    DestroyWindow(state->hwnd);
    return SIMPLEOVERLAY_ERROR_INITIALIZATION_FAILED;
  }
  
  // Add to global list
  OverlayState* rawPtr = state.get();
  {
    std::lock_guard<std::mutex> lock(g_overlaysMutex);
    g_overlays.push_back(std::move(state));
  }
  
  *outHandle = rawPtr;
  return SIMPLEOVERLAY_SUCCESS;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_Destroy(SimpleOverlayHandle handle) {
  if (!handle) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(g_overlaysMutex);
  
  // Find and remove the overlay
  for (auto it = g_overlays.begin(); it != g_overlays.end(); ++it) {
    if (it->get() == handle) {
      OverlayState* state = it->get();
      
      // Clean up window
      if (state->hwnd) {
        DestroyWindow(state->hwnd);
        state->hwnd = nullptr;
      }
      
      // D3D11 resources will be cleaned up automatically by ComPtr
      g_overlays.erase(it);
      return SIMPLEOVERLAY_SUCCESS;
    }
  }
  
  return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_Show(SimpleOverlayHandle handle) {
  OverlayState* state = FindOverlay(handle);
  if (!state) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(state->mutex);
  
  if (!state->hwnd) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  ShowWindow(state->hwnd, SW_SHOW);
  UpdateWindow(state->hwnd);
  state->isVisible = true;
  
  return SIMPLEOVERLAY_SUCCESS;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_Hide(SimpleOverlayHandle handle) {
  OverlayState* state = FindOverlay(handle);
  if (!state) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(state->mutex);
  
  if (!state->hwnd) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  ShowWindow(state->hwnd, SW_HIDE);
  state->isVisible = false;
  
  return SIMPLEOVERLAY_SUCCESS;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_SetPosition(SimpleOverlayHandle handle, int x, int y) {
  OverlayState* state = FindOverlay(handle);
  if (!state) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(state->mutex);
  
  if (!state->hwnd) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  SetWindowPos(
    state->hwnd,
    HWND_TOPMOST,
    x,
    y,
    0,
    0,
    SWP_NOSIZE | SWP_NOACTIVATE
  );
  
  state->config.x = x;
  state->config.y = y;
  
  return SIMPLEOVERLAY_SUCCESS;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_SetOpacity(SimpleOverlayHandle handle, float opacity) {
  if (opacity < 0.0f || opacity > 1.0f) {
    return SIMPLEOVERLAY_ERROR_INVALID_PARAMETER;
  }
  
  OverlayState* state = FindOverlay(handle);
  if (!state) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(state->mutex);
  
  if (!state->hwnd) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  BYTE alpha = static_cast<BYTE>(opacity * 255.0f);
  SetLayeredWindowAttributes(state->hwnd, 0, alpha, LWA_ALPHA);
  
  state->config.opacity = opacity;
  
  return SIMPLEOVERLAY_SUCCESS;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_Render(SimpleOverlayHandle handle) {
  OverlayState* state = FindOverlay(handle);
  if (!state) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(state->mutex);
  
  if (!state->context || !state->renderTargetView || !state->swapChain) {
    return SIMPLEOVERLAY_ERROR_RENDER_FAILED;
  }
  
  // Clear render target to a default color (transparent black)
  float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  state->context->ClearRenderTargetView(state->renderTargetView.Get(), clearColor);
  
  // Get back buffer from swap chain
  ComPtr<ID3D11Texture2D> backBuffer;
  HRESULT hr = state->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
  if (FAILED(hr)) {
    return SIMPLEOVERLAY_ERROR_RENDER_FAILED;
  }
  
  // Copy our texture to the back buffer
  state->context->CopyResource(backBuffer.Get(), state->texture.Get());
  
  // Present
  UINT syncInterval = state->config.enableVSync ? 1 : 0;
  hr = state->swapChain->Present(syncInterval, 0);
  if (FAILED(hr)) {
    return SIMPLEOVERLAY_ERROR_RENDER_FAILED;
  }
  
  return SIMPLEOVERLAY_SUCCESS;
}

SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL
SimpleOverlay_SetTexture(
  SimpleOverlayHandle handle,
  const unsigned char* textureData,
  int width,
  int height
) {
  if (!textureData || width <= 0 || height <= 0) {
    return SIMPLEOVERLAY_ERROR_INVALID_PARAMETER;
  }
  
  OverlayState* state = FindOverlay(handle);
  if (!state) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  std::lock_guard<std::mutex> lock(state->mutex);
  
  if (!state->context || !state->texture) {
    return SIMPLEOVERLAY_ERROR_INVALID_HANDLE;
  }
  
  // Check if dimensions match
  D3D11_TEXTURE2D_DESC desc;
  state->texture->GetDesc(&desc);
  
  if (static_cast<UINT>(width) != desc.Width || static_cast<UINT>(height) != desc.Height) {
    return SIMPLEOVERLAY_ERROR_INVALID_PARAMETER;
  }
  
  // Update texture data
  D3D11_BOX box;
  box.left = 0;
  box.right = width;
  box.top = 0;
  box.bottom = height;
  box.front = 0;
  box.back = 1;
  
  UINT rowPitch = width * 4; // 4 bytes per pixel (RGBA)
  UINT depthPitch = rowPitch * height;
  
  state->context->UpdateSubresource(
    state->texture.Get(),
    0,
    &box,
    textureData,
    rowPitch,
    depthPitch
  );
  
  return SIMPLEOVERLAY_SUCCESS;
}

} // extern "C"
