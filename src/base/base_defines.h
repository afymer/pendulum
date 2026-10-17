#pragma once

#if defined(__clang__)
#if defined(__linux__)
#define LINUX 1
#else
#error Unsupported OS
#endif
#else
#error Unsupported compiler
#endif

#if !defined(LINUX)
#define LINUX 0
#endif
