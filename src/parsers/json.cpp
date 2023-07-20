#include "json.h"
#include "../core/io.h"

#include <cstdio>
#include <iostream>

// ************************************************************************************
size_t JSON_PAE::load(const string& file_name, matrix_t& matrix, double& max_declared_pae)
{
	if (!load_file(file_name, json_data))
		return false;

	const sajson::document& document = sajson::parse(sajson::single_allocation(), sajson::mutable_string_view(json_data.size(), (char*)json_data.data()));

	if (!sajson_success(document))
		return 0;

	sajson_traverse(matrix, max_declared_pae, document.get_root());

	return json_data.size();
}

// ************************************************************************************
size_t JSON_PAE::load(const vector<uint8_t>& json_data, matrix_t& matrix, double& max_declared_pae)
{
	if (json_data.empty())
		return false;

	const sajson::document& document = sajson::parse(sajson::single_allocation(), sajson::mutable_string_view(json_data.size(), (char*)json_data.data()));

	if (!sajson_success(document))
		return 0;

	sajson_traverse(matrix, max_declared_pae, document.get_root());

	return json_data.size();
}

// *****************************************************************
void JSON_PAE::decode_JSON(const matrix_t& matrix, double max_declared_pae)
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
bool JSON_PAE::contents(vector<char>& contents, const matrix_t& matrix, double max_declared_pae)
{
	// !!! TODO: Consider chaning json_txt type to vector<char> 

	decode_JSON(matrix, max_declared_pae);

	contents.assign(json_txt.begin(), json_txt.end());

	return true;
}

// ************************************************************************************
bool JSON_PAE::save(const string& file_name, const matrix_t& matrix, double max_declared_pae)
{
	decode_JSON(matrix, max_declared_pae);

	return save_file(file_name, json_txt);
}

// ************************************************************************************
void JSON_PAE::sajson_traverse(matrix_t& matrix, double& max_declared_pae, const sajson::value& node)
{
	using namespace sajson;

	switch (node.get_type()) {
	case TYPE_NULL:
	case TYPE_FALSE:
	case TYPE_TRUE:
	case TYPE_STRING:
	case TYPE_DOUBLE:
	case TYPE_INTEGER:
		break;

	case TYPE_ARRAY: {
		auto length = node.get_length();

		for (size_t i = 0; i < length; ++i) 
			sajson_traverse(matrix, max_declared_pae, node.get_array_element(i));

		break;
	}

	case TYPE_OBJECT: {
		auto length = node.get_length();

		for (auto i = 0u; i < length; ++i) 
			if (node.get_object_key(i).as_string() == "predicted_aligned_error")
				load_2d_matrix(matrix, node.get_object_value(i));
			else if (node.get_object_key(i).as_string() == "max_predicted_aligned_error")
				max_declared_pae = node.get_object_value(i).get_double_value();
			else
				sajson_traverse(matrix, max_declared_pae, node.get_object_value(i));
		
		break;
	}

	default:
		assert(false && "unknown node type");
	}
}

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

// ************************************************************************************
void JSON_PAE::append_int(int x, vector<uint8_t>& vc)
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
void JSON_PAE::append_double(double x, int prec, vector<uint8_t>& vc)
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
void JSON_PAE::append_string(const string& s, vector<uint8_t>& vc)
{
	for (auto c : s)
		vc.emplace_back(c);
}

// ************************************************************************************
bool JSON_PAE::sajson_success(const sajson::document& doc)
{
	if (!doc.is_valid()) 
	{
		fprintf(stderr, "%s\n", doc.get_error_message_as_cstring());
		return false;
	}

	return true;
}

// EOF
