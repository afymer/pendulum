#pragma once

typedef struct matf32 matf32;
struct matf32 {
    u16 rows;
    u16 cols;
    f32 *data;
};
