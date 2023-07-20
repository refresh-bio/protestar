#include "pae-compressor.h"

#include <chrono>
#include <fstream>
#include <array>
#include <bitset>

// ************************************************************************************
void PAECompressor::clear_model()
{
	vert_stat.clear();
	horz_stat.clear();
}

// ************************************************************************************
rc_context_t PAECompressor::det_ctx(const matrix_t& matrix, int row, int col, bool use_horiz, int* p_x, int* p_r1, int* p_r2, int* p_r3)
{
	rc_context_t ctx = 0;

	int left1 = *(p_x - 1);
	int up1 = *p_r1;
	int left_up = *(p_r1 - 1);

	if (use_horiz)
	{
		int left2 = *(p_x - 2);
		int left3 = *(p_x - 3);
		int left_left_up = *(p_r1 - 2);

		auto ctx1 = cmp5(up1, left_up);
		auto ctx2 = cmp5(left1, left2);

		//		ctx = cmp5(up1, left_up);							ctx <<= 1;

		//		ctx = ctx1 * 5 + ctx2;
		//		ctx <<= 1;

		/*		ctx += up1 == left_up;									ctx <<= 1;
				ctx += up1 < left_up;									ctx <<= 1;
				ctx += abs(up1 - left_up) > 1;							ctx <<= 1;

				ctx += left1 == left2;									ctx <<= 1;
				ctx += left1 < left2;									ctx <<= 1;
				ctx += abs(left1 - left2) > 1;							ctx <<= 1;*/

		auto ctx3 = cmp3(left2, left3);

		/*		ctx += left2 == left3;									ctx <<= 1;
				ctx += left2 < left3;									ctx <<= 1;*/

		auto ctx4 = cmp3(left1, up1);
		/*		ctx += left1 == up1;									ctx <<= 1;
				ctx += left1 < up1;										ctx <<= 1;*/

				/*		ctx += left_up == left_left_up;							ctx <<= 1;
						ctx += left_up < left_left_up;							ctx <<= 1;*/

		auto ctx5 = cmp3(left_up, left_left_up);

		ctx = ((((ctx1 * 5 + ctx2) * 3 + ctx3) * 3 + ctx4) * 3 + ctx5) * 2;

		//		ctx <<= 1;

		ctx += 0;			// horizontal
	}
	else
	{
		int up2 = *p_r2;
		int up3 = *p_r3;
		int left_up_up = *(p_r2 - 1);

		auto ctx1 = cmp5(left1, left_up);
		/*		ctx += left1 == left_up;					ctx <<= 1;
				ctx += left1 < left_up;						ctx <<= 1;
				ctx += abs(left1 - left_up) > 1;			ctx <<= 1;*/

		auto ctx2 = cmp5(up1, up2);
		/*		ctx += up1 == up2;							ctx <<= 1;
				ctx += up1 < up2;							ctx <<= 1;
				ctx += abs(up1 - up2) > 1;					ctx <<= 1;*/

		auto ctx3 = cmp3(up2, up3);
		/*		ctx += up2 == up3;							ctx <<= 1;
				ctx += up2 < up3;							ctx <<= 1;*/

		auto ctx4 = cmp3(left1, up1);
		/*		ctx += left1 == up1;						ctx <<= 1;
				ctx += left1 < up1;							ctx <<= 1;*/

		auto ctx5 = cmp3(left_up, left_up_up);
		/*		ctx += left_up == left_up_up;				ctx <<= 1;
				ctx += left_up < left_up_up;				ctx <<= 1;*/

		ctx = ((((ctx1 * 5 + ctx2) * 3 + ctx3) * 3 + ctx4) * 3 + ctx5) * 2;

		//		ctx <<= 1;

		ctx += 1;					// vertical
	}

	return ctx;
}

// ************************************************************************************
rc_context_t PAECompressor::ext_ctx_lossy(rc_context_t ctx, int v, uint32_t lossy_level)
{
	//	ctx <<= 3;

	if (lossy_level == 1)
	{
		// ll == 1						// 0, 1, ..., 19, 20-21, 22-23, 24-27, 28-32
		ctx *= 6;

		if (v < 8)
			;
		else if (v < 16)
		{
			ctx += 1;
		}
		else
		{
			if (v <= 20)
				ctx += 2;
			else if (v == 21)
				ctx += 3;
			else if (v == 22)
				ctx += 4;
			else
				ctx += 5;
		}
	}
	else if (lossy_level == 2)
	{
		// ll == 2						// 0, 1, ..., 9, 10-12, 13-15, 16-19, 20-23, 24-27, 28-32
		ctx *= 6;

		if (v < 8)
			;
		else if (v < 10)
		{
			ctx += 1;
		}
		else
		{
			if (v <= 12)
				ctx += 2;
			else if (v == 13)
				ctx += 3;
			else if (v == 14)
				ctx += 4;
			else
				ctx += 5;
		}
	}
	else if (lossy_level == 3)
	{
		// ll == 3						// 0, 1, ..., 5, 6-7, 8-10, 11-14, 15-19, 20-25, 26-32
		ctx *= 6;

		if (v < 4)
			;
		else if (v < 6)
		{
			ctx += 1;
		}
		else
		{
			if (v <= 8)
				ctx += 2;
			else if (v == 9)
				ctx += 3;
			else if (v == 10)
				ctx += 4;
			else
				ctx += 5;
		}
	}
	else if (lossy_level == 4)
	{
		// ll == 4						// 0, 1, 2, 3, 4-5, 6-8, 9-12, 13-17, 18-24, 25-32
		ctx *= 6;

		if (v < 4)
			;
		else if (v < 5)
		{
			ctx += 1;
		}
		else
		{
			if (v <= 6)
				ctx += 2;
			else if (v == 7)
				ctx += 3;
			else if (v == 8)
				ctx += 4;
			else
				ctx += 5;
		}
	}
	
	return ctx;
}

// ************************************************************************************
rc_context_t PAECompressor::ext_ctx_lossless(rc_context_t ctx, int v)
{
	//	ctx <<= 3;
	ctx *= 6;

	if (v < 8)
		;
	else if (v < 27)
	{
		ctx += 1;
	}
	else
	{
		if (v == 27 || v == 28)
			ctx += 2;
		else if (v == 29)
			ctx += 3;
		else if (v == 30)
			ctx += 4;
		else
			ctx += 5;
	}

	return ctx;
}

// ************************************************************************************
bool PAECompressor::is_horizontal_better(const matrix_t& matrix, int row, int col, int* p_x, int* p_r1)
{
	if (row > 1 && col > 1)
	{
		//		double horz_frac = horz_stat[row] / (double)col;
		//		double vert_frac = vert_stat[col] / (double)row;

		//		return horz_frac >= vert_frac;

		int64_t hf = (int64_t)horz_stat[row] * row * row;
		int64_t vf = (int64_t)vert_stat[col] * col * col;

		return hf >= vf;
	}

	int x_l1 = *(p_x - 1);
	int x_u1 = *p_r1;

	int x_l1u1 = *(p_r1 - 1);

	int dh = abs(x_u1 - x_l1u1);
	int dv = abs(x_l1 - x_l1u1);

	return dh < dv;
}

// ************************************************************************************
int PAECompressor::pred(const matrix_t& matrix, int row, int col, bool use_horiz, int* p_x, int* p_r1)
{
	if (col == row)
		return 0;

	int up1 = *p_r1;

	if (col == 0)
		return up1;

	int left1 = *(p_x - 1);

	if (row == 0)
		return left1;

	if (abs(row - col) == 1)
		return 1;

	if (left1 - up1 > 4)
	{
		if (use_horiz)
			return left1 - 1;
		else
			return up1 + 1;
	}
	else if (up1 - left1 > 4)
	{
		if (use_horiz)
			return left1 + 1;
		else
			return up1 - 1;
	}

	if (use_horiz)
		return left1;
	else
		return up1;
}

// ************************************************************************************
void PAECompressor::lossy_encoding(matrix_t& matrix, uint32_t lossy_level)
{
	int max_val = matrix.max_value();

	if (max_val < 32)
		max_val = 32;

	vector<int> recoding(max_val + 1);
	iota(recoding.begin(), recoding.end(), 0);

	if (lossy_level == 1)						// 0, 1, ..., 19, 20-21, 22-23, 24-27, 28-32
	{
		for (int i = 28; i < max_val; ++i)
			recoding[i] = 23;
		for (int i = 24; i < 28; ++i)
			recoding[i] = 22;
		for (int i = 22; i < 24; ++i)
			recoding[i] = 21;
		for (int i = 20; i < 22; ++i)
			recoding[i] = 20;
	}
	else if (lossy_level == 2)					// 0, 1, ..., 9, 10-12, 13-15, 16-19, 20-23, 24-27, 28-32
	{
		for (int i = 28; i < max_val; ++i)
			recoding[i] = 15;
		for (int i = 24; i < 28; ++i)
			recoding[i] = 14;
		for (int i = 20; i < 24; ++i)
			recoding[i] = 13;
		for (int i = 16; i < 20; ++i)
			recoding[i] = 12;
		for (int i = 13; i < 16; ++i)
			recoding[i] = 11;
		for (int i = 10; i < 13; ++i)
			recoding[i] = 10;
	}
	else if (lossy_level == 3)					// 0, 1, ..., 5, 6-7, 8-10, 11-14, 15-19, 20-25, 26-32
	{
		for (int i = 26; i < max_val; ++i)
			recoding[i] = 11;
		for (int i = 20; i < 26; ++i)
			recoding[i] = 10;
		for (int i = 15; i < 20; ++i)
			recoding[i] = 9;
		for (int i = 11; i < 15; ++i)
			recoding[i] = 8;
		for (int i = 8; i < 11; ++i)
			recoding[i] = 7;
		for (int i = 6; i < 8; ++i)
			recoding[i] = 6;
	}
	else if (lossy_level == 4)					// 0, 1, 2, 3, 4-5, 6-8, 9-12, 13-17, 18-24, 25-32
	{
		for (int i = 25; i < max_val; ++i)
			recoding[i] = 9;
		for (int i = 18; i < 25; ++i)
			recoding[i] = 8;
		for (int i = 13; i < 18; ++i)
			recoding[i] = 7;
		for (int i = 9; i < 13; ++i)
			recoding[i] = 6;
		for (int i = 6; i < 9; ++i)
			recoding[i] = 5;
		for (int i = 4; i < 6; ++i)
			recoding[i] = 4;
	}

	auto* p = matrix.raw_ptr();
	auto size = matrix.raw_size();

	for (size_t i = 0; i < size; ++i, ++p)
		*p = recoding[*p];
}

// ************************************************************************************
bool PAECompressor::compress_impl(matrix_t& matrix, vector<uint8_t>& packed_data, uint32_t lossy_level)
{
	packed_data.clear();

	vector_io_stream vios(packed_data);
	rc_encoder<vector_io_stream> re(vios);

	rc_context_vec_emb<rc_flag_t> ctx_dict_flag;
	rc_context_vec_emb<rc_large_t> ctx_dict_large;

	rc_flag_t tpl_flag(&re, nullptr, true);

	clear_model();

	int size = (int)matrix.get_n_rows();

	vert_stat.resize(size, 0);
	horz_stat.resize(size, 0);

	re.start();

	if (lossy_level != 0)
		lossy_encoding(matrix, lossy_level);

	int max_val = matrix.max_value();

	rc_large_t tpl_large(&re, max_val + 1, nullptr, true);
	//	rc_large_t tpl_large(&re, nullptr, true);

	for (int row = 0; row < size; ++row)
	{
		int pred_to_enc = 0;

		int* p_x = matrix.ptr(row);
		int* p_r1 = matrix.ptr(row - 1);
		int* p_r2 = matrix.ptr(row - 2);
		int* p_r3 = matrix.ptr(row - 3);

		auto p_vert = vert_stat.begin();
		auto p_horz = horz_stat.begin() + row;

		for (int col = 0; col < size; ++col)
		{
			bool use_horiz = is_horizontal_better(matrix, row, col, p_x, p_r1);
			int pred_val = pred(matrix, row, col, use_horiz, p_x, p_r1);

			int to_enc = *p_x - pred_val;
			rc_context_t ctx = det_ctx(matrix, row, col, use_horiz, p_x, p_r1, p_r2, p_r3);

			if (lossy_level != 0)
				ctx = ext_ctx_lossy(ctx, use_horiz ? *(p_x - 1) : *p_r1, lossy_level);
			else
				ctx = ext_ctx_lossless(ctx, use_horiz ? *(p_x - 1) : *p_r1);

			//			ctx <<= 2;
			ctx *= 3;
			ctx += (rc_context_t)pred_to_enc;

			int val;
			if (to_enc == 0)
				val = 0;
			else if (to_enc == 1)
				val = 1;
			else if (to_enc == -1)
				val = 3;
			else if (to_enc == 2)
				val = 2;
			else if (to_enc == -2)
				val = 4;
			else if (to_enc > 0)
				val = 5;
			else
				val = 6;

			if (to_enc > 0)
				pred_to_enc = 1;
			else if (to_enc < 0)
				pred_to_enc = 2;
			else
				pred_to_enc = 0;

			// Update vert/horz stats
			int x_l1 = *(p_x - 1);
			int x_u1 = *p_r1;
			int x = *p_x;

			int dh = abs(x_l1 - x);
			int dv = abs(x_u1 - x);

			if (dh < dv)
				*p_horz += col;
			else if (dh > dv)
				*p_vert += row;

			//			auto p_flag = rc_find_context(ctx_dict_flag, ctx, &tpl_flag);
			auto p_flag = rc_find_context(ctx_dict_flag, ctx, tpl_flag);

			if (pred_val + 1 > max_val)
				p_flag->encode_excluding(val, 1, 2, 5);
			else if (pred_val + 2 > max_val)
				p_flag->encode_excluding(val, 2, 5);
			else if (pred_val + 3 > max_val)
				p_flag->encode_excluding(val, 5);
			else if (pred_val == 0)
				p_flag->encode_excluding(val, 3, 4, 6);
			else if (pred_val == 1)
				p_flag->encode_excluding(val, 4, 6);
			else if (pred_val == 2)
				p_flag->encode_excluding(val, 6);
			else
				p_flag->encode(val);

			if (val > 4)
			{
				ctx = pred_val;

				if (to_enc < 0)
				{
					ctx += 1ull << 6;
					to_enc = -to_enc;
				}

				//				auto p_large = rc_find_context(ctx_dict_large, ctx, &tpl_large);
				auto p_large = rc_find_context(ctx_dict_large, ctx, tpl_large);

				p_large->encode_excluding(to_enc, 0, 1, 2);
				//				p_large->encode(to_enc);
			}

			++p_x;
			++p_r1;
			++p_r2;
			++p_r3;
			++p_vert;
		}
	}

	re.complete();

	packed_data.emplace_back(max_val);

	return true;
}

// ************************************************************************************
void PAECompressor::add_int(vector<uint8_t>& data, uint32_t x, int no_bytes)
{
	for (int i = 0; i < no_bytes; ++i)
	{
		data.emplace_back(x & 0xffu);
		x >>= 8;
	}
}

// ************************************************************************************
bool PAECompressor::compress(matrix_t& matrix, const double max_declared_pae, uint32_t lossy_level, packed_data_t& packed_data)
{
	bool r = compress_impl(matrix, packed_data, lossy_level);

	if (!r)
		return false;

	add_int(packed_data, (uint32_t)matrix.get_n_rows(), 3);
	add_int(packed_data, (uint32_t)(max_declared_pae * 100), 2);
	packed_data.emplace_back((char)lossy_level);

	return true;
}
// ************************************************************************************
void PAECompressor::lossy_decoding(matrix_t& matrix, uint32_t lossy_level)
{
	vector<int> recoding;
	iota(recoding.begin(), recoding.end(), 0);

	if (lossy_level == 1)
	{
		recoding.resize(24);
		recoding[21] = 22;
		recoding[22] = 25;
		recoding[23] = 30;
	}
	else if (lossy_level == 2)
	{
		// 0, 1, ..., 9, 10-12, 13-15, 16-19, 20-23, 24-27, 28-32
		recoding.resize(15);
		recoding[10] = 11;
		recoding[11] = 14;
		recoding[12] = 17;
		recoding[13] = 21;
		recoding[14] = 25;
		recoding[15] = 30;
	}
	else if (lossy_level == 3)
	{
		recoding.resize(12);
		recoding[6] = 6;
		recoding[7] = 9;
		recoding[8] = 12;
		recoding[9] = 17;
		recoding[10] = 22;
		recoding[11] = 29;
	}
	else if (lossy_level == 4)
	{
		recoding.resize(10);
		recoding[4] = 4;
		recoding[5] = 7;
		recoding[6] = 10;
		recoding[7] = 15;
		recoding[8] = 21;
		recoding[9] = 29;
	}

	auto* p = matrix.raw_ptr();
	auto size = matrix.raw_size();

	for (size_t i = 0; i < size; ++i, ++p)
		*p = recoding[*p];
}

// ************************************************************************************
bool PAECompressor::decompress_impl(vector<uint8_t>& packed_data, matrix_t& matrix, uint32_t lossy_level)
{
	vector_io_stream vios(packed_data);
	rc_decoder<vector_io_stream> rd(vios);

	rc_context_vec_emb<rc_flag_t> ctx_dict_flag;
	rc_context_vec_emb<rc_large_t> ctx_dict_large;

	rc_flag_t tpl_flag(&rd, nullptr, false);

	clear_model();

	int size = (int)matrix.get_n_rows();

	vert_stat.resize(size, 0);
	horz_stat.resize(size, 0);

	int max_val = packed_data.back();
	packed_data.pop_back();

	rc_large_t tpl_large(&rd, max_val + 1, nullptr, false);
	//	rc_large_t tpl_large(&rd, nullptr, false);

	rd.start();

	for (int row = 0; row < size; ++row)
	{
		int pred_to_enc = 0;

		int* p_x = matrix.ptr(row);
		int* p_r1 = matrix.ptr(row - 1);
		int* p_r2 = matrix.ptr(row - 2);
		int* p_r3 = matrix.ptr(row - 3);

		auto p_vert = vert_stat.begin();
		auto p_horz = horz_stat.begin() + row;

		for (int col = 0; col < size; ++col)
		{
			bool use_horiz = is_horizontal_better(matrix, row, col, p_x, p_r1);
			int pred_val = pred(matrix, row, col, use_horiz, p_x, p_r1);

			rc_context_t ctx = det_ctx(matrix, row, col, use_horiz, p_x, p_r1, p_r2, p_r3);

			if (lossy_level != 0)
				ctx = ext_ctx_lossy(ctx, use_horiz ? *(p_x - 1) : *p_r1, lossy_level);
			else
				ctx = ext_ctx_lossless(ctx, use_horiz ? *(p_x - 1) : *p_r1);

			//ctx <<= 2;
			ctx *= 3;
			ctx += (rc_context_t)pred_to_enc;

			//			auto p_flag = rc_find_context(ctx_dict_flag, ctx, &tpl_flag);
			auto p_flag = rc_find_context(ctx_dict_flag, ctx, tpl_flag);

			int val;

			if (pred_val + 1 > max_val)
				val = p_flag->decode_excluding(1, 2, 5);
			else if (pred_val + 2 > max_val)
				val = p_flag->decode_excluding(2, 5);
			else if (pred_val + 3 > max_val)
				val = p_flag->decode_excluding(5);
			else if (pred_val == 0)
				val = p_flag->decode_excluding(3, 4, 6);
			else if (pred_val == 1)
				val = p_flag->decode_excluding(4, 6);
			else if (pred_val == 2)
				val = p_flag->decode_excluding(6);
			else
				val = p_flag->decode();

			int dec_val = 0;
			bool is_large_negative = false;

			if (val == 0)
				dec_val = 0;
			else if (val == 1)
				dec_val = 1;
			else if (val == 3)
				dec_val = -1;
			else if (val == 2)
				dec_val = 2;
			else if (val == 4)
				dec_val = -2;
			else if (val == 5)
				;
			else
				is_large_negative = true;

			if (val > 4)
			{
				ctx = pred_val;

				if (is_large_negative)
					ctx += 1ull << 6;

				//				auto p_large = rc_find_context(ctx_dict_large, ctx, &tpl_large);
				auto p_large = rc_find_context(ctx_dict_large, ctx, tpl_large);

				dec_val = p_large->decode_excluding(0, 1, 2);

				if (is_large_negative)
					dec_val = -dec_val;
			}

			*p_x = pred_val + dec_val;

			if (dec_val > 0)
				pred_to_enc = 1;
			else if (dec_val < 0)
				pred_to_enc = 2;
			else
				pred_to_enc = 0;

			// Update vert/horz stats
			int x_l1 = *(p_x - 1);
			int x_u1 = *p_r1;
			int x = *p_x;

			int dh = abs(x_l1 - x);
			int dv = abs(x_u1 - x);

			if (dh < dv)
				*p_horz += col;
			else if (dh > dv)
				*p_vert += row;

			++p_x;
			++p_r1;
			++p_r2;
			++p_r3;
			++p_vert;
		}
	}

	rd.complete();

	if (lossy_level != 0)
		lossy_decoding(matrix, lossy_level);

	//	cerr << "No. contexts: " << ctx_dict_flag.get_size() << "  " << ctx_dict_large.get_size() << endl;

	return true;
}

// ************************************************************************************
uint32_t PAECompressor::get_int(vector<uint8_t>& data, int no_bytes)
{
	uint32_t x = 0;

	for (int i = 0; i < no_bytes; ++i)
	{
		x <<= 8;
		x += data.back();
		data.pop_back();
	}

	return x;
}

// ************************************************************************************
bool PAECompressor::decompress(packed_data_t& packed_data, matrix_t& matrix, double& max_declared_pae, uint32_t& lossy_level)
{
	if (packed_data.size() < 6)
		return false;

	lossy_level = packed_data.back();
	packed_data.pop_back();
	max_declared_pae = get_int(packed_data, 2) / 100.0;
	uint32_t size = get_int(packed_data, 3);

	matrix.clear();
	matrix.resize(size, size);
	matrix.set_to_zero();

	bool r = decompress_impl(packed_data, matrix, lossy_level);

	return r;
}

// EOF
