#if LINUX
#include "linux_base.h"

void *mem_reserve(size_t size)
{
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        return NULL;
    }
    return ptr;
}

void mem_free(void *ptr, size_t size)
{
    if (ptr == NULL) {
        return;
    }
    munmap(ptr, size);
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
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)ts.tv_sec * NS_PER_SEC + (u64)ts.tv_nsec;
}
#endif
