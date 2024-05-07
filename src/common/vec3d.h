#pragma once

#include <cmath>

struct Vec3Dd
{
	double x, y, z;

	Vec3Dd() : x(0), y(0), z(0)
	{}

	Vec3Dd(double x, double y, double z) : x(x), y(y), z(z)
	{}

	Vec3Dd operator-() const
	{
		return Vec3Dd(-x, -y, -z);
	}

	Vec3Dd& operator+=(const Vec3Dd& a)
	{
		x += a.x;
		y += a.y;
		z += a.z;

		return *this;
	}

	bool operator<(const Vec3Dd& rhs) const
	{
		if (x != rhs.x)
			return x < rhs.x;
		if (y != rhs.y)
			return y < rhs.y;
		return z < rhs.z;
	}

	friend double vector_length(const Vec3Dd& a);
	friend Vec3Dd operator-(const Vec3Dd& a, const Vec3Dd& b);
	friend Vec3Dd operator+(const Vec3Dd& a, const Vec3Dd& b);
	friend Vec3Dd operator*(double d, const Vec3Dd& a);
	friend Vec3Dd normalize_vector(const Vec3Dd& a);
	friend double dot_product(const Vec3Dd& a, const Vec3Dd& b);
	friend Vec3Dd perp_vector(const Vec3Dd& a, const Vec3Dd& b);
	friend Vec3Dd cross_product(const Vec3Dd& a, const Vec3Dd& b);
};

inline double vector_length(const Vec3Dd& a)
{
	return std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline Vec3Dd operator-(const Vec3Dd& a, const Vec3Dd& b)
{
	return Vec3Dd(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vec3Dd operator+(const Vec3Dd& a, const Vec3Dd& b)
{
	return Vec3Dd(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec3Dd operator*(double d, const Vec3Dd& a)
{
	return Vec3Dd(d * a.x, d * a.y, d * a.z);
}

inline Vec3Dd normalize_vector(const Vec3Dd& a)
{
	double len = vector_length(a);

	return Vec3Dd(a.x / len, a.y / len, a.z / len);
}

inline double dot_product(const Vec3Dd& a, const Vec3Dd& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3Dd perp_vector(const Vec3Dd& a, const Vec3Dd& b)
{
	auto dp = dot_product(a, b);

	return Vec3Dd(a.x - b.x * dp, a.y - b.y * dp, a.z - b.z * dp);
}

inline Vec3Dd cross_product(const Vec3Dd& a, const Vec3Dd& b)
{
	return Vec3Dd(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
