#include "math_mat.h"
#include "base/base_memory.h"

#define MAT_EXPM_TAYLOR_TERMS 20

matf32 create_matf32(arena_t *arena, u16 rows, u16 cols)
{
    matf32 mat;
    mat.rows = rows;
    mat.cols = cols;
    usize size = (usize)rows * cols * sizeof(f32);
    mat.data = arena_push(arena, size, alignof(f32));
    set_mem(mat.data, 0, size);
    return mat;
}

matf32 copy_matf32(arena_t *arena, matf32 mat)
{
    matf32 result = create_matf32(arena, mat.rows, mat.cols);
    copy_mem(result.data, mat.data, (usize)mat.rows * mat.cols * sizeof(f32));
    return result;
}

matf32 diag_matf32(arena_t *arena, u16 size, f32 value)
{
    matf32 mat = create_matf32(arena, size, size);
    for (u16 i = 0; i < size; i++) {
        MAT_SET(mat, i, i, value);
    }
    return mat;
}

matf32 id_matf32(arena_t *arena, u16 size)
{
    return diag_matf32(arena, size, 1);
}

matf32 from_array_matf32(arena_t *arena, u16 rows, u16 cols, const f32 *values)
{
    matf32 mat = create_matf32(arena, rows, cols);
    copy_mem(mat.data, values, (usize)rows * cols * sizeof(f32));
    return mat;
}

matf32 add_matf32(arena_t *arena, matf32 a, matf32 b)
{
    matf32 result = create_matf32(arena, a.rows, a.cols);
    for (usize i = 0; i < (usize)a.rows * a.cols; i++) {
        result.data[i] = a.data[i] + b.data[i];
    }
    return result;
}

matf32 sub_matf32(arena_t *arena, matf32 a, matf32 b)
{
    matf32 result = create_matf32(arena, a.rows, a.cols);
    for (usize i = 0; i < (usize)a.rows * a.cols; i++) {
        result.data[i] = a.data[i] - b.data[i];
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
                sum += MAT_GET(a, i, k) * MAT_GET(b, k, j);
            }
            MAT_SET(result, i, j, sum);
        }
    }
    return result;
}

matf32 transpose_matf32(arena_t *arena, matf32 mat)
{
    matf32 result = create_matf32(arena, mat.cols, mat.rows);
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = 0; j < mat.cols; j++) {
            MAT_SET(result, j, i, MAT_GET(mat, i, j));
        }
    }
    return result;
}

void scale_matf32(matf32 mat, f32 scalar)
{
    for (usize i = 0; i < (usize)mat.rows * mat.cols; i++) {
        mat.data[i] *= scalar;
    }
}

void assign_matf32(matf32 dst, matf32 src)
{
    copy_mem(dst.data, src.data, (usize)dst.rows * dst.cols * sizeof(f32));
}

void symmetrize_matf32(matf32 mat)
{
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = i + 1; j < mat.cols; j++) {
            f32 avg = (MAT_GET(mat, i, j) + MAT_GET(mat, j, i)) / 2;
            MAT_SET(mat, i, j, avg);
            MAT_SET(mat, j, i, avg);
        }
    }
}

void paste_matf32(matf32 dst, u16 row0, u16 col0, matf32 src)
{
    for (u16 i = 0; i < src.rows; i++) {
        for (u16 j = 0; j < src.cols; j++) {
            MAT_SET(dst, row0 + i, col0 + j, MAT_GET(src, i, j));
        }
    }
}

matf32 slice_matf32(arena_t *arena, matf32 mat, u16 row0, u16 col0, u16 rows, u16 cols)
{
    matf32 result = create_matf32(arena, rows, cols);
    for (u16 i = 0; i < rows; i++) {
        for (u16 j = 0; j < cols; j++) {
            MAT_SET(result, i, j, MAT_GET(mat, row0 + i, col0 + j));
        }
    }
    return result;
}

f32 max_abs_matf32(matf32 mat)
{
    f32 max = 0;
    for (usize i = 0; i < (usize)mat.rows * mat.cols; i++) {
        max = MAX(max, ABS(mat.data[i]));
    }
    return max;
}

matf32 expm_matf32(arena_t *arena, matf32 mat)
{
    matf32 result = id_matf32(arena, mat.rows);
    matf32 term = id_matf32(arena, mat.rows);
    arena_temp_t temp = arena_temp_begin(arena);

    // Scale so the Taylor series converges fast
    u32 squarings = 0;
    f32 norm = max_abs_matf32(mat) * mat.rows;
    while (norm > 0.5) {
        norm /= 2;
        squarings++;
    }
    matf32 scaled = copy_matf32(arena, mat);
    scale_matf32(scaled, (f32)1 / (f32)(1u << squarings));

    for (u32 n = 1; n <= MAT_EXPM_TAYLOR_TERMS; n++) {
        arena_temp_t step = arena_temp_begin(arena);
        matf32 next = mul_matf32(arena, term, scaled);
        scale_matf32(next, (f32)1 / (f32)n);
        assign_matf32(term, next);
        assign_matf32(result, add_matf32(arena, result, term));
        arena_temp_end(step);
    }
    for (u32 i = 0; i < squarings; i++) {
        arena_temp_t step = arena_temp_begin(arena);
        assign_matf32(result, mul_matf32(arena, result, result));
        arena_temp_end(step);
    }

    arena_temp_end(temp);
    return result;
}

void print_matf32(matf32 mat)
{
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = 0; j < mat.cols; j++) {
            printf("%f ", (f64)MAT_GET(mat, i, j));
        }
        printf("\n");
    }
}

matf64 create_matf64(arena_t *arena, u16 rows, u16 cols)
{
    matf64 mat;
    mat.rows = rows;
    mat.cols = cols;
    usize size = (usize)rows * cols * sizeof(f64);
    mat.data = arena_push(arena, size, alignof(f64));
    set_mem(mat.data, 0, size);
    return mat;
}

matf64 copy_matf64(arena_t *arena, matf64 mat)
{
    matf64 result = create_matf64(arena, mat.rows, mat.cols);
    copy_mem(result.data, mat.data, (usize)mat.rows * mat.cols * sizeof(f64));
    return result;
}

matf64 diag_matf64(arena_t *arena, u16 size, f64 value)
{
    matf64 mat = create_matf64(arena, size, size);
    for (u16 i = 0; i < size; i++) {
        MAT_SET(mat, i, i, value);
    }
    return mat;
}

matf64 id_matf64(arena_t *arena, u16 size)
{
    return diag_matf64(arena, size, 1);
}

matf64 from_array_matf64(arena_t *arena, u16 rows, u16 cols, const f64 *values)
{
    matf64 mat = create_matf64(arena, rows, cols);
    copy_mem(mat.data, values, (usize)rows * cols * sizeof(f64));
    return mat;
}

matf64 add_matf64(arena_t *arena, matf64 a, matf64 b)
{
    matf64 result = create_matf64(arena, a.rows, a.cols);
    for (usize i = 0; i < (usize)a.rows * a.cols; i++) {
        result.data[i] = a.data[i] + b.data[i];
    }
    return result;
}

matf64 sub_matf64(arena_t *arena, matf64 a, matf64 b)
{
    matf64 result = create_matf64(arena, a.rows, a.cols);
    for (usize i = 0; i < (usize)a.rows * a.cols; i++) {
        result.data[i] = a.data[i] - b.data[i];
    }
    return result;
}

matf64 mul_matf64(arena_t *arena, matf64 a, matf64 b)
{
    matf64 result = create_matf64(arena, a.rows, b.cols);
    for (u16 i = 0; i < a.rows; i++) {
        for (u16 j = 0; j < b.cols; j++) {
            f64 sum = 0;
            for (u16 k = 0; k < a.cols; k++) {
                sum += MAT_GET(a, i, k) * MAT_GET(b, k, j);
            }
            MAT_SET(result, i, j, sum);
        }
    }
    return result;
}

matf64 transpose_matf64(arena_t *arena, matf64 mat)
{
    matf64 result = create_matf64(arena, mat.cols, mat.rows);
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = 0; j < mat.cols; j++) {
            MAT_SET(result, j, i, MAT_GET(mat, i, j));
        }
    }
    return result;
}

void scale_matf64(matf64 mat, f64 scalar)
{
    for (usize i = 0; i < (usize)mat.rows * mat.cols; i++) {
        mat.data[i] *= scalar;
    }
}

void assign_matf64(matf64 dst, matf64 src)
{
    copy_mem(dst.data, src.data, (usize)dst.rows * dst.cols * sizeof(f64));
}

void symmetrize_matf64(matf64 mat)
{
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = i + 1; j < mat.cols; j++) {
            f64 avg = (MAT_GET(mat, i, j) + MAT_GET(mat, j, i)) / 2;
            MAT_SET(mat, i, j, avg);
            MAT_SET(mat, j, i, avg);
        }
    }
}

void paste_matf64(matf64 dst, u16 row0, u16 col0, matf64 src)
{
    for (u16 i = 0; i < src.rows; i++) {
        for (u16 j = 0; j < src.cols; j++) {
            MAT_SET(dst, row0 + i, col0 + j, MAT_GET(src, i, j));
        }
    }
}

matf64 slice_matf64(arena_t *arena, matf64 mat, u16 row0, u16 col0, u16 rows, u16 cols)
{
    matf64 result = create_matf64(arena, rows, cols);
    for (u16 i = 0; i < rows; i++) {
        for (u16 j = 0; j < cols; j++) {
            MAT_SET(result, i, j, MAT_GET(mat, row0 + i, col0 + j));
        }
    }
    return result;
}

f64 max_abs_matf64(matf64 mat)
{
    f64 max = 0;
    for (usize i = 0; i < (usize)mat.rows * mat.cols; i++) {
        max = MAX(max, ABS(mat.data[i]));
    }
    return max;
}

matf64 expm_matf64(arena_t *arena, matf64 mat)
{
    matf64 result = id_matf64(arena, mat.rows);
    matf64 term = id_matf64(arena, mat.rows);
    arena_temp_t temp = arena_temp_begin(arena);

    // Scale so the Taylor series converges fast
    u32 squarings = 0;
    f64 norm = max_abs_matf64(mat) * mat.rows;
    while (norm > 0.5) {
        norm /= 2;
        squarings++;
    }
    matf64 scaled = copy_matf64(arena, mat);
    scale_matf64(scaled, (f64)1 / (f64)(1u << squarings));

    for (u32 n = 1; n <= MAT_EXPM_TAYLOR_TERMS; n++) {
        arena_temp_t step = arena_temp_begin(arena);
        matf64 next = mul_matf64(arena, term, scaled);
        scale_matf64(next, (f64)1 / (f64)n);
        assign_matf64(term, next);
        assign_matf64(result, add_matf64(arena, result, term));
        arena_temp_end(step);
    }
    for (u32 i = 0; i < squarings; i++) {
        arena_temp_t step = arena_temp_begin(arena);
        assign_matf64(result, mul_matf64(arena, result, result));
        arena_temp_end(step);
    }

    arena_temp_end(temp);
    return result;
}

void print_matf64(matf64 mat)
{
    for (u16 i = 0; i < mat.rows; i++) {
        for (u16 j = 0; j < mat.cols; j++) {
            printf("%f ", (f64)MAT_GET(mat, i, j));
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
