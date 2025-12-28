#pragma once

#ifdef _WIN32
  #ifdef SIMPLEOVERLAY_EXPORTS
    #define SIMPLEOVERLAY_API __declspec(dllexport)
  #else
    #define SIMPLEOVERLAY_API __declspec(dllimport)
  #endif
  #define SIMPLEOVERLAY_CALL __cdecl
#else
  #define SIMPLEOVERLAY_API
  #define SIMPLEOVERLAY_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Opaque handle for overlay instance
typedef void* SimpleOverlayHandle;

// Result codes
typedef enum {
  SIMPLEOVERLAY_SUCCESS = 0,
  SIMPLEOVERLAY_ERROR_INVALID_HANDLE = -1,
  SIMPLEOVERLAY_ERROR_INVALID_PARAMETER = -2,
  SIMPLEOVERLAY_ERROR_INITIALIZATION_FAILED = -3,
  SIMPLEOVERLAY_ERROR_RENDER_FAILED = -4,
  SIMPLEOVERLAY_ERROR_NOT_FOUND = -5
} SimpleOverlayResult;

// Overlay configuration
typedef struct {
  int width;
  int height;
  int x;
  int y;
  float opacity;
  int enableVSync;
} SimpleOverlayConfig;

/**
 * Initialize and create a new overlay instance
 * @param config Pointer to overlay configuration
 * @param outHandle Pointer to receive the overlay handle
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_Create(const SimpleOverlayConfig* config, SimpleOverlayHandle* outHandle);

/**
 * Destroy an overlay instance and free resources
 * @param handle Overlay handle to destroy
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_Destroy(SimpleOverlayHandle handle);

/**
 * Show the overlay window
 * @param handle Overlay handle
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_Show(SimpleOverlayHandle handle);

/**
 * Hide the overlay window
 * @param handle Overlay handle
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_Hide(SimpleOverlayHandle handle);

/**
 * Update overlay position
 * @param handle Overlay handle
 * @param x X coordinate
 * @param y Y coordinate
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_SetPosition(SimpleOverlayHandle handle, int x, int y);

/**
 * Update overlay opacity
 * @param handle Overlay handle
 * @param opacity Opacity value (0.0 to 1.0)
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_SetOpacity(SimpleOverlayHandle handle, float opacity);

/**
 * Render a frame
 * @param handle Overlay handle
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_Render(SimpleOverlayHandle handle);

/**
 * Set texture data for rendering
 * @param handle Overlay handle
 * @param textureData Pointer to texture pixel data (RGBA)
 * @param width Texture width
 * @param height Texture height
 * @return Result code
 */
SIMPLEOVERLAY_API SimpleOverlayResult SIMPLEOVERLAY_CALL 
SimpleOverlay_SetTexture(SimpleOverlayHandle handle, const unsigned char* textureData, int width, int height);

#ifdef __cplusplus
}
#endif
