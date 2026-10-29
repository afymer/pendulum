#include "ui_slotmap.h"
#include "base/base_arena.h"
#include "base/base_memory.h"

ui_slotmap_t *ui_slotmap_create(arena_t *arena, u32 capacity, usize element_size)
{
    ui_slotmap_t *sm = PUSH_STRUCT(arena, ui_slotmap_t);
    sm->capacity = capacity;
    sm->element_size = element_size;

    sm->data = PUSH_ARRAY(arena, u8, capacity * element_size);
    sm->generations = PUSH_ARRAY(arena, u32, capacity);
    sm->free_list = PUSH_ARRAY(arena, u32, capacity);
    sm->occupied = PUSH_ARRAY(arena, u8, capacity);
    sm->free_head = 0;
    sm->len = 0;

    for (u32 i = 0; i < capacity; i++) {
        sm->free_list[i] = i + 1;
        sm->generations[i] = 1;
    }
    sm->free_list[capacity - 1] = 0;
    return sm;
}

ui_handle_t ui_slotmap_insert(ui_slotmap_t *sm, const void *data)
{
    if (NULL == sm || sm->len == sm->capacity) {
        return UI_HANDLE_INVALID;
    }
    u32 index = sm->free_head;
    sm->free_head = sm->free_list[index];

    u32 gen = sm->generations[index];

    sm->occupied[index] = 1;
    copy_mem(sm->data + index * sm->element_size, data, sm->element_size);
    sm->len++;
    return (ui_handle_t){ .index = index, .gen = gen };
}

void *ui_slotmap_get(ui_slotmap_t *sm, ui_handle_t handle)
{
    if (NULL == sm || !ui_handle_is_valid(handle) || handle.index >= sm->capacity ||
        !sm->occupied[handle.index] || sm->generations[handle.index] != handle.gen) {
        return NULL;
    }
    return sm->data + handle.index * sm->element_size;
}

void ui_slotmap_erase(ui_slotmap_t *sm, ui_handle_t handle)
{
    if (NULL == sm || !ui_handle_is_valid(handle) || handle.index >= sm->capacity) {
        return;
    }
    if (!sm->occupied[handle.index] || sm->generations[handle.index] != handle.gen) {
        return;
    }
    sm->occupied[handle.index] = 0;
    sm->generations[handle.index]++;
    if (0 == sm->generations[handle.index]) {
        sm->generations[handle.index] = 1;
    }
    sm->free_list[handle.index] = sm->free_head;
    sm->free_head = handle.index;
    sm->len--;
}
