#pragma once

#include "../common/defs.h"

#include <rc.h>
#include <cinttypes>
#include "matrix.h"

using namespace refresh;

// *****************************************************************
class PAECompressor
{
	const uint32_t max_lossy_level = 4;

	using rc_flag_t = rc_simple_fixed<vector_io_stream, 7, 1 << 11, 4>;
	using rc_large_t = rc_simple<vector_io_stream, 1 << 13, 16>;

	rc_context_t cmp5(const int a, const int b)
	{
		return
			(((rc_context_t)(a == b)) << 2) +
			(((rc_context_t)(a < b)) << 1) +
			(((rc_context_t)abs(a - b)) > 1);
	}

	rc_context_t cmp3(const int a, const int b)
	{
		return
			(((rc_context_t)(a == b)) << 1) +
			(((rc_context_t)(a < b)));
	}

	vector<int> vert_stat, horz_stat;

	rc_context_t det_ctx(const matrix_t& matrix, int row, int col, bool use_horiz, int* p_x, int* p_r1, int* p_r2, int* p_r3);
	rc_context_t ext_ctx_lossy(rc_context_t ctx, int v, uint32_t lossy_level);
	rc_context_t ext_ctx_lossless(rc_context_t ctx, int v);
	int pred(const matrix_t& matrix, int row, int col, bool use_horiz, int* p_x, int* p_r1);
	bool is_horizontal_better(const matrix_t& matrix, int row, int col, int* p_x, int* p_r1);

	void clear_model();

	// For compression
	bool compress_impl(matrix_t& matrix, vector<uint8_t>& packed_data, uint32_t lossy_level);
	void lossy_encoding(matrix_t& matrix, uint32_t lossy_level);
	void add_int(vector<uint8_t>& data, uint32_t x, int no_bytes);

	// For decompression
	bool decompress_impl(vector<uint8_t>& packed_data, matrix_t& matrix, uint32_t lossy_level);
	void lossy_decoding(matrix_t& matrix, uint32_t lossy_level);
	uint32_t get_int(vector<uint8_t>& data, int no_bytes);

public:
	PAECompressor() = default;
	~PAECompressor() = default;

	bool compress(matrix_t& matrix, const double max_declared_pae, uint32_t lossy_level, packed_data_t& packed_data);
	bool decompress(packed_data_t& packed_data, matrix_t& matrix, double& max_declared_pae, uint32_t& lossy_level);
};

// EOF
