#include "json_base.h"

#include <cstdio>
#include <iostream>

// *****************************************************************
void JSON_PAE_Base::decode_JSON(const matrix_t& matrix, double max_declared_pae)
{
	json_txt.clear();
	json_txt.reserve(matrix.get_n_rows() * matrix.get_n_rows() * 3 + matrix.get_n_rows() * 20 + 100);

	append_string("[{\"predicted_aligned_error\":[", json_txt);

	size_t size = matrix.get_n_rows();

	for (size_t i = 0; i < size; ++i)
	{
		const auto row = matrix.ptr((int)i);
		json_txt.emplace_back('[');

		for (size_t j = 0; j < size - 1; ++j)
		{
			append_int(row[j], json_txt);
			json_txt.emplace_back(',');
		}

		append_int(row[size - 1], json_txt);

		json_txt.emplace_back(']');
		if (i + 1 < size)
			json_txt.emplace_back(',');
	}

	append_string("],\"max_predicted_aligned_error\":", json_txt);
	append_double(max_declared_pae, 2, json_txt);
	append_string("}]", json_txt);
}

// *****************************************************************
bool JSON_PAE_Base::contents(vector<char>& contents, const matrix_t& matrix, double max_declared_pae)
{
	// !!! TODO: Consider chaning json_txt type to vector<char> 

	decode_JSON(matrix, max_declared_pae);

	contents.assign(json_txt.begin(), json_txt.end());

	return true;
}

// ************************************************************************************
void JSON_PAE_Base::append_int(int x, vector<uint8_t>& vc)
{
	if (x >= 100)
	{
		vc.emplace_back('0' + x / 100);
		x %= 100;

		vc.emplace_back('0' + x / 10);
		vc.emplace_back('0' + x % 10);
	}
	else if (x >= 10)
	{
		vc.emplace_back('0' + x / 10);
		vc.emplace_back('0' + x % 10);
	}
	else
		vc.emplace_back('0' + x);
}

// ************************************************************************************
void JSON_PAE_Base::append_double(double x, int prec, vector<uint8_t>& vc)
{
	append_int((int)x, vc);

	int mult = 10;
	for (int i = 1; i < prec; ++i)
		mult *= 10;

	x *= mult;
	int ix = (int)x;
	ix %= mult;
	ix += mult;

	append_int(ix, vc);
	*(vc.end() - prec - 1) = '.';
}

// ************************************************************************************
void JSON_PAE_Base::append_string(const string& s, vector<uint8_t>& vc)
{
	for (auto c : s)
		vc.emplace_back(c);
}

// EOF
