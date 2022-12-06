#include <math.h>

#include "sphere.h"

sphere::sphere(point3 center, double radius) {
    this->center = center;
    this->radius = radius;
}

hit_record sphere::hit(ray r, double t_min, double t_max) {
    vector3 oc = r.origin() - this->center;

    double a = dot(r.direction(), r.direction());
    double b = 2.0 * dot(oc, r.direction());
    double c = dot(oc, oc) - this->radius * this->radius;

    double discriminant = b * b - 4 * a * c;

    hit_record hit;
    double root = 0.0;

    if (discriminant >= 0) {
        hit.is_hit = true;

        root = (-b - sqrt(discriminant)) / (2.0 * a);
        if (root < t_min || root > t_max) {
            root = (-b + sqrt(discriminant)) / (2.0 * a);            
            if (root < t_min || root > t_max) {
                hit.is_hit = false;
            }
        }
    }

    if (hit.is_hit) {
        hit.t = root;
        hit.p = r.at(hit.t);
        hit.objectId = this->objectId;
    }

    return hit;
}

bounding_record sphere::bounding_box() {
    bounding_record bound;

    bound.is_hit = true;
    bound.bounding_box = aabb(
        this->center - vector3(this->radius, this->radius, this->radius),
        this->center + vector3(this->radius, this->radius, this->radius)
    );

    return bound;
}