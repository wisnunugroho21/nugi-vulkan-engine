#pragma once

#include "aabb.h"

struct bounding_record
{
    bool is_hit = false;
    aabb bounding_box;
};
