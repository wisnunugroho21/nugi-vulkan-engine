#pragma once

#include <limits>

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.141592653589;

double random_double();
double random_double(double min, double max);
double clamp(double value, double min, double max);

double degrees_to_radians(double degrees);
int rand_int(int min, int max);