#include "bvh.h"

#include <iostream>

bvh_node::bvh_node() {

}

bvh_node::bvh_node(vector<shared_ptr<hittable>> src_objects) {
    vector<shared_ptr<hittable>> objects = src_objects;

    int axis = bvh_node::find_best_axis(objects);
    auto comparator = (axis == 0) ? bvh_node::box_compare_x
                    : (axis == 1) ? bvh_node::box_compare_y
                                : bvh_node::box_compare_z;

    if (objects.size() == 1) {
        this->left = this->right = objects[0];
    } else if (objects.size() == 2) {
        if (comparator(objects[0], objects[1])) {
            this->left = objects[0];
            this->right = objects[1];
        } else {
            this->left = objects[1];
            this->right = objects[0];
        }        
    } else {
        std::sort(objects.begin(), objects.end(), comparator);
        auto mid = static_cast<int>(objects.size() / 2);

        vector<shared_ptr<hittable>> leftObj;
        for (int i = 0; i < mid; i++)
            leftObj.push_back(objects.at(i));

        vector<shared_ptr<hittable>> rightObj;
        for (int j = mid; j < objects.size(); j++)
            rightObj.push_back(objects.at(j));
        
        this->left = make_shared<bvh_node>(leftObj);
        this->right = make_shared<bvh_node>(rightObj);
    }

    bounding_record bound_left = this->left->bounding_box();
    bounding_record bound_right = this->right->bounding_box();

    this->box = aabb::surrounding_box(bound_left.bounding_box, bound_right.bounding_box);    
}

bounding_record bvh_node::bounding_box() {
    bounding_record bound;

    bound.is_hit = true;
    bound.bounding_box = this->box;

    return bound;
}

hit_record bvh_node::hit(ray r, double t_min, double t_max) {
    hit_record fail_hit;

    if (!this->box.hit(r, t_min, t_max)) {
        return fail_hit;
    }

    hit_record left_hit = this->left->hit(r, t_min, t_max);    
    hit_record right_hit = this->right->hit(r, t_min, left_hit.is_hit ? left_hit.t : t_max);

    if (right_hit.is_hit) {
        return right_hit;
    } 
    
    if (left_hit.is_hit) {
        return left_hit;
    }
    
    return fail_hit;
}

int bvh_node::find_best_axis(vector<shared_ptr<hittable>> src_objects)
{
    vector<shared_ptr<hittable>> objects = src_objects;

    std::sort(objects.begin(), objects.end(), bvh_node::box_compare_x);
    auto length_x = objects[objects.size() - 1]->bounding_box().bounding_box.min().x() - objects[0]->bounding_box().bounding_box.min().x();

    std::sort(objects.begin(), objects.end(), bvh_node::box_compare_y);
    auto length_y = objects[objects.size() - 1]->bounding_box().bounding_box.min().y() - objects[0]->bounding_box().bounding_box.min().y();

    std::sort(objects.begin(), objects.end(), bvh_node::box_compare_z);
    auto length_z = objects[objects.size() - 1]->bounding_box().bounding_box.min().z() - objects[0]->bounding_box().bounding_box.min().z();

    if (length_x >= length_y && length_x >= length_z) {
        return 0;
    }

    if (length_y >= length_x && length_y >= length_z) {
        return 1;
    }

    return 2;
}

bool bvh_node::box_compare(shared_ptr<hittable> a, shared_ptr<hittable> b, int axis) {
    bounding_record bound_a = a->bounding_box();
    bounding_record bound_b = b->bounding_box();

    return bound_a.bounding_box.min()[axis] < bound_b.bounding_box.min()[axis];
}

bool bvh_node::box_compare_x(shared_ptr<hittable> a, shared_ptr<hittable> b) {
    return bvh_node::box_compare(a, b, 0);
}

bool bvh_node::box_compare_y(shared_ptr<hittable> a, shared_ptr<hittable> b) {
    return bvh_node::box_compare(a, b, 1);
}

bool bvh_node::box_compare_z(shared_ptr<hittable> a, shared_ptr<hittable> b) {
    return bvh_node::box_compare(a, b, 2);
}