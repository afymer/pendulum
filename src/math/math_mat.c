#include "math_mat.h"

matf32 create_matf32(arena_t *arena, u16 rows, u16 cols)
{
    matf32 mat;
    mat.rows = rows;
    mat.cols = cols;
    mat.data = arena_push(arena, rows * cols * sizeof(f32), 1);
    return mat;
}

matf32 copy_matf32(arena_t *arena, matf32 mat)
{
    matf32 result = create_matf32(arena, mat.rows, mat.cols);
    memcpy(result.data, mat.data, mat.rows * mat.cols * sizeof(f32));
    return result;
}

matf32 id_matf32(arena_t *arena, u16 size)
{
    matf32 mat = create_matf32(arena, size, size);
    for (u16 i = 0; i < size; i++) {
        for (u16 j = 0; j < size; j++) {
            mat.data[i * size + j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    return mat;
}

matf32 diag_matf32(arena_t *arena, u16 size, f32 value)
{
    matf32 mat = create_matf32(arena, size, size);
    for (u16 i = 0; i < size; i++) {
        for (u16 j = 0; j < size; j++) {
            mat.data[i * size + j] = (i == j) ? value : 0.0f;
        }
    }
    return mat;
}

matf32 add_matf32(arena_t *arena, matf32 a, matf32 b)
{
    matf32 result = create_matf32(arena, a.rows, a.cols);
    for (u16 i = 0; i < a.rows * a.cols; i++) {
        result.data[i] = a.data[i] + b.data[i];
    }
    return result;
}

matf32 mul_matf32(arena_t *arena, matf32 a, matf32 b)
{
    matf32 result = create_matf32(arena, a.rows, b.cols);
    for (u16 i = 0; i < a.rows; i++) {
        for (u16 j = 0; j < b.cols; j++) {
            f32 sum = 0;
            for (u16 k = 0; k < a.cols; k++) {
                sum += a.data[i * a.cols + k] * b.data[k * b.cols + j];
            }
            result.data[i * b.cols + j] = sum;
        }
    }
    return result;
}

void scale_matf32(matf32 mat, f32 scalar)
{
    for (u16 i = 0; i < mat.rows * mat.cols; i++) {
        mat.data[i] *= scalar;
    }
}

void print_matf32(matf32 mat)
{
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = 0; j < mat.cols; j++) {
            printf("%f ", mat.data[i * mat.cols + j]);
        }
        printf("\n");
    }
}

vec2f32 mul_matf32_v2f32(matf32 mat, vec2f32 v)
{
    // TODO: add size assertion
    return v2f32(MAT_GET(mat, 0, 0) * v.x + MAT_GET(mat, 0, 1) * v.y,
                 MAT_GET(mat, 1, 0) * v.x + MAT_GET(mat, 1, 1) * v.y);
}
