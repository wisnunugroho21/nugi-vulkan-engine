#include <cmath>

#include "arr3.h"

arr3::arr3()
{

}

arr3::arr3(double e1, double e2, double e3)
{
	this->e[0] = e1;
	this->e[1] = e2;
	this->e[2] = e3;
}

double arr3::e1()
{
	return this->e[0];
}

double arr3::e2()
{
	return this->e[1];
}

double arr3::e3()
{
	return this->e[2];
}

double arr3::operator [](int i) const
{
	return this->e[i];
}

double& arr3::operator [](int i)
{
	return this->e[i];
}

arr3 arr3::operator -()
{
	return arr3(-1 * this->e[0], -1 * this->e[1], -1 * this->e[2]);
}


arr3& arr3::operator += (arr3& a)
{
	this->e[0] += a.e1();
	this->e[1] += a.e2();
	this->e[2] += a.e3();

	return *this;
}

arr3& arr3::operator += (double t)
{
	this->e[0] += t;
	this->e[1] += t;
	this->e[2] += t;

	return *this;
}

arr3& arr3::operator -= (arr3& a)
{
	this->e[0] -= a.e1();
	this->e[1] -= a.e2();
	this->e[2] -= a.e3();

	return *this;
}

arr3& arr3::operator -= (double t)
{
	this->e[0] -= t;
	this->e[1] -= t;
	this->e[2] -= t;

	return *this;
}

arr3& arr3::operator *= (arr3& a)
{
	this->e[0] *= a.e1();
	this->e[1] *= a.e2();
	this->e[2] *= a.e3();

	return *this;
}

arr3& arr3::operator *= (double t)
{
	this->e[0] *= t;
	this->e[1] *= t;
	this->e[2] *= t;

	return *this;
}

arr3& arr3::operator /= (arr3& a)
{
	this->e[0] /= a.e1();
	this->e[1] /= a.e2();
	this->e[2] /= a.e3();

	return *this;
}

arr3& arr3::operator /= (double t)
{
	this->e[0] /= t;
	this->e[1] /= t;
	this->e[2] /= t;

	return *this;
}

double arr3::length_squared()
{
	return this->e1() * this->e1() + this->e2() * this->e2() + this->e3() * this->e3();
}

double arr3::length()
{
	return sqrt(this->length_squared());
}

arr3 arr3::unit_vector()
{
	return arr3(this->e1() / this->length(), this->e2() / this->length(), this->e3() / this->length());
}

bool arr3::near_zero() {
	double s = 1e-8;
	return (fabs(this->e1()) < s) && (fabs(this->e2()) < s) && (fabs(this->e3()) < s);
}


arr3 arr3::random() {
	return arr3(random_double(), random_double(), random_double());
}

arr3 arr3::random(double min, double max) {
	return arr3(random_double(min, max), random_double(min, max), random_double(min, max));
}

arr3 arr3::random_unit_length() {
	while (true) {
		arr3 p = arr3::random(-1, 1);
		if (p.length_squared() < 1) {
			return p;
		}
	}
}

arr3 arr3::random_unit_vector() {
	return arr3::random_unit_length().unit_vector();
}

arr3 arr3::random_in_unit_disk() {
	while (true) {
		arr3 p = arr3(random_double(-1, 1), random_double(-1, 1), 0);
		if (p.length_squared() < 1) {
			return p;
		}
	}
}

arr3 arr3::random_in_unit_sphere() {
    while (true) {
        auto p = arr3::random(-1,1);
        if (p.length_squared() < 1) {
			return p;
		}
    }
}


arr3 operator + (arr3 u, arr3 v)
{
	return arr3(u.e1() + v.e1(), u.e2() + v.e2(), u.e3() + v.e3());
}

arr3 operator - (arr3 u, arr3 v)
{
	return arr3(u.e1() - v.e1(), u.e2() - v.e2(), u.e3() - v.e3());
}

arr3 operator * (arr3 u, arr3 v)
{
	return arr3(u.e1() * v.e1(), u.e2() * v.e2(), u.e3() * v.e3());
}

arr3 operator / (arr3 u, arr3 v)
{
	return arr3(u.e1() / v.e1(), u.e2() / v.e2(), u.e3() / v.e3());
}


arr3 operator + (double t, arr3 v)
{
	return arr3(t + v.e1(), t + v.e2(), t + v.e3());
}

arr3 operator - (double t, arr3 v)
{
	return arr3(t - v.e1(), t - v.e2(), t - v.e3());
}

arr3 operator * (double t, arr3 v)
{
	return arr3(t * v.e1(), t * v.e2(), t * v.e3());
}

arr3 operator / (double t, arr3 v)
{
	return arr3(t / v.e1(), t / v.e2(), t / v.e3());
}


arr3 operator + (arr3 u, double t)
{
	return arr3(u.e1() + t, u.e2() + t, u.e3() + t);
}

arr3 operator - (arr3 u, double t)
{
	return arr3(u.e1() - t, u.e2() - t, u.e3() - t);
}

arr3 operator * (arr3 u, double t)
{
	return arr3(u.e1() * t, u.e2() * t, u.e3() * t);
}

arr3 operator / (arr3 u, double t)
{
	return arr3(u.e1() / t, u.e2() / t, u.e3() / t);
}

double dot(arr3 u, arr3 v)
{
	return u.e1() * v.e1() + u.e2() * v.e2() + u.e3() * v.e3();
}

arr3 cross(arr3 u, arr3 v)
{
	return arr3(
		u.e2() * v.e3() - u.e3() * v.e2(),
		u.e3() * v.e1() - u.e1() * v.e3(),
		u.e1() * v.e2() - u.e2() * v.e1()
	);
}

vector3::vector3()
{

}

vector3::vector3(double x, double y, double z) : arr3(x, y, z)
{

}

vector3::vector3(const arr3& x) : arr3(x)
{

}


double vector3::x()
{
	return this->e1();
}

double vector3::y()
{
	return this->e2();
}

double vector3::z()
{
	return this->e3();
}


color::color() : arr3()
{

}

color::color(double r, double g, double b) : arr3(r, g, b)
{

}

color::color(const arr3& x) : arr3(x)
{

}

double color::r()
{
	return this->e1();
}

double color::g()
{
	return this->e2();
}

double color::b()
{
	return this->e3();
}