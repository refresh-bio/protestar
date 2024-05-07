#ifndef _RC_CONTEXT_H
#define _RC_CONTEXT_H

#ifdef _WIN32
#include <mmintrin.h>
#include <xmmintrin.h>
#endif

#include <cstdint>
#include <iostream> 
#include <cstddef>
#include <array>

#include "rc_coders.h"

//#define FIND_EXT_SUPPORT

namespace refresh 
{
// *******************************************************************************************
//
// *******************************************************************************************
template<typename MODEL> class rc_context_hm {
public:
	typedef struct {
		rc_context_t ctx;
		MODEL* rcm;
#ifdef FIND_EXT_SUPPORT
		size_t counter;
#endif
	} item_t;

	typedef rc_context_t key_type;
	typedef MODEL* value_type;
	typedef MODEL model_type;
	typedef size_t aux_type;

private:
	double max_fill_factor;

	size_t size;
	item_t* data;
	size_t allocated;
	size_t size_when_restruct;
	size_t allocated_mask;

	size_t ht_memory;
	size_t ht_total;
	size_t ht_match;

	void restruct(void)
	{
		item_t* old_data = data;
		size_t old_allocated = allocated;

		allocated *= 2;
		size = 0;

		allocated_mask = allocated - 1ull;
		size_when_restruct = (size_t)(allocated * max_fill_factor);

		data = new item_t[allocated];
		for (size_t i = 0; i < allocated; ++i)
			data[i].rcm = nullptr;

		ht_memory += allocated * sizeof(item_t);

		for (size_t i = 0; i < old_allocated; ++i)
			if (old_data[i].rcm != nullptr)
#ifdef FIND_EXT_SUPPORT
				insert(old_data[i].ctx, old_data[i].rcm, old_data[i].counter);
#else
				insert(old_data[i].ctx, old_data[i].rcm);
#endif

		delete[] old_data;
		ht_memory -= old_allocated * sizeof(item_t);
	}

	// Based on murmur64
	size_t hash(rc_context_t ctx)
	{
		auto h = ctx;

		h ^= h >> 33;
		h *= 0xff51afd7ed558ccdL;
		h ^= h >> 33;
		h *= 0xc4ceb9fe1a85ec53L;
		h ^= h >> 33;

		return h & allocated_mask;
	}

public:
	rc_context_hm()
	{
		ht_memory = 0;
		ht_total = 0;
		ht_match = 0;

		allocated = 1u << 20;
		allocated_mask = allocated - 1;

		size = 0;
		data = new item_t[allocated];
		for (size_t i = 0; i < allocated; ++i)
			data[i].rcm = nullptr;

		max_fill_factor = 0.4;

		ht_memory += allocated * sizeof(item_t);

		size_when_restruct = (size_t)(allocated * max_fill_factor);
	}

	~rc_context_hm()
	{
		if (data == nullptr)
			return;

		for (size_t i = 0; i < allocated; ++i)
			if (data[i].rcm)
				delete data[i].rcm;
		delete[] data;
	}

	size_t get_bytes() const {
		return ht_memory;
	}

#ifdef FIND_EXT_SUPPORT
	void debug_list(std::vector<CContextHM<MODEL>::item_t>& v_ctx)
	{
		v_ctx.clear();

		for (size_t i = 0; i < allocated; ++i)
			if (data[i].rcm)
				v_ctx.push_back(data[i]);

		sort(v_ctx.begin(), v_ctx.end(), [](auto& x, auto& y) {return x.counter > y.counter; });
	}
#endif

#ifdef FIND_EXT_SUPPORT
	bool insert(const rc_context_t ctx, MODEL* rcm, size_t counter = 0)
#else
	bool insert(const rc_context_t ctx, MODEL* rcm)
#endif
	{
		if (size >= size_when_restruct)
			restruct();

		size_t h = hash(ctx);

		if (data[h].rcm != nullptr)
		{
			do
			{
				h = (h + 1) & allocated_mask;
			} while (data[h].rcm != nullptr);
		}

		++size;

		data[h].ctx = ctx;
		data[h].rcm = rcm;
#ifdef FIND_EXT_SUPPORT
		data[h].counter = counter;
#endif

		return true;
	}

	MODEL* find(const rc_context_t ctx)
	{
		size_t h = hash(ctx);

		if (data[h].rcm == nullptr)
			return nullptr;

		if (data[h].ctx == ctx)
			return data[h].rcm;

		h = (h + 1) & allocated_mask;

		while (data[h].rcm != nullptr)
		{
			if (data[h].ctx == ctx)
				return data[h].rcm;
			h = (h + 1) & allocated_mask;
		}

		return nullptr;
	}

#ifdef FIND_EXT_SUPPORT
	MODEL* find_ext(const rc_context_t ctx, size_t*& p_counter)
	{
		size_t h = hash(ctx);

		if (data[h].rcm == nullptr)
			return nullptr;

		if (data[h].ctx == ctx)
		{
			p_counter = &data[h].counter;
			return data[h].rcm;
		}

		h = (h + 1) & allocated_mask;

		while (data[h].rcm != nullptr)
		{
			if (data[h].ctx == ctx)
			{
				p_counter = &data[h].counter;
				return data[h].rcm;
			}
			h = (h + 1) & allocated_mask;
		}

		return nullptr;
	}
#endif

	void prefetch(const rc_context_t ctx)
	{
		size_t h = hash(ctx);

#ifdef _WIN32
		_mm_prefetch((const char*)(data + h), _MM_HINT_T0);
#else
		__builtin_prefetch(data + h);
#endif
	}

	size_t get_size(void) const
	{
		return size;
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename MODEL> class rc_context_vec {
public:
	typedef MODEL* item_t;

	typedef rc_context_t key_type;
	typedef MODEL* value_type;
	typedef size_t aux_type;

private:
	size_t size;
	item_t* data;

	void allocate()
	{
		if (size)
			data = new item_t[size];
		else
			data = nullptr;

		for (uint64_t i = 0; i < size; ++i)
			data[i] = nullptr;
	}

	void deallocate()
	{
		if (data == nullptr)
			return;

		for (size_t i = 0; i < size; ++i)
			if (data[i])
				delete data[i];
		delete[] data;
	}

	void reallocate(uint64_t _size)
	{
		auto old_data = data;
		auto old_size = size;

		size = _size;
		data = new item_t[size];
		for (uint64_t i = 0; i < old_size; ++i)
			data[i] = old_data[i];

		for (uint64_t i = old_size; i < size; ++i)
			data[i] = nullptr;

		delete[] old_data;
	}

public:
	rc_context_vec(uint64_t _size = 0) : size(_size)
	{
		allocate();
	}

	~rc_context_vec()
	{
		deallocate();
	}
	
	void clear()
	{
		deallocate();
		size = 0;
		allocate();
	}


/*	void resize(uint64_t _size)
	{
		deallocate();
		size = _size;
		allocate();
	}*/

	bool insert(const rc_context_t ctx, MODEL* rcm)
	{
		if (ctx >= size)
			reallocate((uint64_t) (ctx * 1.1) + 1);

		data[ctx] = rcm;

		return true;
	}

	MODEL* find(const rc_context_t ctx)
	{
		if (ctx >= size)
			return nullptr;

		return data[ctx];
	}

	void prefetch(const rc_context_t ctx)
	{
#ifdef _WIN32
		_mm_prefetch((const char*)(data + ctx), _MM_HINT_T0);
#else
		__builtin_prefetch(data + ctx);
#endif
	}

	size_t get_capacity(void) const
	{
		return size;
	}

	size_t get_size(void) const
	{
		size_t r = 0;

		for (size_t i = 0; i < size; ++i)
			if (data[i])
				++r;
		
		return r;
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename MODEL, unsigned int SIZE> class rc_context_arr_vec
{
	array<rc_context_vec<MODEL>, SIZE> models;

public:
	bool insert(const uint32_t model_id, const rc_context_t ctx, MODEL* rcm)
	{
		if (model_id >= SIZE)
			return false;

		return models[model_id].insert(ctx, rcm);
	}

	pair<uint32_t, MODEL*> find(const uint32_t model_id, const rc_context_t ctx)
	{
		if (model_id >= SIZE)
			return make_pair(0, nullptr);

		rc_context_t mask = (1ull << model_id) - 1ull;

		for (int i = model_id; i >= 0; --i)
		{
			rc_context_t loc_ctx = ctx & mask;

			auto p = models[i].find(loc_ctx);
			if (p)
				return make_pair(i, p);

			mask >>= 1;
		}

		return make_pair(0, nullptr);
	}

	void prefetch(const uint32_t model_id, const rc_context_t ctx)
	{
		if (model_id < SIZE)
			models[model_id].prefetch(ctx);
	}

	size_t get_size(uint32_t model_id) const
	{
		if (model_id >= SIZE)
			return 0;

		return models[model_id].get_size();
	}

	size_t get_size() const
	{
		size_t r = 0;

		for (const auto& model : models)
			r += model.get_size();

		return r;
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename MODEL> class rc_context_vec_emb {
public:
	typedef MODEL item_t;

	typedef rc_context_t key_type;
	typedef MODEL* value_type;
	typedef size_t aux_type;

private:
	size_t size;
	vector<item_t> data;

	void reallocate(uint64_t _size, MODEL &item)
	{
		size = _size;
		data.resize(size, item);
	}

public:
	rc_context_vec_emb(uint64_t _size = 0, MODEL *item = nullptr) : size(_size)
	{
		if(size)
			reallocate(size, *item);
	}

	~rc_context_vec_emb()
	{
	}

	MODEL* insert(const rc_context_t ctx, MODEL &rcm)
	{
		if (ctx >= size)
			reallocate((uint64_t)(ctx * 1.1) + 1, rcm);

		return data.data() + ctx;
	}

	MODEL* find(const rc_context_t ctx)
	{
		if (ctx >= size)
			return nullptr;

		return data.data() + ctx;
	}

	void prefetch(const rc_context_t ctx)
	{
		#ifdef _WIN32
				_mm_prefetch((const char*)(data.data() + ctx), _MM_HINT_T0);
		#else
				__builtin_prefetch(data.data() + ctx);
		#endif
	}

	size_t get_size(void) const
	{
		return size;
	}

	void clear()
	{
		data.clear();
		size = 0;
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename MODEL, unsigned int SIZE> class rc_context_arr_vec_emb {
	array<rc_context_vec_emb<MODEL>, SIZE> models;

public:
	MODEL* insert(const uint32_t model_id, const rc_context_t ctx, MODEL& rcm)
	{
		if (model_id >= SIZE)
			return nullptr;

		return models[model_id].insert(ctx, rcm);
	}

	pair<uint32_t, MODEL*> find(const uint32_t model_id, const rc_context_t ctx)
	{
		if (model_id >= SIZE)
			return make_pair(0, nullptr);

		rc_context_t mask = (1ull << model_id) - 1ull;

		for (int i = model_id; i >= 0; --i)
		{
			rc_context_t loc_ctx = ctx & mask;

			auto p = models[i].find(loc_ctx);
			if (p)
				return make_pair(i, p);

			mask >>= 1;
		}

		return make_pair(0, nullptr);
	}

	void prefetch(const uint32_t model_id, const rc_context_t ctx)
	{
		if (model_id < SIZE)
			models[model_id].prefetch(ctx);
	}

	size_t get_size(uint32_t model_id) const
	{
		if (model_id >= SIZE)
			return 0;

		return models[model_id].get_size();
	}

	size_t get_size() const
	{
		size_t r = 0;

		for (const auto& model : models)
			r += model.get_size();

		return r;
	}
};

} // namespace refresh

#endif

// EOF

