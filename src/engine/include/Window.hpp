#pragma once

/// @file
/// @brief Minimal Wayland-backend drawing window.
///
/// A tiny wrapper designed to use basic Wayland features as a blackbox. Open a
/// window, draw simple shapes into it, read keyboard and mouse state, step
/// frames at a fixed rate. All Wayland and xkbcommon details are hidden 
/// (for your own sanity).
///
/// You should NOT need to modify this file, BUT feel free to 
/// add new features if it fits your needs, especially
/// when it comes to input handling and drawing primitives!
///
///
/// Typical use:
/// @code
///   gfx::Window win("My Game", 800, 600);
///   while (win.isOpen()) {
///       win.pollEvents();
///       win.clear({30, 30, 30});
///       win.fillRect(100, 100, 40, 40, {255, 255, 255});
///       win.present();
///       win.waitForNextFrame(60);
///   }
/// @endcode

#include <cstdint>
#include <memory>
#include <string>

namespace gfx {

// -----------------------------------------------------------------------------
// Color
// -----------------------------------------------------------------------------

/// 8-bit-per-channel RGBA color. Alpha 255 is opaque.
struct Color {
    uint8_t r{0}, g{0}, b{0}, a{255};

    /// Construct an opaque black color.
    constexpr Color() = default;

    /// Construct from r, g, b (and optional alpha).
    constexpr Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}
};

// -----------------------------------------------------------------------------
// Keys and mouse buttons
// -----------------------------------------------------------------------------

/// Keyboard keys understood by the wrapper. Any other key reports as Unknown.
enum class Key {
    Unknown = 0,
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Digits (top row)
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    // Named keys
    Escape, Space, Enter, Tab, Backspace,
    Left, Right, Up, Down,
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt,
    _Count ///< Sentinel; not a real key.
};

/// Mouse buttons.
enum class MouseButton { Left, Right, Middle };

// -----------------------------------------------------------------------------
// Window
// -----------------------------------------------------------------------------

/// A simple window where you can draw pixels into.
///
/// The window is **fixed-size**: the compositor cannot resize it. Drawing
/// uses a top-left-origin coordinate system where (0, 0) is the top-left
/// pixel and (width-1, height-1) is the bottom-right. Out-of-bounds draws
/// are clipped.
class Window {
public:
    /// Open a window. Shortly blocks the program while the compositor 
    /// performs the initial Wayland handshake.
    /// @param title  Title shown by the compositor (window manager decoration).
    /// @param width  Width in pixels. Must be > 0.
    /// @param height Height in pixels. Must be > 0.
    /// @throws std::runtime_error on any fatal initialization failure,
    ///         with a message explaining the cause.
    Window(std::string title, int width, int height);

    /// Close the window and release all resources.
    ~Window();

    // Non-copyable & movable.
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) noexcept;
    Window& operator=(Window&&) noexcept;

    // -- Frames management ----------------------------------------------------

    /// True until the user (or compositor) requests the window to close,
    /// or close() is called.
    [[nodiscard]] bool isOpen() const;

    /// Fill the entire back buffer with @p c. Call at the start of each frame
    /// unless you want a persistent trail effect.
    void clear(Color c);

    /// Show the pixels you have drawn since the last present().
    void present();

    /// Process any pending window and input events. Non-blocking; call
    /// exactly once per frame. Edge-triggered key state (pressed/released)
    /// is reset at the start of this call and populated from newly-arrived
    /// events.
    void pollEvents();

    /// Requests the window to close. @ref isOpen will return false after 
    /// the next event loop iteration.
    void close();

    // -- Drawing primitives ---------------------------------------------------
    // All coordinates are pixel-space, top-left origin. All draws are
    // silently clipped to the window bounds.

    /// Set a single pixel.
    void drawPixel(int x, int y, Color c);

    /// Fill a solid rectangle of size @p w × @p h with top-left at (@p x, @p y).
    void fillRect(int x, int y, int w, int h, Color c);

    /// Fill a rectangle with alpha blending (semi-transparent)
    void fillOverlay(int x, int y, int w, int h, Color c);

    /// Draw a 1-pixel rectangle outline.
    void drawRect(int x, int y, int w, int h, Color c);

    /// Draw a 1-pixel line between two points (Bresenham's algorithm).
    void drawLine(int x0, int y0, int x1, int y1, Color c);

    /// Fill a disk of radius @p radius centered at (@p cx, @p cy).
    void fillCircle(int cx, int cy, int radius, Color c);

    /// Draw a 1-pixel circle outline of radius @p radius centered at (@p cx, @p cy).
    void drawCircle(int cx, int cy, int radius, Color c);

    // -- Input ----------------------------------------------------------------

    /// True while @p k is physically held down.
    [[nodiscard]] bool isKeyDown(Key k) const;

    /// True on the single frame @p k went from up to down.
    [[nodiscard]] bool wasKeyPressed(Key k) const;

    /// True on the single frame @p k went from down to up.
    [[nodiscard]] bool wasKeyReleased(Key k) const;

    /// True while @p b is physically held down.
    [[nodiscard]] bool isMouseDown(MouseButton b) const;

    /// Current mouse X position in window pixels. -1 if the pointer is not
    /// over the window.
    [[nodiscard]] int mouseX() const;

    /// Current mouse Y position in window pixels. -1 if the pointer is not
    /// over the window.
    [[nodiscard]] int mouseY() const;

    // -- Geometry -------------------------------------------------------------

    /// Window width in pixels (fixed after construction).
    [[nodiscard]] int width() const;

    /// Window height in pixels (fixed after construction).
    [[nodiscard]] int height() const;

    // -- Frame pacing ---------------------------------------------------------

    /// Sleep just long enough to achieve @p fps frames per second.
    /// The schedule is drift-free: successive calls aim at multiples of
    /// (1 / fps), so occasional overruns do not accumulate.
    void waitForNextFrame(int fps);

    // Forward-declared publicly only so the implementation's file-scope
    // code can name it. The definition is private to Window.cpp; the type
    // remains incomplete and is therefore unusable for safe exposure.
    struct Impl;

private:
    std::unique_ptr<Impl> m_impl;
};

} // namespace gfx
