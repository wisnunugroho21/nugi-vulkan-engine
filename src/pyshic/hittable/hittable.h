#pragma once

#include "ray.h"
#include "struct/hit_record.h"
#include "struct/bounding_record.h"

class hittable
{
public:
    uint32_t objectId = 0;
    
    virtual hit_record hit(ray r, double t_min, double t_max) = 0;
    virtual bounding_record bounding_box() = 0;
};
