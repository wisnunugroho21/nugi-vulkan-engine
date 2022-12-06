#pragma once

#include <memory>

#include "arr3.h"
#include "hittable/hittable.h"
#include "struct/hit_record.h"
#include "struct/hit_record.h"
#include "struct/bounding_record.h"

using std::shared_ptr;

class gameobject : public hittable
{
private:
    shared_ptr<hittable> object_shape;

public:
    gameobject(shared_ptr<hittable> object_shape);

    hit_record hit(ray r, double t_min, double t_max) override;
    bounding_record bounding_box() override;
};
