using System;
using System.Runtime.InteropServices;

namespace OpenKneeboard.SimpleOverlay
{
    /// <summary>
    /// Result codes for SimpleOverlay operations
    /// </summary>
    public enum SimpleOverlayResult
    {
        Success = 0,
        ErrorInvalidHandle = -1,
        ErrorInvalidParameter = -2,
        ErrorInitializationFailed = -3,
        ErrorRenderFailed = -4,
        ErrorNotFound = -5
    }

    /// <summary>
    /// Configuration for creating an overlay
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SimpleOverlayConfig
    {
        public int Width;
        public int Height;
        public int X;
        public int Y;
        public float Opacity;
        public int EnableVSync;

        public SimpleOverlayConfig(int width, int height, int x = 0, int y = 0, float opacity = 1.0f, bool enableVSync = true)
        {
            Width = width;
            Height = height;
            X = x;
            Y = y;
            Opacity = opacity;
            EnableVSync = enableVSync ? 1 : 0;
        }
    }

    /// <summary>
    /// P/Invoke bindings for SimpleOverlay native library
    /// </summary>
    internal static class SimpleOverlayNative
    {
        private const string DllName = "SimpleOverlay.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_Create(
            ref SimpleOverlayConfig config,
            out IntPtr outHandle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_Destroy(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_Show(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_Hide(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_SetPosition(IntPtr handle, int x, int y);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_SetOpacity(IntPtr handle, float opacity);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_Render(IntPtr handle);

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern SimpleOverlayResult SimpleOverlay_SetTexture(
            IntPtr handle,
            byte[] textureData,
            int width,
            int height);
    }

    /// <summary>
    /// Managed wrapper for SimpleOverlay
    /// </summary>
    public class SimpleOverlay : IDisposable
    {
        private IntPtr _handle;
        private bool _disposed;

        /// <summary>
        /// Gets whether the overlay is currently valid
        /// </summary>
        public bool IsValid => _handle != IntPtr.Zero && !_disposed;

        /// <summary>
        /// Creates a new overlay instance
        /// </summary>
        /// <param name="config">Overlay configuration</param>
        /// <exception cref="InvalidOperationException">Thrown if creation fails</exception>
        public SimpleOverlay(SimpleOverlayConfig config)
        {
            var result = SimpleOverlayNative.SimpleOverlay_Create(ref config, out _handle);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to create overlay: {result}");
            }
        }

        /// <summary>
        /// Creates a new overlay with default settings
        /// </summary>
        /// <param name="width">Overlay width</param>
        /// <param name="height">Overlay height</param>
        public SimpleOverlay(int width, int height)
            : this(new SimpleOverlayConfig(width, height))
        {
        }

        /// <summary>
        /// Shows the overlay window
        /// </summary>
        public void Show()
        {
            ThrowIfDisposed();
            var result = SimpleOverlayNative.SimpleOverlay_Show(_handle);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to show overlay: {result}");
            }
        }

        /// <summary>
        /// Hides the overlay window
        /// </summary>
        public void Hide()
        {
            ThrowIfDisposed();
            var result = SimpleOverlayNative.SimpleOverlay_Hide(_handle);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to hide overlay: {result}");
            }
        }

        /// <summary>
        /// Sets the overlay position
        /// </summary>
        /// <param name="x">X coordinate</param>
        /// <param name="y">Y coordinate</param>
        public void SetPosition(int x, int y)
        {
            ThrowIfDisposed();
            var result = SimpleOverlayNative.SimpleOverlay_SetPosition(_handle, x, y);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to set position: {result}");
            }
        }

        /// <summary>
        /// Sets the overlay opacity
        /// </summary>
        /// <param name="opacity">Opacity value (0.0 to 1.0)</param>
        public void SetOpacity(float opacity)
        {
            ThrowIfDisposed();
            if (opacity < 0.0f || opacity > 1.0f)
            {
                throw new ArgumentOutOfRangeException(nameof(opacity), "Opacity must be between 0.0 and 1.0");
            }

            var result = SimpleOverlayNative.SimpleOverlay_SetOpacity(_handle, opacity);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to set opacity: {result}");
            }
        }

        /// <summary>
        /// Renders a frame
        /// </summary>
        public void Render()
        {
            ThrowIfDisposed();
            var result = SimpleOverlayNative.SimpleOverlay_Render(_handle);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to render: {result}");
            }
        }

        /// <summary>
        /// Sets the texture data for rendering
        /// </summary>
        /// <param name="textureData">RGBA pixel data</param>
        /// <param name="width">Texture width</param>
        /// <param name="height">Texture height</param>
        public void SetTexture(byte[] textureData, int width, int height)
        {
            ThrowIfDisposed();
            if (textureData == null)
            {
                throw new ArgumentNullException(nameof(textureData));
            }

            if (textureData.Length != width * height * 4)
            {
                throw new ArgumentException("Texture data size must match width * height * 4 (RGBA)");
            }

            var result = SimpleOverlayNative.SimpleOverlay_SetTexture(_handle, textureData, width, height);
            if (result != SimpleOverlayResult.Success)
            {
                throw new InvalidOperationException($"Failed to set texture: {result}");
            }
        }

        private void ThrowIfDisposed()
        {
            if (_disposed)
            {
                throw new ObjectDisposedException(nameof(SimpleOverlay));
            }
        }

        /// <summary>
        /// Disposes the overlay and releases native resources
        /// </summary>
        public void Dispose()
        {
            if (!_disposed)
            {
                if (_handle != IntPtr.Zero)
                {
                    SimpleOverlayNative.SimpleOverlay_Destroy(_handle);
                    _handle = IntPtr.Zero;
                }
                _disposed = true;
            }
            GC.SuppressFinalize(this);
        }

        ~SimpleOverlay()
        {
            Dispose();
        }
    }
}
