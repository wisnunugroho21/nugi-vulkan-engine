#include "box.h"

box::box(point3 box_min, point3 box_max) {
	this->p0 = box_min;
	this->p1 = box_max;

	this->rectangles.push_back(make_shared<xy_rect>(this->p0.x(), this->p1.x(), this->p0.y(), this->p1.y(), this->p1.z()));
	this->rectangles.push_back(make_shared<xy_rect>(this->p0.x(), this->p1.x(), this->p0.y(), this->p1.y(), this->p0.z()));

	this->rectangles.push_back(make_shared<xz_rect>(this->p0.x(), this->p1.x(), this->p0.z(), this->p1.z(), this->p1.y()));
	this->rectangles.push_back(make_shared<xz_rect>(this->p0.x(), this->p1.x(), this->p0.z(), this->p1.z(), this->p0.y()));

	this->rectangles.push_back(make_shared<yz_rect>(this->p0.y(), this->p1.y(), this->p0.z(), this->p1.z(), this->p1.x()));
	this->rectangles.push_back(make_shared<yz_rect>(this->p0.y(), this->p1.y(), this->p0.z(), this->p1.z(), this->p0.x()));
}

hit_record box::hit(ray r, double t_min, double t_max) {
	hit_record res;
	double closest = t_max;

	for (auto object : this->rectangles) {
		hit_record hitted_obj = object->hit(r, t_min, closest);

		if (hitted_obj.is_hit) {
			res = hitted_obj;
			res.objectId = this->objectId;
			
			closest = hitted_obj.t;
		}
	}

	return res;
}

bounding_record box::bounding_box() {
	bounding_record bound;

	bound.is_hit = true;
	bound.bounding_box = aabb(this->p0, this->p1);

	return bound;
}