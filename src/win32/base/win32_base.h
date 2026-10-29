#pragma once

#undef RGB
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <timeapi.h>
#undef RGB
#define RGB(r, g, b) ARGB(0xFF, r, g, b)
#include <string.h> // IWYU pragma: keep
