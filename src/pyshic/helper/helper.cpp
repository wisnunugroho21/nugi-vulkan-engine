#include <random>

#include "helper.h"

double random_double() {
    return rand() / (RAND_MAX + 1.0);
}

double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

double clamp(double value, double min, double max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

int rand_int(int min, int max) {
    return static_cast<int>(random_double(min, max + 1));
}