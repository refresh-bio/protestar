#pragma once

#include <string>
#include <vector>

#include "../3rd-party/sajson/sajson.h"

#include "../common/defs.h"
#include "../compressors/matrix.h"

using namespace std;

// *****************************************************************
class JSON_PAE
{
	vector<uint8_t> json_data;
	vector<uint8_t> json_txt;

	bool load_2d_matrix(matrix_t& matrix, const sajson::value& node);

	void append_int(int x, vector<uint8_t>& vc);
	void append_double(double x, int prec, vector<uint8_t>& vc);
	void append_string(const string& s, vector<uint8_t>& vc);

	inline bool sajson_success(const sajson::document& doc);

	void sajson_traverse(matrix_t& matrix, double &max_declared_pae, const sajson::value& node);

	void decode_JSON(const matrix_t& matrix, double max_declared_pae);

public:
	JSON_PAE() = default;
	~JSON_PAE() = default;

	size_t load(const string& file_name, matrix_t& matrix, double& max_declared_pae);
	size_t load(const vector<uint8_t>& json_data, matrix_t& matrix, double& max_declared_pae);
	bool save(const string& file_name, const matrix_t& matrix, double max_declared_pae);
	bool contents(vector<char> &contents, const matrix_t& matrix, double max_declared_pae);
};

// EOF