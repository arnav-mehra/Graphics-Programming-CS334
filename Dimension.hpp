#pragma once

#define max3(x, y, z) (max(max((x), (y)), (z)))
#define min3(x, y, z) (min(min((x), (y)), (z)))

enum Dim {
    X = 0,
    Y = 1,
    Z = 2
};