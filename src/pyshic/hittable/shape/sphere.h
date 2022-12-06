#pragma once

#include <memory>

#include "arr3.h"
#include "../../aabb.h"
#include "hittable/hittable.h"
#include "struct/bounding_record.h"

using std::shared_ptr;

class sphere : public hittable
{
private:
    point3 center;
    double radius;

public:
    sphere();
    sphere(point3 center, double radius);

    hit_record hit(ray r, double t_min, double t_max) override;
    bounding_record bounding_box() override;
};
