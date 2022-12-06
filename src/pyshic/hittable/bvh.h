#pragma once

#include <memory>
#include <vector>

#include "aabb.h"
#include "hittable/hittable.h"
#include "hittable/gameobject.h"
#include "hittable/hittable_list.h"
#include "struct/bounding_record.h"
#include "struct/hit_record.h"
#include "helper/helper.h"

using std::shared_ptr;
using std::vector;
using std::make_shared;

class bvh_node : public hittable
{
private:
    aabb box;
    shared_ptr<hittable> left;
    shared_ptr<hittable> right;    
    
public:
    bvh_node();

    bvh_node(hittable_list list) : bvh_node(list.getAll()) {}
    bvh_node(vector<shared_ptr<hittable>> src_objects);

    hit_record hit(ray r, double t_min, double t_max) override;
    bounding_record bounding_box() override;

    static int find_best_axis(vector<shared_ptr<hittable>> src_objects);
    static bool box_compare(shared_ptr<hittable> a, shared_ptr<hittable> b, int axis);
    static bool box_compare_x(shared_ptr<hittable> a, shared_ptr<hittable> b);
    static bool box_compare_y(shared_ptr<hittable> a, shared_ptr<hittable> b);
    static bool box_compare_z(shared_ptr<hittable> a, shared_ptr<hittable> b);
};
