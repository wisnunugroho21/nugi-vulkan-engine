#pragma once

#include <algorithm>
#include <cmath>

#include "arr3.h"
#include "ray.h"

class aabb
{
private:
    point3 minimum;
    point3 maximum;

public:
    aabb();
    aabb(point3 minimum, point3 maximum);

    point3 min();
    point3 max();

    bool hit(ray r, double t_min, double t_max);
    
    static aabb surrounding_box(aabb box0, aabb box1);
};
