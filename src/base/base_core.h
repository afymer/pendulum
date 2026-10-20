#pragma once

#include <stddef.h> // IWYU pragma: export
#include <stdio.h>  // IWYU pragma: export
#include <stdlib.h> // IWYU pragma: export

// W: side effects
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define ARGB(a, r, g, b) ((((u32)(a) << 24) | ((u32)(r) << 16) | ((u32)(g) << 8)) | (u32)(b))
#define RGB(r, g, b) ARGB(0xFF, r, g, b)

#define OFFSET_OF(type, member) offsetof(type, member)
#define CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - OFFSET_OF(type, member)))
