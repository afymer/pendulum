#pragma once

#define LOG_COLOR_RED "31"
#define LOG_COLOR_YELLOW "33"
#define LOG_COLOR_GREEN "32"
#define LOG_COLOR_CYAN "36"

#define LOG_AT(color, level, fmt, ...)                                                            \
    fprintf(stderr, "\x1b[" color "m[" level "]\x1b[0m %s:%d: " fmt "\n", __FILE__,               \
            __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#define LOG_ERROR(fmt, ...) LOG_AT(LOG_COLOR_RED, "error", fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_WARN(fmt, ...) LOG_AT(LOG_COLOR_YELLOW, "warn", fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_INFO(fmt, ...) LOG_AT(LOG_COLOR_GREEN, "info", fmt __VA_OPT__(, ) __VA_ARGS__)
#define LOG_DEBUG(fmt, ...) LOG_AT(LOG_COLOR_CYAN, "debug", fmt __VA_OPT__(, ) __VA_ARGS__)
