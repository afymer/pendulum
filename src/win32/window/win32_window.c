#include "win32_window.h"
#include "window/window_inc.h"

static win32_window_t *g_active_win32_window = NULL;

static void win32_event_push(win32_window_t *window, const ui_event_t *ev)
{
    EnterCriticalSection(&window->event_lock);
    win32_event_queue_t *q = &window->event_queue;

    if (ev->kind == UI_EVENT_WINDOW_RESIZE) {
        for (u32 i = 0; i < q->count; i++) {
            u32 idx = (q->head + i) % WIN32_EVENT_QUEUE_CAPACITY;
            if (q->events[idx].kind == UI_EVENT_WINDOW_RESIZE) {
                q->events[idx].resize = ev->resize;
                LeaveCriticalSection(&window->event_lock);
                SetEvent(window->event_signal);
                return;
            }
        }
    } else if (ev->kind == UI_EVENT_WINDOW_REDRAW) {
        for (u32 i = 0; i < q->count; i++) {
            u32 idx = (q->head + i) % WIN32_EVENT_QUEUE_CAPACITY;
            if (q->events[idx].kind == UI_EVENT_WINDOW_REDRAW) {
                LeaveCriticalSection(&window->event_lock);
                return;
            }
        }
    }

    if (q->count < WIN32_EVENT_QUEUE_CAPACITY) {
        q->events[q->tail] = *ev;
        q->tail = (q->tail + 1) % WIN32_EVENT_QUEUE_CAPACITY;
        q->count++;
    }
    LeaveCriticalSection(&window->event_lock);
    SetEvent(window->event_signal);
}

static bool win32_event_pop(win32_window_t *window, ui_event_t *out)
{
    EnterCriticalSection(&window->event_lock);
    win32_event_queue_t *q = &window->event_queue;
    if (0 == q->count) {
        LeaveCriticalSection(&window->event_lock);
        return false;
    }
    *out = q->events[q->head];
    q->head = (q->head + 1) % WIN32_EVENT_QUEUE_CAPACITY;
    q->count--;
    LeaveCriticalSection(&window->event_lock);
    return true;
}

static LRESULT CALLBACK win32_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    win32_window_t *window = (win32_window_t *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (msg) {
    case WM_NCCREATE: {
        CREATESTRUCTA *cs = (CREATESTRUCTA *)lParam;
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    case WM_CLOSE: {
        if (window) {
            window->base.is_open = false;
            window->base.should_close = true;
            win32_event_push(window, &(ui_event_t){ .kind = UI_EVENT_WINDOW_CLOSE });
        }
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    case WM_SYSCOMMAND: {
        if ((wParam & 0xFFF0) == SC_KEYMENU) {
            return 0;
        }
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    case WM_SIZE: {
        i32 width = (i32)LOWORD(lParam);
        i32 height = (i32)HIWORD(lParam);
        if (window && (width != window->base.width || height != window->base.height)) {
            win32_event_push(window, &(ui_event_t){
                                         .kind = UI_EVENT_WINDOW_RESIZE,
                                         .resize = { .width = width, .height = height },
                                     });
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        if (window) {
            win32_event_push(window, &(ui_event_t){ .kind = UI_EVENT_WINDOW_REDRAW });
        }
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_KEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP: {
        if (window) {
            bool is_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
            u32 modifiers = 0;
            if (GetKeyState(VK_SHIFT) & 0x8000)
                modifiers |= (1 << 0);
            if (GetKeyState(VK_CONTROL) & 0x8000)
                modifiers |= (1 << 1);
            if (GetKeyState(VK_MENU) & 0x8000)
                modifiers |= (1 << 2);

            win32_event_push(window, &(ui_event_t){
                                         .kind = UI_EVENT_KEY,
                                         .key = {
                                             .key = (u32)wParam,
                                             .is_down = is_down,
                                             .modifiers = modifiers,
                                         },
                                     });
        }
        if (msg == WM_SYSKEYDOWN && wParam == VK_F4 && (lParam & (1 << 29))) {
            return DefWindowProcA(hwnd, msg, wParam, lParam);
        }
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (window) {
            f32 x = (f32)(short)LOWORD(lParam);
            f32 y = (f32)(short)HIWORD(lParam);
            win32_event_push(window, &(ui_event_t){
                                         .kind = UI_EVENT_MOUSE_MOVE,
                                         .mouse = { .x = x, .y = y },
                                     });
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP: {
        if (window) {
            f32 x = (f32)(short)LOWORD(lParam);
            f32 y = (f32)(short)HIWORD(lParam);
            win32_event_push(window, &(ui_event_t){
                                         .kind = UI_EVENT_MOUSE_BUTTON,
                                         .mouse = {
                                             .x = x,
                                             .y = y,
                                             .button = 1,
                                             .is_down = (msg == WM_LBUTTONDOWN),
                                         },
                                     });
        }
        return 0;
    }
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP: {
        if (window) {
            f32 x = (f32)(short)LOWORD(lParam);
            f32 y = (f32)(short)HIWORD(lParam);
            win32_event_push(window, &(ui_event_t){
                                         .kind = UI_EVENT_MOUSE_BUTTON,
                                         .mouse = {
                                             .x = x,
                                             .y = y,
                                             .button = 2,
                                             .is_down = (msg == WM_RBUTTONDOWN),
                                         },
                                     });
        }
        return 0;
    }
    case WM_MOUSEWHEEL: {
        if (window) {
            f32 delta = (f32)GET_WHEEL_DELTA_WPARAM(wParam) / (f32)WHEEL_DELTA;
            win32_event_push(window, &(ui_event_t){
                                         .kind = UI_EVENT_MOUSE_SCROLL,
                                         .scroll = { .dx = 0.0f, .dy = delta },
                                     });
        }
        return 0;
    }
    default:
        return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

typedef struct win32_thread_init_s {
    win32_window_t *window;
    window_params_t params;
    HANDLE ready_event;
    bool init_success;
} win32_thread_init_t;

static DWORD WINAPI win32_window_thread_proc(LPVOID param)
{
    win32_thread_init_t *init = (win32_thread_init_t *)param;
    win32_window_t *window = init->window;
    window_params_t params = init->params;

    HINSTANCE hinst = GetModuleHandleA(NULL);
    WNDCLASSEXA wc = {
        .cbSize = sizeof(WNDCLASSEXA),
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS,
        .lpfnWndProc = win32_wnd_proc,
        .hInstance = hinst,
        .hCursor = LoadCursorA(NULL, IDC_ARROW),
        .lpszClassName = "PendulumWindowClass",
    };
    RegisterClassExA(&wc);

    RECT rect = { 0, 0, params.width, params.height };
    DWORD style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&rect, style, FALSE);
    i32 win_width = rect.right - rect.left;
    i32 win_height = rect.bottom - rect.top;

    window->hwnd = CreateWindowExA(0, "PendulumWindowClass", params.title, style,
                                   CW_USEDEFAULT, CW_USEDEFAULT, win_width, win_height,
                                   NULL, NULL, hinst, window);
    if (!window->hwnd) {
        init->init_success = false;
        SetEvent(init->ready_event);
        return 1;
    }

    ShowWindow(window->hwnd, SW_SHOW);
    UpdateWindow(window->hwnd);

    init->init_success = true;
    SetEvent(init->ready_event);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    if (window) {
        window->base.is_open = false;
        window->base.should_close = true;
    }
    return 0;
}

window_t *window_create(ui_app_t *app, window_params_t params)
{
    arena_t *arena = arena_create((arena_params_t){
        .reserve_size = 64 * 1024,
        .commit_size = 64 * 1024,
    });
    if (NULL == arena) {
        return NULL;
    }

    win32_window_t *window = PUSH_STRUCT(arena, win32_window_t);
    *window = (win32_window_t){
        .base = {
            .title = params.title,
            .width = params.width,
            .height = params.height,
            .scale_factor = 1.0f,
            .is_open = true,
            .should_close = false,
            .is_dirty = true,
            .is_animating = false,
            .needs_full_redraw = true,
            .full_redraw_buffers = 1,
        },
        .arena = arena,
    };

    InitializeCriticalSection(&window->event_lock);
    window->event_signal = CreateEventA(NULL, FALSE, FALSE, NULL);

    timeBeginPeriod(1);

    win32_thread_init_t init = {
        .window = window,
        .params = params,
        .ready_event = CreateEventA(NULL, FALSE, FALSE, NULL),
        .init_success = false,
    };

    window->thread = CreateThread(NULL, 0, win32_window_thread_proc, &init, 0, &window->thread_id);
    if (!window->thread) {
        CloseHandle(init.ready_event);
        CloseHandle(window->event_signal);
        DeleteCriticalSection(&window->event_lock);
        arena_free(arena);
        return NULL;
    }

    WaitForSingleObject(init.ready_event, INFINITE);
    CloseHandle(init.ready_event);

    if (!init.init_success) {
        WaitForSingleObject(window->thread, INFINITE);
        CloseHandle(window->thread);
        CloseHandle(window->event_signal);
        DeleteCriticalSection(&window->event_lock);
        arena_free(arena);
        return NULL;
    }

    g_active_win32_window = window;
    ui_app_add_window(app, &window->base);
    return &window->base;
}

void window_destroy(window_t *base)
{
    if (NULL == base) {
        return;
    }
    win32_window_t *window = CONTAINER_OF(base, win32_window_t, base);
    if (g_active_win32_window == window) {
        g_active_win32_window = NULL;
    }

    if (window->hwnd) {
        PostMessageA(window->hwnd, WM_CLOSE, 0, 0);
    }
    if (window->thread) {
        WaitForSingleObject(window->thread, 2000);
        CloseHandle(window->thread);
        window->thread = NULL;
    }
    timeEndPeriod(1);

    if (window->event_signal) {
        CloseHandle(window->event_signal);
        window->event_signal = NULL;
    }
    DeleteCriticalSection(&window->event_lock);

    if (window->pixels) {
        VirtualFree(window->pixels, 0, MEM_RELEASE);
        window->pixels = NULL;
    }
    arena_t *arena = window->arena;
    if (arena) {
        arena_free(arena);
    }
}

ui_event_t wait_for_event(void)
{
    win32_window_t *window = g_active_win32_window;
    if (NULL == window || !window->base.is_open) {
        return (ui_event_t){ .kind = UI_EVENT_WINDOW_CLOSE };
    }

    ui_event_t ev = { 0 };
    if (win32_event_pop(window, &ev)) {
        return ev;
    }

    if (window->base.is_animating) {
        WaitForSingleObject(window->event_signal, 1);
        if (win32_event_pop(window, &ev)) {
            return ev;
        }
        window->base.is_dirty = true;
        return (ui_event_t){ .kind = UI_EVENT_WINDOW_REDRAW };
    }

    while (window->base.is_open) {
        WaitForSingleObject(window->event_signal, 100);
        if (win32_event_pop(window, &ev)) {
            return ev;
        }
    }

    return (ui_event_t){ .kind = UI_EVENT_WINDOW_CLOSE };
}

bool poll_event(ui_event_t *out)
{
    win32_window_t *window = g_active_win32_window;
    if (NULL == window || !window->base.is_open) {
        return false;
    }
    return win32_event_pop(window, out);
}

framebuffer_t window_framebuffer(window_t *base)
{
    win32_window_t *window = CONTAINER_OF(base, win32_window_t, base);
    i32 width = window->base.width;
    i32 height = window->base.height;
    if (width <= 0 || height <= 0) {
        return (framebuffer_t){ 0 };
    }

    i32 stride = width * 4;
    usize required_size = (usize)stride * (usize)height;

    if (required_size > window->pixels_capacity) {
        if (window->pixels) {
            VirtualFree(window->pixels, 0, MEM_RELEASE);
        }
        window->pixels_capacity = required_size;
        window->pixels = (u32 *)VirtualAlloc(NULL, window->pixels_capacity,
                                             MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    }

    window->framebuffer = (framebuffer_t){
        .pixels = window->pixels,
        .width = width,
        .height = height,
        .stride = stride,
    };
    base->is_dirty = false;
    return window->framebuffer;
}

void window_damage(window_t *base, i32 x, i32 y, i32 w, i32 h)
{
    (void)base;
    (void)x;
    (void)y;
    (void)w;
    (void)h;
}

void window_present(window_t *base)
{
    win32_window_t *window = CONTAINER_OF(base, win32_window_t, base);
    if (NULL == window || NULL == window->pixels || window->base.width <= 0 ||
        window->base.height <= 0) {
        return;
    }

    HDC hdc = GetDC(window->hwnd);
    if (!hdc) {
        return;
    }

    BITMAPINFO bmi = { 0 };
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = window->base.width;
    bmi.bmiHeader.biHeight = -window->base.height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    StretchDIBits(hdc, 0, 0, window->base.width, window->base.height, 0, 0,
                  window->base.width, window->base.height, window->pixels, &bmi,
                  DIB_RGB_COLORS, SRCCOPY);

    ReleaseDC(window->hwnd, hdc);
}
