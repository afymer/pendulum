#pragma once

#define ARENA_DEFAULT_RESERVE_SIZE 4096
#define ARENA_DEFAULT_COMMIT_SIZE 1024

typedef struct arena_params_s arena_params_t;
struct arena_params_s {
    usize reserve_size;
    usize commit_size;
};

typedef struct arena_s arena_t;
struct arena_s {
    usize reserved;
    usize committed;
    usize opl;
};

#define ARENA_METADATA_SIZE sizeof(arena_t)

arena_t *arena_create(arena_params_t params);
void arena_free(arena_t *arena);
void *arena_push(arena_t *arena, u64 size, u64 align);

#define PUSH_STRUCT(arena, type) ((type *)arena_push((arena), sizeof(type), alignof(type)))
#define PUSH_ARRAY(arena, type, count)                                                            \
    ((type *)arena_push((arena), sizeof(type) * (count), alignof(type)))
