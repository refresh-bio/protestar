#include "serializer.h"
#include <algorithm>
#include <cassert>

#include "../common/aa_atoms.h"

// *****************************************************************
void Serializer::store_uint64(uint8_t* p, uint64_t x, int len)
{
	for (int i = 0; i < len; ++i)
	{
		p[i] = x & 0xffu;
		x >>= 8;
	}
}

// *****************************************************************
uint64_t Serializer::read_uint64(uint8_t* p, int len)
{
	uint64_t x = 0;
	int shift = 0;

	for (int i = 0; i < len; ++i, shift += 8)
		x += ((uint64_t) p[i]) << shift;

	return x;
}

// *****************************************************************
uint8_t Serializer::no_bytes(uint64_t x)
{
	uint8_t r = 1;

	for (; x > 255ull; x >>= 8)
		++r;

	return r;
}

// *****************************************************************
size_t Serializer::size()
{
	return data.size();
}

// *****************************************************************
void Serializer::clear()
{
	data.clear();
	data.shrink_to_fit();
	iter = data.begin();
}

// *****************************************************************
void Serializer::restart()
{
	iter = data.begin();
}

// *****************************************************************
bool Serializer::eof()
{
	return iter == data.end();
}

// *****************************************************************
vector<uint8_t>& Serializer::get_data()
{
	return data;
}

// *****************************************************************
void Serializer::set_data(const vector<uint8_t>& _data)
{
	data = _data;

	restart();
}

// *****************************************************************
void Serializer::set_data(vector<uint8_t>&& _data)
{
	data = move(_data);

	restart();
}

// *****************************************************************
void Serializer::append64u(uint64_t x)
{
	auto nb = no_bytes(x);

	data.push_back(nb);

	for (int i = 0; i < nb; ++i)
	{
		data.push_back(x & 0xffull);
		x >>= 8;
	}
}

// *****************************************************************
void Serializer::append64i(int64_t x)
{
	uint64_t y = 0;

	if (x < 0)
	{
		y = (uint64_t) -(x + 1);
		y *= 2;
	}
	else
	{
		y = (uint64_t) x;
		y *= 2;
		++y;
	}

	append64u(y);
}

// *****************************************************************
void Serializer::append32u(uint32_t x)
{
	if (x < uint32_size_1)
		data.push_back(x);
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

// *****************************************************************
void Serializer::append32i(int32_t x)
{
	uint32_t y = 0;

	if (x < 0)
	{
		y = (uint32_t)-(x + 1);
		y *= 2;
	}
	else
	{
		y = (uint32_t)x;
		y *= 2;
		++y;
	}

	append32u(y);
}

// *****************************************************************
void Serializer::append8u(uint8_t x)
{
	data.push_back(x);
}

// *****************************************************************
void Serializer::append8i(int8_t x)
{
	data.push_back((uint8_t)x);
}

// *****************************************************************
void Serializer::append_str(const string& str)
{
	append_cstr(str.c_str());
}

// *****************************************************************
void Serializer::append_cstr(const char* str)
{
	for (; *str; ++str)
		data.push_back((uint8_t)*str);

	data.push_back(0);
}

// *****************************************************************
uint64_t Serializer::load64u()
{
	uint64_t r = 0;
	uint8_t nb = *iter++;

	int shift = 0;

	for (int i = 0; i < nb; ++i, shift += 8)
		r += ((uint64_t)*iter++) << shift;

	return r;
}

// *****************************************************************
int64_t Serializer::load64i()
{
	int64_t r = 0;
	uint64_t y = load64u();

	if (y & 1ull)	
	{
		y >>= 1;
		r = (int64_t)y;
	}
	else
	{
		y >>= 1;
		r = -(int64_t)y;
		--r;
	}

	return r;
}

// *****************************************************************
uint32_t Serializer::load32u()
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

// *****************************************************************
int32_t Serializer::load32i()
{
	int32_t r = 0;
	uint32_t y = load32u();

	if (y & 1ull)
	{
		y >>= 1;
		r = (int32_t)y;
	}
	else
	{
		y >>= 1;
		r = -(int32_t)y;
		--r;
	}

	return r;
}

// *****************************************************************
uint8_t Serializer::load8u()
{
	return *iter++;
}

// *****************************************************************
int8_t Serializer::load8i()
{
	return (int8_t) *iter++;
}

// *****************************************************************
string Serializer::load_str()
{
	auto p = iter;

	while (*iter)
		++iter;

	++iter;

	return string(p, iter-1);
}

// *****************************************************************
char* Serializer::load_cstr()
{
	char* r = (char*) (data.data() + (iter - data.begin()));

	char* q = r;
	
	while (*q)
		++q;

	iter += (q - r) + 1;

	return r;
}

// *****************************************************************
void Serializer::append_vec_str(const vector<string>& vs)
{
	for (const auto& x : vs)
		append_str(x);
}

// *****************************************************************
void Serializer::load_vec_str(vector<string>& vs, size_t n_items)
{
	vs.resize(n_items);

	for (size_t i = 0; i < n_items; ++i)
		vs[i] = load_str();
}

// *****************************************************************
void Serializer::append_bytes(const char* p, const size_t len)
{
	data.insert(data.end(), p, p + len);
}

// *****************************************************************
void Serializer::load_bytes(char* p, const size_t len)
{
	copy_n(iter, len, p);
	iter += len;
}

// *****************************************************************
void Serializer::append_serializer(const Serializer& ser)
{
	append64u(ser.data.size());
	append_bytes((char*)ser.data.data(), ser.data.size());
}

// *****************************************************************
void Serializer::load_serializer(Serializer& ser)
{
	uint64_t ser_size = load64u();

	vector<uint8_t> ser_data(ser_size);

	load_bytes((char*)ser_data.data(), ser_size);

	ser.set_data(move(ser_data));	
}

// *****************************************************************
//void Serializer::compress_zstd(ZSTD_CCtx* zstd_cctx, ZSTD_CDict* zstd_cdict)
void Serializer::compress_zstd(ZSTD_CCtx* zstd_cctx, vector<char>* zstd_dict)
{
	uint64_t max_size = ZSTD_compressBound(data.size());

	vector<uint8_t> tmp(max_size + 8);

	size_t compressed_size;

	if (zstd_cctx && zstd_dict)
// For unknown reason using ZSTD_compress_usingCDict leads to worse ratios than using ZSTD_compress_usingDict
//		compressed_size = ZSTD_compress_usingCDict(zstd_cctx, tmp.data() + 8, max_size, data.data(), data.size(), zstd_cdict);
		compressed_size = ZSTD_compress_usingDict(zstd_cctx, tmp.data() + 8, max_size, data.data(), data.size(), zstd_dict->data(), zstd_dict->size(), local_zstd_compression_level);
	else if(zstd_cctx)
		compressed_size = ZSTD_compress2(zstd_cctx, tmp.data() + 8, max_size, data.data(), data.size());
	else
		compressed_size = ZSTD_compress(tmp.data() + 8, max_size, data.data(), data.size(), local_zstd_compression_level);
	
	store_uint64(tmp.data(), data.size(), 8);

	tmp.resize(compressed_size + 8);

	swap(tmp, data);
}

// *****************************************************************
//void Serializer::decompress_zstd(ZSTD_DCtx* zstd_dctx, ZSTD_DDict* zstd_ddict)
void Serializer::decompress_zstd(ZSTD_DCtx* zstd_dctx, vector<char> *zstd_dict)
{
	uint64_t raw_size = read_uint64(data.data(), 8);

	vector<uint8_t> tmp(raw_size);

	if(zstd_dctx && zstd_dict)
		ZSTD_decompress_usingDict(zstd_dctx, tmp.data(), tmp.size(), data.data() + 8, data.size() - 8, zstd_dict->data(), zstd_dict->size());
	else if(zstd_dctx)
		ZSTD_decompressDCtx(zstd_dctx, tmp.data(), tmp.size(), data.data() + 8, data.size() - 8);
	else
		ZSTD_decompress(tmp.data(), tmp.size(), data.data() + 8, data.size() - 8);

	swap(tmp, data);

	restart();
}

// *****************************************************************
void Serializer::compress_zstd_rc(ZSTD_CCtx* zstd_cctx, vector<char>* zstd_dict, rc_encoder<vector_io_stream>* rce)
{
	compress_zstd(zstd_cctx, zstd_dict);

	model_no_reps_t tpl_no_reps(rce, nullptr, true);

	uint32_t prev = 0;
	uint64_t ctx = 0;

	for (size_t i = 0; i < reps.size(); ++i)
	{
		auto p = rc_find_context(dict_no_reps, ctx, &tpl_no_reps);
		
		ctx <<= 3;

		if (reps[i] == 0)
		{
			p->encode(0);				// 0
			ctx += 0;
		}
		else if (reps[i] == prev)
		{
			p->encode(1);				// same as prev
			ctx += 1;
		}
		else 
		{
			int nb = no_bytes(reps[i]);

			p->encode(1 + nb);
			ctx += 1 + nb;

			if (nb == 1)
			{
				rce->encode_frequency(1, reps[i], 256);
			}
			else if (nb == 2)
			{
				rce->encode_frequency(1, reps[i] >> 8, 256);
				rce->encode_frequency(1, reps[i] & 0xffu, 256);
			}
			else
			{
				rce->encode_frequency(1, reps[i] >> 16, 256);
				rce->encode_frequency(1, (reps[i] >> 8) & 0xffu, 256);
				rce->encode_frequency(1, reps[i] & 0xffu, 256);
			}
		}

		ctx &= 0x7u;
		prev = reps[i];
	}
}

// *****************************************************************
void Serializer::decompress_zstd_rc(ZSTD_DCtx* zstd_dctx, vector<char>* zstd_dict, rc_decoder<vector_io_stream>* rcd)
{
	decompress_zstd(zstd_dctx, zstd_dict);

	model_no_reps_t tpl_no_reps(rcd, nullptr, false);

	reps.clear();

	uint32_t prev = 0;
	uint64_t ctx = 0;

	size_t no_reps = count(data.begin(), data.end(), 0);

	for (size_t i = 0; i < no_reps; ++i)
	{
		auto p = rc_find_context(dict_no_reps, ctx, &tpl_no_reps);

		ctx <<= 3;

		int flag = p->decode();

		if (flag == 0)
			reps.emplace_back(0);			
		else if (1)
			reps.emplace_back(prev);
		else
		{
			int nb = flag - 1;

			uint32_t val = 0;

			if (nb == 1)
			{
				val = rcd->get_cumulative_freq(256);
				rcd->update_frequency(1, val, 256);
			}
			else if (nb == 2)
			{
				val = rcd->get_cumulative_freq(256);
				rcd->update_frequency(1, val, 256);
				val <<= 8;
				val += rcd->get_cumulative_freq(256);
				rcd->update_frequency(1, val, 256);
			}
			else
			{
				val = rcd->get_cumulative_freq(256);
				rcd->update_frequency(1, val, 256);
				val <<= 8;
				val += rcd->get_cumulative_freq(256);
				rcd->update_frequency(1, val, 256);
				val <<= 8;
				val += rcd->get_cumulative_freq(256);
				rcd->update_frequency(1, val, 256);
			}
		}

		ctx += flag;
		ctx &= 0x7u;
		prev = reps.back();
	}
}

// EOF
