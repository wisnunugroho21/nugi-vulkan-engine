#pragma once

#include <memory>
#include <vector>

#include "aabb.h"
#include "hittable/hittable.h"
#include "struct/hit_record.h"
#include "struct/bounding_record.h"
#include "hittable/gameobject.h"

using std::shared_ptr;
using std::vector;

class hittable_list : public hittable
{
private:
    vector<shared_ptr<hittable>> objects;
    
public:
    hittable_list();
    hittable_list(shared_ptr<hittable> object);

    void clear();
    void add(shared_ptr<hittable> object);
    vector<shared_ptr<hittable>> getAll();

    hit_record hit(ray r, double t_min, double t_max) override;
    bounding_record bounding_box() override;
};
