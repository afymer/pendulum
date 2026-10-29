#include "win32_base.h"

void *mem_reserve(size_t size)
{
    return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

void mem_free(void *ptr, size_t size)
{
    (void)size;
    if (ptr == NULL) {
        return;
    }
    VirtualFree(ptr, 0, MEM_RELEASE);
}

void set_mem(void *ptr, int value, size_t size)
{
    memset(ptr, value, size);
}

void copy_mem(void *dst, const void *src, size_t size)
{
    memcpy(dst, src, size);
}

u64 time_now_ns(void)
{
    static LARGE_INTEGER freq;
    static bool init = false;
    if (!init) {
        QueryPerformanceFrequency(&freq);
        init = true;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    u64 sec = (u64)(counter.QuadPart / freq.QuadPart);
    u64 rem = (u64)(counter.QuadPart % freq.QuadPart);
    return sec * 1000000000ULL + (rem * 1000000000ULL) / (u64)freq.QuadPart;
}
