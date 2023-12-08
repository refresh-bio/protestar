#pragma once

#include <string>
#include <vector>

#include "../3rd-party/sajson/sajson.h"

#include "../common/defs.h"
#include "../compressors/matrix.h"
#include "../parsers/json_base.h"

using namespace std;

// *****************************************************************
class JSON_PAE : public JSON_PAE_Base
{
	bool load_2d_matrix(matrix_t& matrix, const sajson::value& node);

	inline bool sajson_success(const sajson::document& doc);

	void sajson_traverse(matrix_t& matrix, double &max_declared_pae, const sajson::value& node);

public:
	JSON_PAE() = default;
	~JSON_PAE() = default;

	size_t load(const string& file_name, matrix_t& matrix, double& max_declared_pae);
	size_t load(const vector<uint8_t>& json_data, matrix_t& matrix, double& max_declared_pae);
	bool save(const string& file_name, const matrix_t& matrix, double max_declared_pae);
};

// EOF