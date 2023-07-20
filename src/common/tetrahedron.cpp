#include "tetrahedron.h"

#include <iostream>

using namespace std;

void test_tetrahedron()
{
	tetrahedron_t t1({ 5, 5, 5 }, { 7, -18, 9 }, { 6, 8, 4 }, { 6, 5, 8 });

	tetrahedron_t t2(t1.get_b1(), t1.get_b2(), t1.get_b3(), t1.get_dist_q1(), t1.get_dist_q2(), t1.get_dist_q3());

	auto qs = t2.get_q_both();
}
