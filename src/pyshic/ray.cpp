#include "ray.h"

ray::ray() {

}

ray::ray(point3 origin, vector3 direction)
{
    this->org = origin;
    this->dir = direction;
}

point3 ray::origin()
{
    return this->org;
}

vector3 ray::direction()
{
    return this->dir;
}

vector3 ray::at(double t)
{
    vector3 a = this->org;
    vector3 b = t * this->dir;

    return a + b;
}