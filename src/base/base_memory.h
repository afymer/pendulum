#pragma once

void *mem_reserve(size_t size);
void mem_free(void *ptr, size_t size);
void set_mem(void *ptr, int value, size_t size);
void copy_mem(void *dst, const void *src, size_t size);
