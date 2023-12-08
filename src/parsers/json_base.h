#pragma once

#include <string>
#include <vector>

#include "../common/defs.h"
#include "../compressors/matrix.h"

using namespace std;

// *****************************************************************
class JSON_PAE_Base
{
protected:
	vector<uint8_t> json_data;
	vector<uint8_t> json_txt;

	void append_int(int x, vector<uint8_t>& vc);
	void append_double(double x, int prec, vector<uint8_t>& vc);
	void append_string(const string& s, vector<uint8_t>& vc);

	void decode_JSON(const matrix_t& matrix, double max_declared_pae);

public:
	JSON_PAE_Base() = default;
	~JSON_PAE_Base() = default;

	bool contents(vector<char> &contents, const matrix_t& matrix, double max_declared_pae);
};

// EOF