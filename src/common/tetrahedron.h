#pragma once

#include <cmath>
#include <vectorclass.h>
#include <vector3d/vector3d.h>

#include <utility>

#include "../common/defs.h"

// *****************************************************************
static inline Vec3Dd perp_vector(Vec3Dd const a, Vec3Dd const b)
{
	auto dp = dot_product(a, b);

	return a - b * dp;
}

// *****************************************************************
struct tetrahedron_t
{
private:
	enum class type_t {points, points_distances, distances };

	type_t type;
	bool known_b_coords = false;
	bool known_q_coords = false;
	bool known_b_distances = false;
	bool known_q_distances = false;

	double_coords_t b1, b2, b3, q, q_alt;
	double dist_b12, dist_b13, dist_b23, dist_q1, dist_q2, dist_q3;

	double calc_dist(const double_coords_t a, const double_coords_t b)
	{
		Vec3Dd va, vb;

		va.load((const double*) & a);
		vb.load((const double*) & b);

		return vector_length(va - vb);

//		return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
	}

	void calc_b_dist()
	{
		Vec3Dd p1{ b1.x, b1.y, b1.z };
		Vec3Dd p2{ b2.x, b2.y, b2.z };
		Vec3Dd p3{ b3.x, b3.y, b3.z };

		Vec3Dd p12 = p1 - p2;
		Vec3Dd p13 = p1 - p3;
		Vec3Dd p23 = p2 - p3;

		dist_b12 = vector_length(p12);
		dist_b13 = vector_length(p13);
		dist_b23 = vector_length(p23);

		known_b_distances = true;
	}

	void calc_q_dist()
	{
		if (!known_b_coords)
			calc_b_coords();

		dist_q1 = calc_dist(q, b1);
		dist_q2 = calc_dist(q, b2);
		dist_q3 = calc_dist(q, b3);

		known_q_distances = true;
	}

	void calc_b_coords()
	{
		b1 = { 0, 0, 0 };
		b2 = { dist_b12, 0, 0 };

		double x = (dist_b12 * dist_b12 + dist_b13 * dist_b13 - dist_b23 * dist_b23) / dist_b12 / 2;
		double h2 = dist_b13 * dist_b13 - x * x;

		if (h2 < 0)
			b3 = { dist_b12 / 2, 0, 0 };		// Impossible tetrahedron
		else
			b3 = { x, std::sqrt(h2), 0 };
	}

	void calc_q_coords()
	{
		// Code based on https://math.stackexchange.com/questions/3753340/finding-the-coordinates-of-the-fourth-vertex-of-tetrahedron-given-coordinates-o

		if (!known_b_coords)
			calc_b_coords();

		Vec3Dd p1{ b1.x, b1.y, b1.z };
		Vec3Dd p2{ b2.x, b2.y, b2.z };
		Vec3Dd p3{ b3.x, b3.y, b3.z };

		Vec3Dd p12 = p1 - p2;
		Vec3Dd p13 = p1 - p3;
		Vec3Dd p23 = p2 - p3;

		if (!known_b_distances)
		{
			dist_b12 = vector_length(p12);
			dist_b13 = vector_length(p13);
			dist_b23 = vector_length(p23);
			known_b_distances = true;
		}
//			calc_b_dist();

//		auto u_axis = normalize_vector(p1 - p2);
		auto u_axis = normalize_vector(p12);
//		auto v_axis = normalize_vector(perp_vector(p3 - p1, u_axis));
		auto v_axis = normalize_vector(perp_vector(-p13, u_axis));
		auto w_axis = cross_product(u_axis, v_axis);

//		auto u2 = dot_product(p2 - p1, u_axis);
		auto u2 = dot_product(-p12, u_axis);
		//auto u3 = dot_product(p3 - p1, u_axis);
		auto u3 = dot_product(-p13, u_axis);
//		auto v3 = dot_product(p3 - p1, v_axis);
		auto v3 = dot_product(-p13, v_axis);
		
		auto u = (dist_q1 * dist_q1 - dist_q2 * dist_q2 + u2 * u2) / (2 * u2);
		auto v = (dist_q1 * dist_q1 - dist_q3 * dist_q3 + u3 * u3 + v3 * v3 - 2 * u * u3) / (2 * v3);
		auto w2 = dist_q1 * dist_q1 - u * u - v * v;
		double w = w2 > 0 ? sqrt(w2) : 0;

		auto wx = w * w_axis;

		auto vq1 = p1 + u * u_axis + v * v_axis;
		auto vq2 = vq1 - wx;
		vq1 += wx;

		q.x = vq1.get_x();
		q.y = vq1.get_y();
		q.z = vq1.get_z();

		q_alt.x = vq2.get_x();
		q_alt.y = vq2.get_y();
		q_alt.z = vq2.get_z();

		known_q_coords = true;
	}

public:
	tetrahedron_t() : 
		type(type_t::points),
		known_b_coords(true),
		known_q_coords(true),
		b1{ 0, 0, 0 }, b2{ 0, 0, 0 }, b3{ 0, 0, 0 }, q{ 0, 0, 0 }, q_alt{},
		dist_b12{}, dist_b13{}, dist_b23{}, dist_q1{}, dist_q2{}, dist_q3{}
	{}

	tetrahedron_t(const double_coords_t& _b1, const double_coords_t& _b2, const double_coords_t& _b3, const double_coords_t& _q) :
		type(type_t::points),
		known_b_coords(true),
		known_q_coords(true),
		b1(_b1), b2(_b2), b3(_b3), q(_q), q_alt{}
	{}

	tetrahedron_t(const double_coords_t& _b1, const double_coords_t& _b2, const double_coords_t& _b3, const double _dist_q1, const double _dist_q2, const double _dist_q3) :
		type(type_t::points_distances),
		known_b_coords(true),
		known_q_distances(true),
		b1(_b1), b2(_b2), b3(_b3),
		q{}, q_alt{},
		dist_b12{}, dist_b13{}, dist_b23{},
		dist_q1(_dist_q1), dist_q2(_dist_q2), dist_q3(_dist_q3)
	{}

	tetrahedron_t(const double _dist_b12, const double _dist_b13, const double _dist_b23, const double _dist_q1, const double _dist_q2, const double _dist_q3) :
		type(type_t::distances),
		known_b_distances(true),
		known_q_distances(true),
		q{}, q_alt{},
		dist_b12(_dist_b12), dist_b13(_dist_b13), dist_b23(_dist_b23),
		dist_q1(_dist_q1), dist_q2(_dist_q2), dist_q3(_dist_q3)
	{}

	double_coords_t get_b1()
	{
		if (!known_b_coords)
			calc_b_coords();
		return b1;
	}

	double_coords_t get_b2()
	{
		if (!known_b_coords)
			calc_b_coords();
		return b2;
	}

	double_coords_t get_b3()
	{
		if (!known_b_coords)
			calc_b_coords();
		return b3;
	}

	double get_dist_b12()
	{
		if (!known_b_distances)
			calc_b_dist();

		return dist_b12;
	}

	double get_dist_b13()
	{
		if (!known_b_distances)
			calc_b_dist();

		return dist_b13;
	}

	double get_dist_b23()
	{
		if (!known_b_distances)
			calc_b_dist();

		return dist_b23;
	}

	double get_dist_q1()
	{
		if (!known_q_distances)
			calc_q_dist();

		return dist_q1;
	}

	double get_dist_q2()
	{
		if (!known_q_distances)
			calc_q_dist();

		return dist_q2;
	}

	double get_dist_q3()
	{
		if (!known_q_distances)
			calc_q_dist();

		return dist_q3;
	}

	void nan_correction()
	{
		if (isnan(q.x) || isnan(q.y) || isnan(q.z) || isnan(q_alt.x) || isnan(q_alt.y) || isnan(q_alt.z))
		{
			q = b1;
			q_alt = b2;
		}
	}

	double_coords_t get_q()
	{
		if (!known_q_coords)
			calc_q_coords();

		nan_correction();

		return q;
	}

	double_coords_t get_q_alt()
	{
		if (!known_q_coords)
			calc_q_coords();

		nan_correction();

		return q_alt;
	}

	std::pair<double_coords_t, double_coords_t> get_q_both()
	{
		if (!known_q_coords)
			calc_q_coords();

		nan_correction();

		return std::make_pair(q, q_alt);
	}
};

// EOF
