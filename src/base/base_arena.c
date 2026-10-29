#include "base_arena.h"

#include "base_memory.h"

#define ALIGN_POW2(x, b) (((x) + (b) - 1) & (~((b) - 1)))
#define PAGE_SIZE 4096

arena_t *arena_create(arena_params_t params)
{
    usize reserved = params.reserve_size + ARENA_METADATA_SIZE;
    usize committed = params.commit_size + ARENA_METADATA_SIZE;
    reserved = ALIGN_POW2(reserved, PAGE_SIZE);
    committed = ALIGN_POW2(committed, PAGE_SIZE);

    arena_t *arena = mem_reserve(committed);
    if (arena == NULL) {
        return NULL;
    }
    set_mem(arena, 0, committed);
    arena->reserved = reserved;
    arena->committed = committed;
    arena->opl = ARENA_METADATA_SIZE;

    return arena;
}

void arena_free(arena_t *arena)
{
    mem_free(arena, arena->committed);
}

void *arena_push(arena_t *arena, u64 size, u64 align)
{
    u64 offset = ALIGN_POW2(arena->opl, align);
    void *ptr = (void *)((char *)arena + offset);
    arena->opl = offset + size;
    return ptr;
}

arena_temp_t arena_temp_begin(arena_t *arena)
{
    return (arena_temp_t){ .arena = arena, .opl = arena->opl };
}

void arena_temp_end(arena_temp_t temp)
{
    temp.arena->opl = temp.opl;
}
