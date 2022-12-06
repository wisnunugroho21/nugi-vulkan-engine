#pragma once

#include "helper/helper.h"

class arr3
{
public:
    double e[3];

    arr3();
    arr3(double e1, double e2, double e3);

    double e1();
    double e2();
    double e3();
    
    double operator [](int i) const;
    double& operator [](int i);

    arr3 operator -();

    arr3& operator += (arr3 &a);
    arr3& operator += (double t);

    arr3& operator -= (arr3 &a);
    arr3& operator -= (double t);

    arr3& operator *= (arr3 &a);
    arr3& operator *= (double t);

    arr3& operator /= (arr3 &a);
    arr3& operator /= (double t);

    double length_squared();
    double length();
    arr3 unit_vector();
    bool near_zero();

    static arr3 random();
    static arr3 random(double min, double max);
    static arr3 random_unit_length();
    static arr3 random_unit_vector();
    static arr3 random_in_unit_disk();
    static arr3 random_in_unit_sphere();
};

arr3 operator + (arr3 u, arr3 v);
arr3 operator - (arr3 u, arr3 v);
arr3 operator * (arr3 u, arr3 v);
arr3 operator / (arr3 u, arr3 v);

arr3 operator + (double t, arr3 v);
arr3 operator - (double t, arr3 v);
arr3 operator * (double t, arr3 v);
arr3 operator / (double t, arr3 v);

arr3 operator + (arr3 u, double t);
arr3 operator - (arr3 u, double t);
arr3 operator * (arr3 u, double t);
arr3 operator / (arr3 u, double t);

double dot (arr3 u, arr3 v);
arr3 cross (arr3 u, arr3 v);

class vector3 : public arr3
{
public:
    vector3();
    vector3(double x, double y, double z);
    vector3(const arr3 &x);

    double x();
    double y();
    double z();
};

class color : public arr3
{
public:
    color();
    color(double r, double g, double b);
    color(const arr3 &x);

    double r();
    double g();
    double b();
};

using point3 = vector3;