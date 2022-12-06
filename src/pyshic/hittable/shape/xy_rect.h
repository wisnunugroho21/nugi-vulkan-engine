#pragma once

#include "arr3.h"
#include "aabb.h"
#include "hittable/hittable.h"
#include "struct/bounding_record.h"

class xy_rect : public hittable
{
private:
    double x0, x1, y0, y1, k;

public:
    xy_rect();
    xy_rect(double x0, double x1, double y0, double y1, double k);

    hit_record hit(ray r, double t_min, double t_max) override;
    bounding_record bounding_box() override;
};
