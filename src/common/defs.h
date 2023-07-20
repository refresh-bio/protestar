#pragma once

#include <cinttypes>
#include <vector>

using namespace std;

#include "../compressors/matrix.h"

template<typename T> struct coords_t
{
	T x, y, z;

	bool operator<(const coords_t<T>& rhs) const
	{
		if (x != rhs.x)
			return x < rhs.x;
		if (y != rhs.y)
			return y < rhs.y;
		return z < rhs.z;
	}
};

using int_coords_t = coords_t<int64_t>;
using double_coords_t = coords_t<double>;

const unsigned int MATRIX_FRAME_SIZE = 3;

using matrix_elem_t = int;
using matrix_t = matrix2d_frame_t<matrix_elem_t, MATRIX_FRAME_SIZE>;
using packed_data_t = vector<uint8_t>;

#if __cplusplus > 201700L
#define FALL_THROUGH [[fallthrough]];
#else
#define FALL_THROUGH
#endif

// Just to check what is the compression of cart values
//#define NO_CART_STORAGE

// EOF
