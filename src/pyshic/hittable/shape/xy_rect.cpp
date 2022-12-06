#include "xy_rect.h"

xy_rect::xy_rect(double x0, double x1, double y0, double y1, double k) {
    this->x0 = x0;
    this->x1 = x1;
    this->y0 = y0;
    this->y1 = y1;
    this->k = k;
}

hit_record xy_rect::hit(ray r, double t_min, double t_max) {
    hit_record hit;
    hit.is_hit = true;

    auto t = (k - r.origin().z()) / r.direction().z();
    if (t < t_min || t > t_max) {
        hit.is_hit = false;
    }

    auto x = r.origin().x() + t * r.direction().x();
    auto y = r.origin().y() + t * r.direction().y();

    if (x < this->x0 || x > this->x1 || y < this->y0 || y > this->y1) {
        hit.is_hit = false;
    }

    if (hit.is_hit) {
        hit.t = t;
        hit.p = r.at(hit.t);
    }

    return hit;
}

bounding_record xy_rect::bounding_box() {
    bounding_record bound;

    bound.is_hit = true;
    bound.bounding_box = aabb(
        point3(this->x0, this->y0, k - 0.001), 
        point3(this->x1, this->y1, k + 0.001)
    );

    return bound;
}