#pragma once

// IWYU pragma: begin_exports
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdalign.h>
// IWYU pragma: end_exports

// W: side effects
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS(a) ((a) < 0 ? -(a) : (a))
#define CLAMP(x, lo, hi) MIN(MAX((x), (lo)), (hi))

#define ARGB(a, r, g, b) ((((u32)(a) << 24) | ((u32)(r) << 16) | ((u32)(g) << 8)) | (u32)(b))
#define RGB(r, g, b) ARGB(0xFF, r, g, b)

#define OFFSET_OF(type, member) offsetof(type, member)
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - OFFSET_OF(type, member)))
