#include "yz_rect.h"

yz_rect::yz_rect(double y0, double y1, double z0, double z1, double k) {
    this->y0 = y0;
    this->y1 = y1;
    this->z0 = z0;
    this->z1 = z1;
    this->k = k;
}

hit_record yz_rect::hit(ray r, double t_min, double t_max) {
    hit_record hit;
    hit.is_hit = true;

    auto t = (k - r.origin().x()) / r.direction().x();
    if (t < t_min || t > t_max) {
        hit.is_hit = false;
    }

    auto y = r.origin().y() + t * r.direction().y();
    auto z = r.origin().z() + t * r.direction().z();

    if (y < this->y0 || y > this->y1 || z < this->z0 || z > this->z1) {
        hit.is_hit = false;
    }

    if (hit.is_hit) {
        hit.t = t;
        hit.p = r.at(hit.t);
    }

    return hit;
}

bounding_record yz_rect::bounding_box() {
    bounding_record bound;

    bound.is_hit = true;
    bound.bounding_box = aabb(
        point3(k - 0.001, this->y0, this->z0),
        point3(k + 0.001, this->y1, this->z1)
    );

    return bound;
}