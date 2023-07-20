#pragma once

#include <cinttypes>
#include <vector>
#include <string>

#include <zstd.h>

#include "../libs/refresh/range_coder/rc.h"

using namespace std;
using namespace refresh;

// *****************************************************************
class SerializerStatic
{
	static constexpr uint8_t uint32_mask_1 = 0b10000000u;
	static constexpr uint8_t uint32_mask_2 = 0b11000000u;
	static constexpr uint8_t uint32_mask_3 = 0b11100000u;
	static constexpr uint8_t uint32_mask_4 = 0b11110000u;
	static constexpr uint8_t uint32_mask_5 = 0b11110000u;

	static constexpr uint8_t uint32_prefix_1 = 0b00000000u;
	static constexpr uint8_t uint32_prefix_2 = 0b10000000u;
	static constexpr uint8_t uint32_prefix_3 = 0b11000000u;
	static constexpr uint8_t uint32_prefix_4 = 0b11100000u;
	static constexpr uint8_t uint32_prefix_5 = 0b11110000u;

	static constexpr uint32_t uint32_size_1 = 128u;
	static constexpr uint32_t uint32_size_2 = uint32_size_1 + (64u << 8);
	static constexpr uint32_t uint32_size_3 = uint32_size_2 + (32u << 16);
	static constexpr uint32_t uint32_size_4 = uint32_size_3 + (16u << 24);

	static constexpr uint64_t uint64_size_1 = 128ull;
	static constexpr uint64_t uint64_size_2 = uint64_size_1 + (64ull << 8);
	static constexpr uint64_t uint64_size_3 = uint64_size_2 + (32ull << 16);
	static constexpr uint64_t uint64_size_4 = uint64_size_3 + (16ull << 24);

	public:
	static uint8_t low_byte(uint32_t x)
	{
		return (uint8_t)(x & 0xffu);
	}

	static uint8_t low_byte(uint64_t x)
	{
		return (uint8_t)(x & 0xffull);
	}

	static void append32u(uint32_t x, vector<uint8_t> &data)
	{
		if (x < uint32_size_1)
			data.push_back((uint8_t) x);
		else if (x < uint32_size_2)
		{
			x -= uint32_size_1;
			data.push_back(uint32_prefix_2 + low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
		else if (x < uint32_size_3)
		{
			x -= uint32_size_2;
			data.push_back(uint32_prefix_3 + low_byte(x >> 16));
			data.push_back(low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
		else if (x < uint32_size_4)
		{
			x -= uint32_size_3;
			data.push_back(uint32_prefix_4 + low_byte(x >> 24));
			data.push_back(low_byte(x >> 16));
			data.push_back(low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
		else
		{
			x -= uint32_size_4;
			data.push_back(uint32_prefix_5);
			data.push_back(low_byte(x >> 24));
			data.push_back(low_byte(x >> 16));
			data.push_back(low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
	}

	static void append64u(uint64_t x, vector<uint8_t>& data)
	{
		if (x < uint64_size_1)
			data.push_back((uint8_t) x);
		else if (x < uint64_size_2)
		{
			x -= uint64_size_1;
			data.push_back(uint32_prefix_2 + low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
		else if (x < uint64_size_3)
		{
			x -= uint64_size_2;
			data.push_back(uint32_prefix_3 + low_byte(x >> 16));
			data.push_back(low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
		else if (x < uint64_size_4)
		{
			x -= uint32_size_3;
			data.push_back(uint32_prefix_4 + low_byte(x >> 24));
			data.push_back(low_byte(x >> 16));
			data.push_back(low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
		else
		{
			x -= uint64_size_4;
			data.push_back(uint32_prefix_5);
			data.push_back(low_byte(x >> 56));
			data.push_back(low_byte(x >> 48));
			data.push_back(low_byte(x >> 40));
			data.push_back(low_byte(x >> 32));
			data.push_back(low_byte(x >> 24));
			data.push_back(low_byte(x >> 16));
			data.push_back(low_byte(x >> 8));
			data.push_back(low_byte(x));
		}
	}

	static uint32_t load32u(vector<uint8_t>::iterator &iter)
	{
		uint32_t r = 0;

		if ((*iter & uint32_mask_1) == uint32_prefix_1)
			r = *iter++;
		else if ((*iter & uint32_mask_2) == uint32_prefix_2)
		{
			r = *iter++ & ~uint32_mask_2;			r <<= 8;
			r += *iter++;
			r += uint32_size_1;
		}
		else if ((*iter & uint32_mask_3) == uint32_prefix_3)
		{
			r = *iter++ & ~uint32_mask_3;			r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;
			r += uint32_size_2;
		}
		else if ((*iter & uint32_mask_4) == uint32_prefix_4)
		{
			r = *iter++ & ~uint32_mask_4;			r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;
			r += uint32_size_3;
		}
		else
		{
			++iter;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;
			r += uint32_size_4;
		}

		return r;
	}

	static uint64_t load64u(vector<uint8_t>::iterator &iter)
	{
		uint64_t r = 0;

		if ((*iter & uint32_mask_1) == uint32_prefix_1)
			r = *iter++;
		else if ((*iter & uint32_mask_2) == uint32_prefix_2)
		{
			r = *iter++ & ~uint32_mask_2;			r <<= 8;
			r += *iter++;
			r += uint64_size_1;
		}
		else if ((*iter & uint32_mask_3) == uint32_prefix_3)
		{
			r = *iter++ & ~uint32_mask_3;			r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;
			r += uint64_size_2;
		}
		else if ((*iter & uint32_mask_4) == uint32_prefix_4)
		{
			r = *iter++ & ~uint32_mask_4;			r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;
			r += uint64_size_3;
		}
		else
		{
			++iter;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;							r <<= 8;
			r += *iter++;
			r += uint64_size_4;
		}

		return r;
	}
};

// *****************************************************************
class Serializer
{
	const uint8_t uint32_mask_1 = 0b10000000u;
	const uint8_t uint32_mask_2 = 0b11000000u;
	const uint8_t uint32_mask_3 = 0b11100000u;
	const uint8_t uint32_mask_4 = 0b11110000u;
	const uint8_t uint32_mask_5 = 0b11110000u;

	const uint8_t uint32_prefix_1 = 0b00000000u;
	const uint8_t uint32_prefix_2 = 0b10000000u;
	const uint8_t uint32_prefix_3 = 0b11000000u;
	const uint8_t uint32_prefix_4 = 0b11100000u;
	const uint8_t uint32_prefix_5 = 0b11110000u;

	const uint32_t uint32_size_1 = 128u;
	const uint32_t uint32_size_2 = uint32_size_1 + (64u << 8);
	const uint32_t uint32_size_3 = uint32_size_2 + (32u << 16);
	const uint32_t uint32_size_4 = uint32_size_3 + (16u << 24);

	const int local_zstd_compression_level = 19;

	using model_no_reps_t = rc_simple_fixed<vector_io_stream, 5, 1 << 12, 2>;	// 0, same_as_before, 1B, 2B, 3B
	rc_context_vec<model_no_reps_t> dict_no_reps;

	vector<uint8_t> data;
	vector<uint8_t>::iterator iter;

	vector<uint32_t> reps;

	uint8_t no_bytes(uint64_t x);

	uint8_t low_byte(uint32_t x)
	{
		return (uint8_t)  (x & 0xffu);
	}

	void store_uint64(uint8_t* p, uint64_t x, int len);
	uint64_t read_uint64(uint8_t* p, int len);

public: 
	Serializer() = default;
	~Serializer() = default;

	void clear();
	void restart();

	bool eof();

	size_t size();

	vector<uint8_t>& get_data();
	void set_data(const vector<uint8_t> &_data);
	void set_data(vector<uint8_t> &&_data);
	
	void append64u(uint64_t x);
	void append64i(int64_t x);
	void append32u(uint32_t x);
	void append32i(int32_t x);
	void append8u(uint8_t x);
	void append8i(int8_t x);

	void append_str(const string& str);
	void append_cstr(const char* str);

	void append_vec_str(const vector<string>& vs);

	void append_bytes(const char* p, const size_t len);

	void append_serializer(const Serializer& ser);

	uint64_t load64u();
	int64_t load64i();
	uint32_t load32u();
	int32_t load32i();
	uint8_t load8u();
	int8_t load8i();

	string load_str();
	char* load_cstr();

	void load_vec_str(vector<string>& vs, size_t n_items);

	void load_bytes(char* p, const size_t len);

	void load_serializer(Serializer& ser);

	// Compression methods
	void compress_zstd(ZSTD_CCtx *zstd_cctx = nullptr, vector<char>* zstd_dict = nullptr);
	void compress_zstd_rc(ZSTD_CCtx* zstd_cctx, vector<char>* zstd_dict, rc_encoder<vector_io_stream>* rce);
	void decompress_zstd(ZSTD_DCtx* zstd_dctx = nullptr, vector<char>* zstd_dict = nullptr);
	void decompress_zstd_rc(ZSTD_DCtx* zstd_dctx, vector<char>* zstd_dict, rc_decoder<vector_io_stream>* rcd);
};

// EOF
