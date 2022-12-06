#include "aabb.h"

aabb::aabb() {

}

aabb::aabb(point3 minimum, point3 maximum) {
	this->minimum = minimum;
	this->maximum = maximum;
}

point3 aabb::min() {
	return this->minimum;
}

point3 aabb::max() {
	return this->maximum;
}

bool aabb::hit(ray r, double t_min, double t_max) {
	for (int a = 0; a < 3; a++) {
		auto invD = 1.0 / r.direction()[a];

		auto t0 = (this->min()[a] - r.origin()[a]) * invD;
		auto t1 = (this->max()[a] - r.origin()[a]) * invD;

		if (invD < 0.0) {
			auto temp = t1;
			t1 = t0;
			t0 = temp;
		}

		t_min = t0 > t_min ? t0 : t_min;
		t_max = t1 < t_max ? t1 : t_max;

		if (t_max <= t_min) {
			return false;
		}
	}

	return true;
}

aabb aabb::surrounding_box(aabb box0, aabb box1) {
	point3 small(
		fmin(box0.min().x(), box1.min().x()),
		fmin(box0.min().y(), box1.min().y()),
		fmin(box0.min().z(), box1.min().z())
	);

	point3 big(
		fmax(box0.max().x(), box1.max().x()),
		fmax(box0.max().y(), box1.max().y()),
		fmax(box0.max().z(), box1.max().z())
	);

	return aabb(small, big);
}