#pragma once

#include "arr3.h"

class ray
{
private:
    vector3 org;
    vector3 dir;

public:
    ray();
    ray(point3 origin, vector3 direction);

    point3 origin();
    vector3 direction();

    vector3 at(double t);
};