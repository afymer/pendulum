#include "base_arena.h"

#include <string.h>
#include <sys/mman.h>

#define ALIGN_POW2(x, b) (((x) + (b) - 1) & (~((b) - 1)))
#define PAGE_SIZE 4096

arena_t *arena_create(arena_params_t params)
{
    usize reserved = params.reserve_size + ARENA_METADATA_SIZE;
    usize committed = params.commit_size + ARENA_METADATA_SIZE;
    reserved = ALIGN_POW2(reserved, PAGE_SIZE);
    committed = ALIGN_POW2(committed, PAGE_SIZE);

    arena_t *arena =
        mmap(NULL, reserved, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena == MAP_FAILED) {
        return NULL;
    }
    memset(arena, 0, committed);
    arena->reserved = reserved;
    arena->committed = committed;
    arena->opl = ARENA_METADATA_SIZE;

    return arena;
}

void arena_free(arena_t *arena)
{
    munmap(arena, arena->reserved);
}

void *arena_push(arena_t *arena, u64 size, u64 align)
{
    u64 offset = ALIGN_POW2(arena->opl, align);
    void *ptr = (void *)((char *)arena + offset);
    arena->opl = offset + size;
    return ptr;
}
