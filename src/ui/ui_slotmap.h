#pragma once

typedef struct ui_handle_s ui_handle_t;
struct ui_handle_s {
    u32 index;
    u32 gen;
};

#define UI_HANDLE_INVALID ((ui_handle_t){ 0, 0 })

static inline bool ui_handle_is_valid(ui_handle_t h)
{
    return h.gen != 0;
}

static inline bool ui_handle_eq(ui_handle_t a, ui_handle_t b)
{
    return a.index == b.index && a.gen == b.gen;
}

typedef struct ui_slotmap_s ui_slotmap_t;
struct ui_slotmap_s {
    u8 *data;
    u32 *generations;
    u32 *free_list;
    u8 *occupied;

    u32 capacity;
    u32 free_head;
    u32 len;
    usize element_size;
};

ui_slotmap_t *ui_slotmap_create(arena_t *arena, u32 capacity, usize element_size);
ui_handle_t ui_slotmap_insert(ui_slotmap_t *sm, const void *data);
void *ui_slotmap_get(ui_slotmap_t *sm, ui_handle_t handle);
void ui_slotmap_erase(ui_slotmap_t *sm, ui_handle_t handle);
