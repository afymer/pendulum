#pragma once

#if defined(__clang__) || defined(_MSC_VER)
#if defined(__linux__)
#define LINUX 1
#elif defined(_WIN32)
#define WINDOWS 1
#else
#error Unsupported OS
#endif
#else
#error Unsupported compiler
#endif

#if !defined(LINUX)
#define LINUX 0
#endif

#if !defined(WINDOWS)
#define WINDOWS 0
#endif
