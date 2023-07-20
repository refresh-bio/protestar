#ifndef _RC_UTILS_H
#define _RC_UTILS_H

#include<vector>
#include <map>
#include "rc_context.h"

using namespace std;

namespace refresh
{
#if 0
// *****************************************************************
constexpr uint64_t ilog2(uint64_t x)
{
	uint64_t r = 0;

	for (; x; ++r)
		x >>= 1;

	return r;
}

// *****************************************************************
constexpr uint64_t no_bytes(uint64_t x)
{
	uint64_t r = 1;

	x >>= 8;

	for (; x; ++r)
		x >>= 8;

	return r;
}
#endif

// *******************************************************************************************
// Class for storage of range coder compressed data
class vector_io_stream
{
	vector<uint8_t>& v;
	size_t read_pos;

public:
	vector_io_stream(vector<uint8_t>& _v) : v(_v), read_pos(0)
	{}

	void restart_read()
	{
		read_pos = 0;
	}

	bool eof() const
	{
		return read_pos >= v.size();
	}

	uint8_t get_byte()
	{
		return v[read_pos++];
	}

	void put_byte(uint8_t x)
	{
		v.push_back(x);
	}

	template<typename T>
	void put_byte(T x)
	{
		v.push_back(static_cast<uint8_t>(x));
	}

	void clear()
	{
		v.clear();
	}

	size_t size()
	{
		return v.size();
	}
};

// *******************************************************************************************
template<typename T, typename V>
V* rc_find_context(T& ctx_dict, rc_context_t ctx, V* tpl)
{
	auto p = ctx_dict.find(ctx);

	if (p == nullptr)
		ctx_dict.insert(ctx, p = new V(*tpl));

	return p;
}

// *******************************************************************************************
template<typename T, typename V>
V* rc_find_context(T& ctx_dict, rc_context_t ctx, V& tpl)
{
	auto p = ctx_dict.find(ctx);

	if (p == nullptr)
		p = ctx_dict.insert(ctx, tpl);

	return p;
}

// *******************************************************************************************
template<typename T, typename V>
V* rc_find_context(T& ctx_dict, uint32_t ctx_len, size_t counter_limit, rc_context_t ctx, V* tpl)
{
	V* p;
	uint32_t len;

	tie(len, p) = ctx_dict.find(ctx_len, ctx);

	if (p == nullptr)
		ctx_dict.insert(0, 0, p = new V(*tpl));
	else
	{
		auto counter = p->get_counter();

		if (counter >= counter_limit && len < ctx_len)
		{
			ctx_dict.insert(len + 1,
				ctx & ((1ull << (len + 1)) - 1ull),
				p = new V(*p));
			p->set_counter();
		}
	}

	return p;
}

// *******************************************************************************************
template<typename T, typename V>
V* rc_find_context(T& ctx_dict, uint32_t ctx_len, size_t counter_limit, rc_context_t ctx, V& tpl)
{
	V* p;
	uint32_t len;

	tie(len, p) = ctx_dict.find(ctx_len, ctx);

	if (p == nullptr)
		p = ctx_dict.insert(0, 0, tpl);
	else
	{
		auto counter = p->get_counter();

		if (counter >= counter_limit && len < ctx_len)
		{
			p = ctx_dict.insert(len + 1,
				ctx & ((1ull << (len + 1)) - 1ull),
				tpl);
			p->set_counter();
		}

		//		p->force_rescale();
	}

	return p;
}

// *******************************************************************************************
// Private, non-documented yet
class CBasicCoder
{
protected:
	rc_encoder<vector_io_stream>* rce;
	rc_decoder<vector_io_stream>* rcd;

	vector_io_stream* v_vios_io;
	vector<uint8_t> v_io;

	template<typename T, typename V>
	V* find_rc_context(T& m_ctx_rc, rc_context_t ctx, V* tpl)
	{
		auto p = m_ctx_rc.find(ctx);

		if (p == nullptr)
			m_ctx_rc.insert(ctx, p = new V(*tpl));

		return p;
	}

	template<typename T, typename V>
	V* find_rce_context(T& m_ctx_rc, rc_context_t ctx, V* tpl)
	{
		auto p = m_ctx_rc.find(ctx);

		if (p == nullptr)
			p = m_ctx_rc.insert(ctx, *tpl);

		return p;
	}

	bool is_compressing;

public:
	CBasicCoder()
	{
		rce = nullptr;
		rcd = nullptr;

		v_vios_io = nullptr;
	}

	virtual ~CBasicCoder()
	{
		delete rce;
		delete rcd;

		delete v_vios_io;
	}

	// Reading compression output (after Finish)
	void GetOutput(vector<uint8_t>& _v_io)
	{
		_v_io = move(v_io);
	}

	// Setting compressed input (before Init)
	void SetInput(vector<uint8_t>& _v_io)
	{
		v_io = move(_v_io);
	}
};

}

// EOF


#endif
