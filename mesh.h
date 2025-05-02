#pragma once
#include "ray.h"
#include "vec.h"
#include "aabb.h"

struct Mesh {
    std::string name;
    Uint index_start;
    Uint index_count;
    AABB bbox;
};