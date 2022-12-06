#include "hittable_list.h"

hittable_list::hittable_list()
{

}

hittable_list::hittable_list(shared_ptr<hittable> object)
{
    this->add(object);
}

void hittable_list::clear()
{
    this->objects.clear();
}

void hittable_list::add(shared_ptr<hittable> object)
{
    this->objects.push_back(object);
}

vector<shared_ptr<hittable>> hittable_list::getAll() 
{
    return this->objects;
}

hit_record hittable_list::hit(ray r, double t_min, double t_max){
    hit_record hitted;
    double closest = t_max;

    for (shared_ptr<hittable> object : this->objects) {
        hit_record hit = object->hit(r, t_min, closest);

        if (hit.is_hit) {
            hitted = hit;
            closest = hitted.t;
        }
    }

    return hitted;
}

bounding_record hittable_list::bounding_box() {
    bounding_record res_bound;

    if (this->objects.empty()) {
        return res_bound;
    }

    bool first_box = true;
    for (shared_ptr<hittable> object : this->objects) {
        bounding_record obj_bound = object->bounding_box();

        if (!obj_bound.is_hit) {
            return res_bound;
        }

        res_bound.bounding_box = first_box ? obj_bound.bounding_box : aabb::surrounding_box(res_bound.bounding_box, obj_bound.bounding_box);
        first_box = false;
    }

    res_bound.is_hit = true;
    return res_bound;
}