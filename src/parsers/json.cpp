#include "json.h"

#include <cstdio>
#include <iostream>

// ************************************************************************************
bool JSON_PAE::load_2d_matrix(matrix_t& matrix, const sajson::value& node)
{
	auto size = node.get_length();

	matrix.resize(size, size);
	matrix.set_frame_to_zero();

	for (size_t i = 0; i < size; ++i)
	{
		auto matrix_row = matrix.ptr((int) i);
		const auto& json_row = node.get_array_element(i);

		auto length = json_row.get_length();

		if (length != size)
			return false;

		for (size_t j = 0; j < size; ++j)
			matrix_row[j] = (matrix_elem_t) json_row.get_array_element(j).get_integer_value();
	}

	return true;
}
// EOF
