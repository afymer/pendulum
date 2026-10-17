#pragma once

#define MAT_AT(mat, row, col) (mat.data[(row) * (mat.cols) + (col)])
#define MAT_SET(mat, row, col, val) (MAT_AT(mat, row, col) = val)
#define MAT_GET(mat, row, col) (MAT_AT(mat, row, col))

typedef struct matf32 matf32;
struct matf32 {
    u16 rows;
    u16 cols;
    f32 *data;
};

matf32 create_matf32(arena_t *arena, u16 rows, u16 cols);
matf32 copy_matf32(arena_t *arena, matf32 mat);
matf32 id_matf32(arena_t *arena, u16 size);
matf32 add_matf32(arena_t *arena, matf32 a, matf32 b);
matf32 mul_matf32(arena_t *arena, matf32 a, matf32 b);
void scale_matf32(arena_t *arena, matf32 mat, f32 scalar);

void print_matf32(matf32 mat);
