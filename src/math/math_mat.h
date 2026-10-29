#pragma once

#include "math_vec.h"

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
matf32 diag_matf32(arena_t *arena, u16 size, f32 value);
matf32 from_array_matf32(arena_t *arena, u16 rows, u16 cols, const f32 *values);

matf32 add_matf32(arena_t *arena, matf32 a, matf32 b);
matf32 sub_matf32(arena_t *arena, matf32 a, matf32 b);
matf32 mul_matf32(arena_t *arena, matf32 a, matf32 b);
matf32 transpose_matf32(arena_t *arena, matf32 mat);
void scale_matf32(matf32 mat, f32 scalar);

void assign_matf32(matf32 dst, matf32 src);
void symmetrize_matf32(matf32 mat);

void paste_matf32(matf32 dst, u16 row0, u16 col0, matf32 src);
matf32 slice_matf32(arena_t *arena, matf32 mat, u16 row0, u16 col0, u16 rows, u16 cols);

f32 max_abs_matf32(matf32 mat);

matf32 expm_matf32(arena_t *arena, matf32 mat);

void print_matf32(matf32 mat);

typedef struct matf64 matf64;
struct matf64 {
    u16 rows;
    u16 cols;
    f64 *data;
};

matf64 create_matf64(arena_t *arena, u16 rows, u16 cols);
matf64 copy_matf64(arena_t *arena, matf64 mat);
matf64 id_matf64(arena_t *arena, u16 size);
matf64 diag_matf64(arena_t *arena, u16 size, f64 value);
matf64 from_array_matf64(arena_t *arena, u16 rows, u16 cols, const f64 *values);

matf64 add_matf64(arena_t *arena, matf64 a, matf64 b);
matf64 sub_matf64(arena_t *arena, matf64 a, matf64 b);
matf64 mul_matf64(arena_t *arena, matf64 a, matf64 b);
matf64 transpose_matf64(arena_t *arena, matf64 mat);
void scale_matf64(matf64 mat, f64 scalar);

void assign_matf64(matf64 dst, matf64 src);
void symmetrize_matf64(matf64 mat);

void paste_matf64(matf64 dst, u16 row0, u16 col0, matf64 src);
matf64 slice_matf64(arena_t *arena, matf64 mat, u16 row0, u16 col0, u16 rows, u16 cols);

f64 max_abs_matf64(matf64 mat);

matf64 expm_matf64(arena_t *arena, matf64 mat);

void print_matf64(matf64 mat);

vec2f32 mul_matf32_v2f32(matf32 mat, vec2f32 v);
