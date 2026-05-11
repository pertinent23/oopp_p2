// Window.cpp - implementation of gfx::Window.
// All Wayland, XKB, and SHM details are confined here.
//
// You should NOT need to modify this file, BUT feel free to 
// add new features if it fits your needs, especially
// when it comes to input handling and drawing primitives!

#include "Window.hpp"

// -- Wayland / XKB -----------------------------------------------------------
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include "xdg-shell-client-protocol.h"

// -- POSIX -------------------------------------------------------------------
#include <fcntl.h>
#include <poll.h>
#include <sys/mman.h>
#include <unistd.h>

// -- Standard library --------------------------------------------------------
#include <algorithm>
#include <array>
#include <bitset>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace gfx { // Buckle up!

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

namespace {

/// Pack a gfx::Color into a 32-bit value in WL_SHM_FORMAT_ARGB8888 layout.
inline uint32_t packARGB(Color c) noexcept {
    return (uint32_t(c.a) << 24) |
           (uint32_t(c.r) << 16) |
           (uint32_t(c.g) << 8)  |
            uint32_t(c.b);
}

/// Create a file descriptor backed by anonymous shared memory of @p size bytes.
int createShmFd(size_t size) {
    int fd = memfd_create("gfx_shm", MFD_CLOEXEC);
    if (fd < 0)
        throw std::runtime_error(std::string("memfd_create failed: ") + std::strerror(errno));
    if (ftruncate(fd, off_t(size)) < 0) {
        int err = errno;
        ::close(fd);
        throw std::runtime_error(std::string("ftruncate failed: ") + std::strerror(err));
    }
    return fd;
}

/// Convert an XKB keysym to our public Key enum. Returns Key::Unknown if the
/// keysym isn't one we care about.
Key keysymToKey(xkb_keysym_t sym) {
    switch (sym) {
    // Letters (both cases map to the same Key)
    case XKB_KEY_a: case XKB_KEY_A: return Key::A;
    case XKB_KEY_b: case XKB_KEY_B: return Key::B;
    case XKB_KEY_c: case XKB_KEY_C: return Key::C;
    case XKB_KEY_d: case XKB_KEY_D: return Key::D;
    case XKB_KEY_e: case XKB_KEY_E: return Key::E;
    case XKB_KEY_f: case XKB_KEY_F: return Key::F;
    case XKB_KEY_g: case XKB_KEY_G: return Key::G;
    case XKB_KEY_h: case XKB_KEY_H: return Key::H;
    case XKB_KEY_i: case XKB_KEY_I: return Key::I;
    case XKB_KEY_j: case XKB_KEY_J: return Key::J;
    case XKB_KEY_k: case XKB_KEY_K: return Key::K;
    case XKB_KEY_l: case XKB_KEY_L: return Key::L;
    case XKB_KEY_m: case XKB_KEY_M: return Key::M;
    case XKB_KEY_n: case XKB_KEY_N: return Key::N;
    case XKB_KEY_o: case XKB_KEY_O: return Key::O;
    case XKB_KEY_p: case XKB_KEY_P: return Key::P;
    case XKB_KEY_q: case XKB_KEY_Q: return Key::Q;
    case XKB_KEY_r: case XKB_KEY_R: return Key::R;
    case XKB_KEY_s: case XKB_KEY_S: return Key::S;
    case XKB_KEY_t: case XKB_KEY_T: return Key::T;
    case XKB_KEY_u: case XKB_KEY_U: return Key::U;
    case XKB_KEY_v: case XKB_KEY_V: return Key::V;
    case XKB_KEY_w: case XKB_KEY_W: return Key::W;
    case XKB_KEY_x: case XKB_KEY_X: return Key::X;
    case XKB_KEY_y: case XKB_KEY_Y: return Key::Y;
    case XKB_KEY_z: case XKB_KEY_Z: return Key::Z;
    // Digits
    case XKB_KEY_0: return Key::Num0;
    case XKB_KEY_1: return Key::Num1;
    case XKB_KEY_2: return Key::Num2;
    case XKB_KEY_3: return Key::Num3;
    case XKB_KEY_4: return Key::Num4;
    case XKB_KEY_5: return Key::Num5;
    case XKB_KEY_6: return Key::Num6;
    case XKB_KEY_7: return Key::Num7;
    case XKB_KEY_8: return Key::Num8;
    case XKB_KEY_9: return Key::Num9;
    // Named keys
    case XKB_KEY_Escape:    return Key::Escape;
    case XKB_KEY_space:     return Key::Space;
    case XKB_KEY_Return:    return Key::Enter;
    case XKB_KEY_Tab:       return Key::Tab;
    case XKB_KEY_BackSpace: return Key::Backspace;
    case XKB_KEY_Left:      return Key::Left;
    case XKB_KEY_Right:     return Key::Right;
    case XKB_KEY_Up:        return Key::Up;
    case XKB_KEY_Down:      return Key::Down;
    case XKB_KEY_Shift_L:   return Key::LShift;
    case XKB_KEY_Shift_R:   return Key::RShift;
    case XKB_KEY_Control_L: return Key::LCtrl;
    case XKB_KEY_Control_R: return Key::RCtrl;
    case XKB_KEY_Alt_L:     return Key::LAlt;
    case XKB_KEY_Alt_R:     return Key::RAlt;
    default:                return Key::Unknown;
    }
}

} // namespace

// -----------------------------------------------------------------------------
// Impl
// This is the private implementation of gfx::Window. All Wayland, XKB, and SHM
// details are confined here. The public Window class is just a thin wrapper
// that forwards to this Impl via a unique_ptr, so the header file is clean and
// obscure details don't leak out to users of the class.
// -----------------------------------------------------------------------------

struct Window::Impl {
    // -- Wayland globals -----------------------------------------------------
    wl_display*    display    = nullptr;
    wl_registry*   registry   = nullptr;
    wl_compositor* compositor = nullptr;
    wl_shm*        shm        = nullptr;
    wl_seat*       seat       = nullptr;
    wl_keyboard*   keyboard   = nullptr;
    wl_pointer*    pointer    = nullptr;
    xdg_wm_base*   wmBase     = nullptr;

    // -- Surface hierarchy ---------------------------------------------------
    wl_surface*    surface    = nullptr;
    xdg_surface*   xdgSurface = nullptr;
    xdg_toplevel*  toplevel   = nullptr;

    // -- XKB keyboard state --------------------------------------------------
    xkb_context*   xkbCtx   = nullptr;
    xkb_keymap*    xkbKmap  = nullptr;
    xkb_state*     xkbState = nullptr;

    // -- Shared memory double buffer -----------------------------------------
    struct ShmBuf {
        wl_buffer* buffer = nullptr;
        uint8_t*   data   = nullptr;
        size_t     size   = 0;
        bool       busy   = false;
    };
    std::array<ShmBuf, 2> shmBufs{};

    // -- Back buffer (game is written in here) -------------------------------
    std::vector<uint32_t> back; ///< width * height pixels, packed ARGB8888.

    // -- Window state --------------------------------------------------------
    std::string title;
    int  width      = 0;
    int  height     = 0;
    bool open       = false;
    bool configured = false;

    // -- Input state ---------------------------------------------------------
    using KeyBits = std::bitset<size_t(Key::_Count)>;
    KeyBits keysDown{};
    KeyBits keysPressed{};
    KeyBits keysReleased{};
    std::bitset<3> mouseDown{};
    int mouseXpos = -1;
    int mouseYpos = -1;

    // -- Frame pacing --------------------------------------------------------
    std::chrono::steady_clock::time_point nextFrame{};
    bool pacerStarted = false;

    // -- Lifecycle -----------------------------------------------------------
    void init(std::string t, int w, int h);
    void teardown() noexcept;

    // -- Buffer management ---------------------------------------------------
    void allocShmBuf(ShmBuf& b);
    ShmBuf* pickFreeBuf();

    // -- Listener trampolines (static; dispatch via data pointer) ------------
    static void onRegistryGlobal(void*, wl_registry*, uint32_t, const char*, uint32_t);
    static void onRegistryRemove(void*, wl_registry*, uint32_t);
    static void onWmBasePing(void*, xdg_wm_base*, uint32_t);
    static void onXdgSurfaceConfigure(void*, xdg_surface*, uint32_t);
    static void onToplevelConfigure(void*, xdg_toplevel*, int32_t, int32_t, wl_array*);
    static void onToplevelClose(void*, xdg_toplevel*);
    static void onBufferRelease(void*, wl_buffer*);
    static void onSeatCaps(void*, wl_seat*, uint32_t);
    static void onSeatName(void*, wl_seat*, const char*);
    static void onKbKeymap(void*, wl_keyboard*, uint32_t, int32_t, uint32_t);
    static void onKbEnter(void*, wl_keyboard*, uint32_t, wl_surface*, wl_array*);
    static void onKbLeave(void*, wl_keyboard*, uint32_t, wl_surface*);
    static void onKbKey(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t);
    static void onKbMods(void*, wl_keyboard*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    static void onPtrEnter(void*, wl_pointer*, uint32_t, wl_surface*, wl_fixed_t, wl_fixed_t);
    static void onPtrLeave(void*, wl_pointer*, uint32_t, wl_surface*);
    static void onPtrMotion(void*, wl_pointer*, uint32_t, wl_fixed_t, wl_fixed_t);
    static void onPtrButton(void*, wl_pointer*, uint32_t, uint32_t, uint32_t, uint32_t);
    static void onPtrAxis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t);
};

// -----------------------------------------------------------------------------
// Listener structs
// -----------------------------------------------------------------------------
// IMPORTANT: on newer wayland protocols, these structs have extra fields
// (e.g. xdg_toplevel_listener::configure_bounds / wm_capabilities,
//  wl_pointer_listener::frame / axis_value120 / axis_relative_direction).
//
// -> ALL GLOBALS ARE BOUND AT VERSION 1, so these newer fields are never 
// sent by the compositor.
// 
// HOWEVER: we still fill every slot with a no-op lambda trampoline so that:
//  (a) if the dispatcher ever indexes beyond what we handle it still finds
//      a valid callback;
//  (b) -Wmissing-field-initializers stays quiet across versions.
//
// We use designated initializers by named field so the mapping is explicit
// and robust even if future protocol XML reorders anything (it won't, but
// being explicit is cheap).

namespace {

// Registry
const wl_registry_listener kRegistryListener = {
    .global        = &Window::Impl::onRegistryGlobal,
    .global_remove = &Window::Impl::onRegistryRemove,
};

// XDG wm_base
const xdg_wm_base_listener kWmBaseListener = {
    .ping = &Window::Impl::onWmBasePing,
};

// XDG surface
const xdg_surface_listener kXdgSurfaceListener = {
    .configure = &Window::Impl::onXdgSurfaceConfigure,
};

// XDG toplevel - fill every slot with a stub to survive newer protocol versions.
const xdg_toplevel_listener kToplevelListener = {
    .configure        = &Window::Impl::onToplevelConfigure,
    .close            = &Window::Impl::onToplevelClose,
    .configure_bounds = [](void*, xdg_toplevel*, int32_t, int32_t) {},
    .wm_capabilities  = [](void*, xdg_toplevel*, wl_array*) {},
};

// wl_buffer
const wl_buffer_listener kBufferListener = {
    .release = &Window::Impl::onBufferRelease,
};

// Seat
const wl_seat_listener kSeatListener = {
    .capabilities = &Window::Impl::onSeatCaps,
    .name         = &Window::Impl::onSeatName,
};

// Keyboard - repeat_info is v4; we bind seat at v1 so it should never fire,
// but we provide a stub anyway.
const wl_keyboard_listener kKeyboardListener = {
    .keymap      = &Window::Impl::onKbKeymap,
    .enter       = &Window::Impl::onKbEnter,
    .leave       = &Window::Impl::onKbLeave,
    .key         = &Window::Impl::onKbKey,
    .modifiers   = &Window::Impl::onKbMods,
    .repeat_info = [](void*, wl_keyboard*, int32_t, int32_t) {},
};

// Pointer - frame/axis_source/etc. are v5+; we bind seat at v1. Provide
// stubs for all optional slots for safety.
const wl_pointer_listener kPointerListener = {
    .enter   = &Window::Impl::onPtrEnter,
    .leave   = &Window::Impl::onPtrLeave,
    .motion  = &Window::Impl::onPtrMotion,
    .button  = &Window::Impl::onPtrButton,
    .axis    = &Window::Impl::onPtrAxis,
    .frame                   = [](void*, wl_pointer*) {},
    .axis_source             = [](void*, wl_pointer*, uint32_t) {},
    .axis_stop               = [](void*, wl_pointer*, uint32_t, uint32_t) {},
    .axis_discrete           = [](void*, wl_pointer*, uint32_t, int32_t) {},
    .axis_value120           = [](void*, wl_pointer*, uint32_t, int32_t) {},
    .axis_relative_direction = [](void*, wl_pointer*, uint32_t, uint32_t) {},
};

} // namespace

// -----------------------------------------------------------------------------
// Init / teardown
// -----------------------------------------------------------------------------

void Window::Impl::init(std::string t, int w, int h) {
    if (w <= 0 || h <= 0)
        throw std::runtime_error("Window: width and height must be positive");

    title  = std::move(t);
    width  = w;
    height = h;

    // Connect
    display = wl_display_connect(nullptr);
    if (!display)
        throw std::runtime_error(
            "Failed to connect to Wayland display "
            "(is WAYLAND_DISPLAY set and are you running in a Wayland session?)");

    // Registry: first roundtrip populates globals
    registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &kRegistryListener, this);
    wl_display_roundtrip(display);

    if (!compositor) throw std::runtime_error("Wayland: no wl_compositor global");
    if (!shm)        throw std::runtime_error("Wayland: no wl_shm global");
    if (!wmBase)     throw std::runtime_error("Wayland: no xdg_wm_base global");

    xdg_wm_base_add_listener(wmBase, &kWmBaseListener, this);

    // Second roundtrip: deliver seat capabilities (pointer/keyboard creation)
    wl_display_roundtrip(display);

    // Surface hierarchy
    surface    = wl_compositor_create_surface(compositor);
    xdgSurface = xdg_wm_base_get_xdg_surface(wmBase, surface);
    xdg_surface_add_listener(xdgSurface, &kXdgSurfaceListener, this);
    toplevel = xdg_surface_get_toplevel(xdgSurface);
    xdg_toplevel_add_listener(toplevel, &kToplevelListener, this);

    xdg_toplevel_set_title(toplevel, title.c_str());
    xdg_toplevel_set_app_id(toplevel, "gfx.Window");
    xdg_toplevel_set_fullscreen(toplevel, nullptr); // <-- Mode plein écran activé

    // Protocol: must commit an empty surface first, wait for configure, ack,
    // THEN attach the first buffer.
    wl_surface_commit(surface);
    while (!configured) {
        if (wl_display_dispatch(display) < 0)
            throw std::runtime_error("Wayland: dispatch failed during initial configure");
    }

    // Allocate both SHM buffers
    for (auto& b : shmBufs) allocShmBuf(b);

    // Back buffer
    back.assign(size_t(width) * size_t(height), packARGB(Color{0, 0, 0}));

    open = true;
}

void Window::Impl::teardown() noexcept {
    // Reverse construction order.
    for (auto& b : shmBufs) {
        if (b.buffer) wl_buffer_destroy(b.buffer);
        if (b.data && b.size) munmap(b.data, b.size);
        b = {};
    }
    if (xkbState)   { xkb_state_unref(xkbState);   xkbState = nullptr; }
    if (xkbKmap)    { xkb_keymap_unref(xkbKmap);   xkbKmap  = nullptr; }
    if (xkbCtx)     { xkb_context_unref(xkbCtx);   xkbCtx   = nullptr; }
    if (toplevel)   { xdg_toplevel_destroy(toplevel);   toplevel   = nullptr; }
    if (xdgSurface) { xdg_surface_destroy(xdgSurface);  xdgSurface = nullptr; }
    if (surface)    { wl_surface_destroy(surface);      surface    = nullptr; }
    if (keyboard)   { wl_keyboard_destroy(keyboard);    keyboard   = nullptr; }
    if (pointer)    { wl_pointer_destroy(pointer);      pointer    = nullptr; }
    if (seat)       { wl_seat_destroy(seat);            seat       = nullptr; }
    if (wmBase)     { xdg_wm_base_destroy(wmBase);      wmBase     = nullptr; }
    if (shm)        { wl_shm_destroy(shm);              shm        = nullptr; }
    if (compositor) { wl_compositor_destroy(compositor); compositor = nullptr; }
    if (registry)   { wl_registry_destroy(registry);    registry   = nullptr; }
    if (display)    { wl_display_disconnect(display);   display    = nullptr; }
}

// -----------------------------------------------------------------------------
// SHM buffer allocation
// -----------------------------------------------------------------------------

void Window::Impl::allocShmBuf(ShmBuf& b) {
    const int stride = width * 4;
    const size_t sz  = size_t(stride) * size_t(height);

    int fd = createShmFd(sz);
    void* map = mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        int err = errno;
        ::close(fd);
        throw std::runtime_error(std::string("mmap failed: ") + std::strerror(err));
    }

    wl_shm_pool* pool = wl_shm_create_pool(shm, fd, int(sz));
    b.buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride,
                                         WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    ::close(fd); // pool held its own reference during creation

    b.data = static_cast<uint8_t*>(map);
    b.size = sz;
    b.busy = false;
    wl_buffer_add_listener(b.buffer, &kBufferListener, this);
}

Window::Impl::ShmBuf* Window::Impl::pickFreeBuf() {
    for (auto& b : shmBufs)
        if (!b.busy) return &b;
    return nullptr;
}

// -----------------------------------------------------------------------------
// Listener trampolines
// -----------------------------------------------------------------------------

void Window::Impl::onRegistryGlobal(void* data, wl_registry* reg,
                                    uint32_t name, const char* interface,
                                    uint32_t /*version*/) {
    auto* self = static_cast<Impl*>(data);
    // All interfaces bound at version 1.
    if (std::strcmp(interface, wl_compositor_interface.name) == 0) {
        self->compositor = static_cast<wl_compositor*>(
            wl_registry_bind(reg, name, &wl_compositor_interface, 1));
    } else if (std::strcmp(interface, wl_shm_interface.name) == 0) {
        self->shm = static_cast<wl_shm*>(
            wl_registry_bind(reg, name, &wl_shm_interface, 1));
    } else if (std::strcmp(interface, xdg_wm_base_interface.name) == 0) {
        self->wmBase = static_cast<xdg_wm_base*>(
            wl_registry_bind(reg, name, &xdg_wm_base_interface, 1));
    } else if (std::strcmp(interface, wl_seat_interface.name) == 0) {
        self->seat = static_cast<wl_seat*>(
            wl_registry_bind(reg, name, &wl_seat_interface, 1));
        wl_seat_add_listener(self->seat, &kSeatListener, self);
    }
}

void Window::Impl::onRegistryRemove(void*, wl_registry*, uint32_t) {
    // Ignore hot-unplug.
}

void Window::Impl::onWmBasePing(void*, xdg_wm_base* base, uint32_t serial) {
    xdg_wm_base_pong(base, serial);
}

void Window::Impl::onXdgSurfaceConfigure(void* data, xdg_surface* surf,
                                         uint32_t serial) {
    auto* self = static_cast<Impl*>(data);
    xdg_surface_ack_configure(surf, serial);
    self->configured = true;
}

void Window::Impl::onToplevelConfigure(void* data, xdg_toplevel*,
                                       int32_t w, int32_t h,
                                       wl_array* /*states*/) {
    // Si Wayland nous donne une taille (en plein écran), on l'adopte !
    auto* self = static_cast<Impl*>(data);
    if (w > 0 && h > 0) {
        self->width = w;
        self->height = h;
    }
}

void Window::Impl::onToplevelClose(void* data, xdg_toplevel*) {
    auto* self = static_cast<Impl*>(data);
    self->open = false;
}

void Window::Impl::onBufferRelease(void* data, wl_buffer* buf) {
    auto* self = static_cast<Impl*>(data);
    for (auto& b : self->shmBufs)
        if (b.buffer == buf) { b.busy = false; break; }
}

void Window::Impl::onSeatCaps(void* data, wl_seat* seat, uint32_t caps) {
    auto* self = static_cast<Impl*>(data);
    const bool hasKb  = caps & WL_SEAT_CAPABILITY_KEYBOARD;
    const bool hasPtr = caps & WL_SEAT_CAPABILITY_POINTER;

    if (hasKb && !self->keyboard) {
        self->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(self->keyboard, &kKeyboardListener, self);
    } else if (!hasKb && self->keyboard) {
        wl_keyboard_destroy(self->keyboard);
        self->keyboard = nullptr;
    }
    if (hasPtr && !self->pointer) {
        self->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(self->pointer, &kPointerListener, self);
    } else if (!hasPtr && self->pointer) {
        wl_pointer_destroy(self->pointer);
        self->pointer = nullptr;
    }
}

void Window::Impl::onSeatName(void*, wl_seat*, const char*) {}

void Window::Impl::onKbKeymap(void* data, wl_keyboard*, uint32_t format,
                              int32_t fd, uint32_t size) {
    auto* self = static_cast<Impl*>(data);
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) { ::close(fd); return; }

    char* map = static_cast<char*>(
        mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (map == MAP_FAILED) { ::close(fd); return; }

    if (!self->xkbCtx) self->xkbCtx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    xkb_keymap* km = xkb_keymap_new_from_string(
        self->xkbCtx, map, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map, size);
    ::close(fd);
    if (!km) return;

    xkb_state* st = xkb_state_new(km);
    if (!st) { xkb_keymap_unref(km); return; }

    if (self->xkbState) xkb_state_unref(self->xkbState);
    if (self->xkbKmap)  xkb_keymap_unref(self->xkbKmap);
    self->xkbKmap  = km;
    self->xkbState = st;
}

void Window::Impl::onKbEnter(void* data, wl_keyboard*, uint32_t,
                             wl_surface*, wl_array*) {
    auto* self = static_cast<Impl*>(data);
    // Clear state on focus gain to avoid stale held-key bits.
    self->keysDown.reset();
}

void Window::Impl::onKbLeave(void* data, wl_keyboard*, uint32_t, wl_surface*) {
    auto* self = static_cast<Impl*>(data);
    self->keysDown.reset();
}

void Window::Impl::onKbKey(void* data, wl_keyboard*, uint32_t /*serial*/,
                           uint32_t /*time*/, uint32_t key, uint32_t state) {
    auto* self = static_cast<Impl*>(data);
    if (!self->xkbState) return;

    // evdev -> XKB keycode offset
    const xkb_keycode_t keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym(self->xkbState, keycode);
    Key k = keysymToKey(sym);
    if (k == Key::Unknown) return;

    const size_t idx = size_t(k);
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        if (!self->keysDown.test(idx)) self->keysPressed.set(idx);
        self->keysDown.set(idx);
    } else {
        if (self->keysDown.test(idx)) self->keysReleased.set(idx);
        self->keysDown.reset(idx);
    }
}

void Window::Impl::onKbMods(void* data, wl_keyboard*, uint32_t,
                            uint32_t mods_depressed, uint32_t mods_latched,
                            uint32_t mods_locked, uint32_t group) {
    auto* self = static_cast<Impl*>(data);
    if (self->xkbState)
        xkb_state_update_mask(self->xkbState, mods_depressed, mods_latched,
                              mods_locked, 0, 0, group);
}

void Window::Impl::onPtrEnter(void* data, wl_pointer*, uint32_t,
                              wl_surface*, wl_fixed_t sx, wl_fixed_t sy) {
    auto* self = static_cast<Impl*>(data);
    self->mouseXpos = wl_fixed_to_int(sx);
    self->mouseYpos = wl_fixed_to_int(sy);
}

void Window::Impl::onPtrLeave(void* data, wl_pointer*, uint32_t, wl_surface*) {
    auto* self = static_cast<Impl*>(data);
    self->mouseXpos = -1;
    self->mouseYpos = -1;
    self->mouseDown.reset();
}

void Window::Impl::onPtrMotion(void* data, wl_pointer*, uint32_t,
                               wl_fixed_t sx, wl_fixed_t sy) {
    auto* self = static_cast<Impl*>(data);
    self->mouseXpos = wl_fixed_to_int(sx);
    self->mouseYpos = wl_fixed_to_int(sy);
}

void Window::Impl::onPtrButton(void* data, wl_pointer*, uint32_t, uint32_t,
                               uint32_t button, uint32_t state) {
    auto* self = static_cast<Impl*>(data);
    size_t idx;
    switch (button) {
        case 0x110: idx = 0; break; // BTN_LEFT
        case 0x111: idx = 1; break; // BTN_RIGHT
        case 0x112: idx = 2; break; // BTN_MIDDLE
        default: return;
    }
    self->mouseDown.set(idx, state == WL_POINTER_BUTTON_STATE_PRESSED);
}

void Window::Impl::onPtrAxis(void*, wl_pointer*, uint32_t, uint32_t, wl_fixed_t) {
    // Not exposed in the public API.
}

// -----------------------------------------------------------------------------
// Window: lifecycle
// -----------------------------------------------------------------------------

Window::Window(std::string title, int width, int height)
    : m_impl(std::make_unique<Impl>()) {
    m_impl->init(std::move(title), width, height);
}

Window::~Window() {
    if (m_impl) m_impl->teardown();
}

Window::Window(Window&&) noexcept            = default;
Window& Window::operator=(Window&&) noexcept = default;

// -----------------------------------------------------------------------------
// Window: state queries
// -----------------------------------------------------------------------------

bool Window::isOpen() const          { return m_impl && m_impl->open; }
int  Window::width() const           { return m_impl ? m_impl->width  : 0; }
int  Window::height() const          { return m_impl ? m_impl->height : 0; }

void Window::close() {
    if (m_impl) m_impl->open = false;
}

// -----------------------------------------------------------------------------
// Window: input
// -----------------------------------------------------------------------------

bool Window::isKeyDown(Key k) const {
    return m_impl && m_impl->keysDown.test(size_t(k));
}
bool Window::wasKeyPressed(Key k) const {
    return m_impl && m_impl->keysPressed.test(size_t(k));
}
bool Window::wasKeyReleased(Key k) const {
    return m_impl && m_impl->keysReleased.test(size_t(k));
}
bool Window::isMouseDown(MouseButton b) const {
    return m_impl && m_impl->mouseDown.test(size_t(b));
}
int  Window::mouseX() const { return m_impl ? m_impl->mouseXpos : -1; }
int  Window::mouseY() const { return m_impl ? m_impl->mouseYpos : -1; }

// -----------------------------------------------------------------------------
// Window: drawing
// -----------------------------------------------------------------------------

void Window::clear(Color c) {
    if (!m_impl) return;
    const uint32_t pix = packARGB(c);
    std::fill(m_impl->back.begin(), m_impl->back.end(), pix);
}

void Window::drawPixel(int x, int y, Color c) {
    if (!m_impl) return;
    if (x < 0 || y < 0 || x >= m_impl->width || y >= m_impl->height) return;
    m_impl->back[size_t(y) * size_t(m_impl->width) + size_t(x)] = packARGB(c);
}

void Window::fillRect(int x, int y, int w, int h, Color c) {
    if (!m_impl || w <= 0 || h <= 0) return;
    const int W = m_impl->width, H = m_impl->height;
    const int x0 = std::max(0, x);
    const int y0 = std::max(0, y);
    const int x1 = std::min(W, x + w);
    const int y1 = std::min(H, y + h);
    if (x0 >= x1 || y0 >= y1) return;

    const uint32_t pix = packARGB(c);
    uint32_t* const base = m_impl->back.data();
    for (int yy = y0; yy < y1; ++yy) {
        uint32_t* row = base + size_t(yy) * size_t(W);
        std::fill(row + x0, row + x1, pix);
    }
}

void Window::fillOverlay(int x, int y, int w, int h, Color c) {
    if (!m_impl || w <= 0 || h <= 0) return;
    const int W = m_impl->width, H = m_impl->height;
    const int x0 = std::max(0, x);
    const int y0 = std::max(0, y);
    const int x1 = std::min(W, x + w);
    const int y1 = std::min(H, y + h);
    if (x0 >= x1 || y0 >= y1) return;

    uint32_t* const base = m_impl->back.data();
    uint32_t alpha = c.a;
    uint32_t invAlpha = 255 - alpha;
    
    for (int yy = y0; yy < y1; ++yy) {
        uint32_t* row = base + size_t(yy) * size_t(W);
        for (int xx = x0; xx < x1; ++xx) {
            uint32_t dest = row[xx];
            uint32_t r = (((dest >> 16) & 0xFF) * invAlpha + c.r * alpha) / 255;
            uint32_t g = (((dest >> 8) & 0xFF) * invAlpha + c.g * alpha) / 255;
            uint32_t b = ((dest & 0xFF) * invAlpha + c.b * alpha) / 255;
            row[xx] = (255 << 24) | (r << 16) | (g << 8) | b;
        }
    }
}

void Window::drawRect(int x, int y, int w, int h, Color c) {
    if (!m_impl || w <= 0 || h <= 0) return;
    // Top and bottom
    fillRect(x,           y,           w, 1, c);
    fillRect(x,           y + h - 1,   w, 1, c);
    // Left and right (excluding corners already drawn)
    if (h > 2) {
        fillRect(x,           y + 1,   1, h - 2, c);
        fillRect(x + w - 1,   y + 1,   1, h - 2, c);
    }
}

void Window::drawLine(int x0, int y0, int x1, int y1, Color c) {
    if (!m_impl) return;
    int dx =  std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        drawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void Window::fillCircle(int cx, int cy, int radius, Color c) {
    if (!m_impl || radius <= 0) return;
    const int W = m_impl->width, H = m_impl->height;
    const int r2 = radius * radius;
    const int yLo = std::max(0, cy - radius);
    const int yHi = std::min(H - 1, cy + radius);
    const uint32_t pix = packARGB(c);
    uint32_t* const base = m_impl->back.data();

    for (int y = yLo; y <= yHi; ++y) {
        const int dy = y - cy;
        const int dx2 = r2 - dy * dy;
        if (dx2 < 0) continue;
        const int dx = int(std::sqrt(double(dx2)));
        const int x0 = std::max(0, cx - dx);
        const int x1 = std::min(W - 1, cx + dx);
        if (x0 > x1) continue;
        uint32_t* row = base + size_t(y) * size_t(W);
        std::fill(row + x0, row + x1 + 1, pix);
    }
}

void Window::drawCircle(int cx, int cy, int radius, Color c) {
    if (!m_impl || radius <= 0) return;
    // Bresenham midpoint circle.
    int x = radius, y = 0, err = 1 - radius;
    while (x >= y) {
        drawPixel(cx + x, cy + y, c);
        drawPixel(cx + y, cy + x, c);
        drawPixel(cx - y, cy + x, c);
        drawPixel(cx - x, cy + y, c);
        drawPixel(cx - x, cy - y, c);
        drawPixel(cx - y, cy - x, c);
        drawPixel(cx + y, cy - x, c);
        drawPixel(cx + x, cy - y, c);
        ++y;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            --x;
            err += 2 * (y - x) + 1;
        }
    }
}

// -----------------------------------------------------------------------------
// Window: present + event pump
// -----------------------------------------------------------------------------

void Window::present() {
    if (!m_impl || !m_impl->open) return;
    auto* buf = m_impl->pickFreeBuf();
    if (!buf) {
        // Both buffers busy: drop this frame rather than stall.
        return;
    }
    std::memcpy(buf->data, m_impl->back.data(),
                size_t(m_impl->width) * size_t(m_impl->height) * 4);
    buf->busy = true;
    wl_surface_attach(m_impl->surface, buf->buffer, 0, 0);
    wl_surface_damage(m_impl->surface, 0, 0,
                      m_impl->width, m_impl->height);
    wl_surface_commit(m_impl->surface);
    wl_display_flush(m_impl->display);
}

void Window::pollEvents() {
    if (!m_impl) return;

    // Edge state resets each frame.
    m_impl->keysPressed.reset();
    m_impl->keysReleased.reset();

    // Basic non-blocking Wayland dispatch pattern: 
    // prepare_read, flush, poll, read_events, and dispatch.
    while (wl_display_prepare_read(m_impl->display) != 0)
        wl_display_dispatch_pending(m_impl->display);
    wl_display_flush(m_impl->display);

    pollfd pfd{ wl_display_get_fd(m_impl->display), POLLIN, 0 };
    int ret = poll(&pfd, 1, 0);
    if (ret > 0 && (pfd.revents & POLLIN)) {
        if (wl_display_read_events(m_impl->display) < 0) {
            m_impl->open = false;
            return;
        }
    } else {
        wl_display_cancel_read(m_impl->display);
    }
    wl_display_dispatch_pending(m_impl->display);
}

// -----------------------------------------------------------------------------
// Window: frame pacer
// -----------------------------------------------------------------------------

void Window::waitForNextFrame(int fps) {
    if (!m_impl || fps <= 0) return;
    using namespace std::chrono;
    const auto period = duration_cast<steady_clock::duration>(
        duration<double>(1.0 / double(fps)));
    const auto now = steady_clock::now();
    if (!m_impl->pacerStarted) {
        m_impl->nextFrame    = now + period;
        m_impl->pacerStarted = true;
        return;
    }
    if (now < m_impl->nextFrame) {
        std::this_thread::sleep_until(m_impl->nextFrame);
    } else {
        // We're behind; reset to avoid unbounded catch-up!
        m_impl->nextFrame = now;
    }
    m_impl->nextFrame += period;
}

} // namespace gfx (i.e., end of the nightmare)