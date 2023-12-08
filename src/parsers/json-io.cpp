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
