#ifndef _RC_MODELS_H
#define _RC_MODELS_H

//#include "defs.h"
#include "rc_meta_switch.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <cstdint>
#include <numeric>

//#define STATS_MODE

using namespace std;

// *******************************************************************************************
//
// *******************************************************************************************
#define REFRESH_RC_CASE(n)		case n: left_freq += stats[n-1]

namespace refresh
{
template<unsigned MAX_TOTAL, unsigned ADDER>
class freq_model_simple
{
	uint32_t n_symbols;
	uint32_t* stats;
	uint32_t total;

	void rescale()
	{
		while (total >= MAX_TOTAL)
		{
			total = 0;
			for (uint32_t i = 0; i < n_symbols; ++i)
			{
				stats[i] = (stats[i] + 1) / 2;
				total += stats[i];
			}
		}
	}

public:
	freq_model_simple() : n_symbols(0), stats(nullptr), total(0)
	{
	};

	~freq_model_simple()
	{
		if (stats)
			delete[] stats;
	};

	freq_model_simple(const freq_model_simple& c)
	{
		n_symbols = c.n_symbols;
		total = c.total;
		stats = new uint32_t[n_symbols];

		for (uint32_t i = 0; i < n_symbols; ++i)
			stats[i] = c.stats[i];
	}

	freq_model_simple& operator=(const freq_model_simple& c)
	{
		if (this != &c)
		{
			if (stats)
				delete[] stats;

			n_symbols = c.n_symbols;
			total = c.total;

			stats = new uint32_t[n_symbols];

			for (uint32_t i = 0; i < n_symbols; ++i)
				stats[i] = c.stats[i];
		}

		return *this;
	}

	void init(uint32_t _n_symbols, uint32_t* _init_stats)
	{
		if (stats)
		{
			if (n_symbols != _n_symbols)
			{
				delete[] stats;
				n_symbols = _n_symbols;
				stats = new uint32_t[n_symbols];
			}
		}
		else
		{
			n_symbols = _n_symbols;
			stats = new uint32_t[n_symbols];
		}

		if (_init_stats)
			for (uint32_t i = 0; i < n_symbols; ++i)
				stats[i] = _init_stats[i];
		else
			std::fill_n(stats, n_symbols, 1);

		total = std::accumulate(stats, stats + n_symbols, 0u);
		rescale();
	}

	void init(const freq_model_simple& c)
	{
		n_symbols = c.n_symbols;

		if (stats)
			delete[] stats;

		stats = new uint32_t[n_symbols];
		std::copy_n(c.stats, n_symbols, stats);
		total = std::accumulate(stats, stats + n_symbols, 0u);
	}

	void get_freq(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf)
	{
		//		left_freq = 0;

		//		if(symbol >= 100)
		left_freq = accumulate(stats, stats + symbol, 0u);
		/*		else
				{
					switch (symbol)
					{
						REFRESH_RC_CASE(99);	REFRESH_RC_CASE(98);	REFRESH_RC_CASE(97);	REFRESH_RC_CASE(96);	REFRESH_RC_CASE(95);
						REFRESH_RC_CASE(94);	REFRESH_RC_CASE(93);	REFRESH_RC_CASE(92);	REFRESH_RC_CASE(91);	REFRESH_RC_CASE(90);
						REFRESH_RC_CASE(89);	REFRESH_RC_CASE(88);	REFRESH_RC_CASE(87);	REFRESH_RC_CASE(86);	REFRESH_RC_CASE(85);	
						REFRESH_RC_CASE(84);	REFRESH_RC_CASE(83);	REFRESH_RC_CASE(82);	REFRESH_RC_CASE(81);	REFRESH_RC_CASE(80);
						REFRESH_RC_CASE(79);	REFRESH_RC_CASE(78);	REFRESH_RC_CASE(77);	REFRESH_RC_CASE(76);	REFRESH_RC_CASE(75);	
						REFRESH_RC_CASE(74);	REFRESH_RC_CASE(73);	REFRESH_RC_CASE(72);	REFRESH_RC_CASE(71);	REFRESH_RC_CASE(70);
						REFRESH_RC_CASE(69);	REFRESH_RC_CASE(68);	REFRESH_RC_CASE(67);	REFRESH_RC_CASE(66);	REFRESH_RC_CASE(65);	
						REFRESH_RC_CASE(64);	REFRESH_RC_CASE(63);	REFRESH_RC_CASE(62);	REFRESH_RC_CASE(61);	REFRESH_RC_CASE(60);
						REFRESH_RC_CASE(59);	REFRESH_RC_CASE(58);	REFRESH_RC_CASE(57);	REFRESH_RC_CASE(56);	REFRESH_RC_CASE(55);	
						REFRESH_RC_CASE(54);	REFRESH_RC_CASE(53);	REFRESH_RC_CASE(52);	REFRESH_RC_CASE(51);	REFRESH_RC_CASE(50);
						REFRESH_RC_CASE(49);	REFRESH_RC_CASE(48);	REFRESH_RC_CASE(47);	REFRESH_RC_CASE(46);	REFRESH_RC_CASE(45);	
						REFRESH_RC_CASE(44);	REFRESH_RC_CASE(43);	REFRESH_RC_CASE(42);	REFRESH_RC_CASE(41);	REFRESH_RC_CASE(40);
						REFRESH_RC_CASE(39);	REFRESH_RC_CASE(38);	REFRESH_RC_CASE(37);	REFRESH_RC_CASE(36);	REFRESH_RC_CASE(35);	
						REFRESH_RC_CASE(34);	REFRESH_RC_CASE(33);	REFRESH_RC_CASE(32);	REFRESH_RC_CASE(31);	REFRESH_RC_CASE(30);
						REFRESH_RC_CASE(29);	REFRESH_RC_CASE(28);	REFRESH_RC_CASE(27);	REFRESH_RC_CASE(26);	REFRESH_RC_CASE(25);	
						REFRESH_RC_CASE(24);	REFRESH_RC_CASE(23);	REFRESH_RC_CASE(22);	REFRESH_RC_CASE(21);	REFRESH_RC_CASE(20);
						REFRESH_RC_CASE(19);	REFRESH_RC_CASE(18);	REFRESH_RC_CASE(17);	REFRESH_RC_CASE(16);	REFRESH_RC_CASE(15);	
						REFRESH_RC_CASE(14);	REFRESH_RC_CASE(13);	REFRESH_RC_CASE(12);	REFRESH_RC_CASE(11);	REFRESH_RC_CASE(10);
						REFRESH_RC_CASE(9);		REFRESH_RC_CASE(8);		REFRESH_RC_CASE(7);		REFRESH_RC_CASE(6);		REFRESH_RC_CASE(5);		
						REFRESH_RC_CASE(4);		REFRESH_RC_CASE(3);		REFRESH_RC_CASE(2);		REFRESH_RC_CASE(1);

					case 0: break;
					default:
						for (int i = 0; i < symbol; ++i)
							left_freq += stats[i];
					}
				}*/

		sym_freq = stats[symbol];
		totf = total;
	}

	void get_freq_exc(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf, const uint32_t exc)
	{
		left_freq = 0;

		for (uint32_t i = 0; i < symbol; ++i)
			if (i != exc)
				left_freq += stats[i];

		sym_freq = stats[symbol];
		totf = total - stats[exc];
	}

	void get_freq_exc(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf, const uint32_t exc1, const uint32_t exc2)
	{
		left_freq = 0;

		for (uint32_t i = 0; i < symbol; ++i)
			if (i != exc1 && i != exc2)
				left_freq += stats[i];

		sym_freq = stats[symbol];
		totf = total - stats[exc1] - stats[exc2];
	}

	void get_freq_exc(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf, const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		left_freq = 0;

		for (uint32_t i = 0; i < symbol; ++i)
			if (i != exc1 && i != exc2 && i != exc3)
				left_freq += stats[i];

		sym_freq = stats[symbol];
		totf = total - stats[exc1] - stats[exc2] - stats[exc3];
	}

	void update(uint32_t symbol)
	{
		stats[symbol] += ADDER;
		total += ADDER;

		if (total >= MAX_TOTAL)
			rescale();
	}

	uint32_t get_sym(uint32_t left_freq)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < n_symbols; ++i)
		{
			t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	uint32_t get_sym_exc(uint32_t left_freq, uint32_t exc)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < n_symbols; ++i)
		{
			if (i != exc)
				t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	// !!! TODO: can be without a loop
	uint32_t get_sym_exc(uint32_t left_freq, uint32_t exc1, uint32_t exc2)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < n_symbols; ++i)
		{
			if (i != exc1 && i != exc2)
				t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	// !!! TODO: can be without a loop
	uint32_t get_sym_exc(uint32_t left_freq, uint32_t exc1, uint32_t exc2, uint32_t exc3)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < n_symbols; ++i)
		{
			if (i != exc1 && i != exc2 && i != exc3)
				t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	uint32_t get_total()
	{
		return total;
	}

	uint32_t get_total_exc(const uint32_t exc)
	{
		return total - stats[exc];
	}

	uint32_t get_total_exc(const uint32_t exc1, const uint32_t exc2)
	{
		return total - stats[exc1] - stats[exc2];
	}

	uint32_t get_total_exc(const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		return total - stats[exc1] - stats[exc2] - stats[exc3];
	}

	uint32_t* get_stats()
	{
		return stats;
	}

	void set_stats(uint32_t* stats_to_set)
	{
		total = 0;
		for (uint32_t i = 0; i < n_symbols; ++i)
			total += stats[i] = stats_to_set[i];
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template <unsigned N_SYMBOLS, unsigned MAX_TOTAL, unsigned ADDER> class freq_model_simple_fixed
{
	uint32_t stats[N_SYMBOLS];
	uint32_t total;
#ifdef STATS_MODE
	size_t no_updates;
#endif

	void rescale()
	{
		while (total >= MAX_TOTAL)
		{
			total = 0;
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			{
				stats[i] = (stats[i] + 1) / 2;
				total += stats[i];
			}
		}
	}

	void get_freq_imp(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf)
	{
		left_freq = 0;

		if constexpr (N_SYMBOLS > 10)
			/*			for (uint32_t i = 0; i < symbol; ++i)
									left_freq += stats[i];*/
			left_freq = accumulate(stats, stats + symbol, 0u);
		else
			rc_switch_impl<N_SYMBOLS>::get_freq(symbol, left_freq, stats);

		sym_freq = stats[symbol];
		totf = total;
	}

public:
#ifdef STATS_MODE
	freq_model_simple_fixed() : total(0), no_updates(0)
#else
	freq_model_simple_fixed() : total(0)
#endif
	{
		for (unsigned i = 0; i < N_SYMBOLS; ++i)
			stats[i] = 0;
	};

	~freq_model_simple_fixed()
	{
	};

	freq_model_simple_fixed(const freq_model_simple_fixed& c)
	{
		for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			stats[i] = c.stats[i];
		total = c.total;
	}

	freq_model_simple_fixed& operator=(const freq_model_simple_fixed& c)
	{
		if (this != &c)
		{
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
				stats[i] = c.stats[i];
			total = c.total;
		}

		return *this;
	}

	void init(const uint32_t* _init_stats)
	{
		if (_init_stats)
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
				stats[i] = _init_stats[i];
		else
			std::fill_n(stats, N_SYMBOLS, 1);

		total = std::accumulate(stats, stats + N_SYMBOLS, 0u);
		rescale();

#ifdef STATS_MODE
		no_updates = 0;
#endif
	}

	void init(const freq_model_simple_fixed& c)
	{
		std::copy_n(c.stats, N_SYMBOLS, stats);
		total = std::accumulate(stats, stats + N_SYMBOLS, 0u);

#ifdef STATS_MODE
		no_updates = 0;
#endif
	}

	void get_freq(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf)
	{
		get_freq_imp(symbol, sym_freq, left_freq, totf);
	}

	void get_freq_exc(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf, const uint32_t exc)
	{
		get_freq_imp(symbol, sym_freq, left_freq, totf);

		totf -= stats[exc];

		if (exc < symbol)
			left_freq -= stats[exc];
	}

	void get_freq_exc(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf, const uint32_t exc1, const uint32_t exc2)
	{
		get_freq_imp(symbol, sym_freq, left_freq, totf);

		totf -= stats[exc1];
		totf -= stats[exc2];

		if (exc1 < symbol)
			left_freq -= stats[exc1];
		if (exc2 < symbol)
			left_freq -= stats[exc2];
	}

	void get_freq_exc(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf, const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		get_freq_imp(symbol, sym_freq, left_freq, totf);

		totf -= stats[exc1];
		totf -= stats[exc2];
		totf -= stats[exc3];

		if (exc1 < symbol)
			left_freq -= stats[exc1];
		if (exc2 < symbol)
			left_freq -= stats[exc2];
		if (exc3 < symbol)
			left_freq -= stats[exc3];
	}

	void update(uint32_t symbol)
	{
		stats[symbol] += ADDER;
		total += ADDER;

		if (total >= MAX_TOTAL)
			rescale();

#ifdef STATS_MODE
		++no_updates;
#endif
	}

	uint32_t get_sym(uint32_t left_freq)
	{
		uint32_t t = 0;

		if constexpr (N_SYMBOLS > 10)
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			{
				t += stats[i];
				if (t > left_freq)
					return i;
			}
		else
		{
			return rc_if_impl<N_SYMBOLS>::get_sym(left_freq, stats);
		}

		return ~0u;
	}

	void get_sym_freq_and_update(uint32_t& left_freq, uint32_t& sym, uint32_t& sym_freq)
	{
		if constexpr (N_SYMBOLS > 10)
		{
			uint32_t t = 0;
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			{
				t += stats[i];
				if (t > left_freq)
				{
					sym = i;
					left_freq = t - stats[i];
					break;
				}
			}
		}
		else
		{
			sym = rc_if_impl<N_SYMBOLS>::get_sym_lf(left_freq, stats);
		}

		sym_freq = stats[sym];
		update(sym);
	}

	uint32_t get_sym_exc(uint32_t left_freq, uint32_t exc)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < N_SYMBOLS; ++i)
		{
			if (i != exc)
				t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	// !!! TODO: can be without a loop
	uint32_t get_sym_exc(uint32_t left_freq, uint32_t exc1, uint32_t exc2)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < N_SYMBOLS; ++i)
		{
			if (i != exc1 && i != exc2)
				t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	// !!! TODO: can be without a loop
	uint32_t get_sym_exc(uint32_t left_freq, uint32_t exc1, uint32_t exc2, uint32_t exc3)
	{
		uint32_t t = 0;

		for (uint32_t i = 0; i < N_SYMBOLS; ++i)
		{
			if (i != exc1 && i != exc2 && i != exc3)
				t += stats[i];
			if (t > left_freq)
				return i;
		}

		return ~0u;
	}

	uint32_t get_total()
	{
		return total;
	}

	uint32_t get_total_exc(const uint32_t exc)
	{
		return total - stats[exc];
	}

	uint32_t get_total_exc(const uint32_t exc1, const uint32_t exc2)
	{
		return total - stats[exc1] - stats[exc2];
	}

	uint32_t get_total_exc(const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		return total - stats[exc1] - stats[exc2] - stats[exc3];
	}

	uint32_t* get_stats()
	{
		return stats;
	}

	void set_stats(uint32_t* stats_to_set)
	{
		total = 0;
		for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			total += stats[i] = stats_to_set[i];
	}

	void force_rescale()
	{
		do
		{
			total = 0;
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			{
				stats[i] = (stats[i] + 1) / 2;
				total += stats[i];
			}
		} while (total >= MAX_TOTAL);
	}

#ifdef STATS_MODE
	void get_log_stats(size_t& _no_updates, std::vector<float>& _v_freq)
	{
		_no_updates = no_updates;

		_v_freq.resize(N_SYMBOLS, 0u);
		auto sum = accumulate(stats, stats + N_SYMBOLS, 0u);

		if (sum)
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
				_v_freq[i] = (float)stats[i] / sum;
	}

	size_t get_no_updates()
	{
		return no_updates;
	}
#endif
};


// *******************************************************************************************
//
// *******************************************************************************************
// Max value of N_SYMBOLS = 256
template <unsigned N_SYMBOLS, unsigned MAX_TOTAL, unsigned ADDER> class freq_model_fenwick_fixed
{
	uint32_t arr[N_SYMBOLS];
	uint32_t total;

	static constexpr uint32_t LSB[] = { 0, 1, 2, 2, 4, 4, 4, 4, 8, 8, 8, 8, 8, 8, 8, 8,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
		64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 256 };

	uint32_t prefix_sum(uint32_t symbol)
	{
		uint32_t sum = 0;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];
		symbol &= symbol - 1;

		if (!symbol)		return sum;
		sum += arr[symbol - 1];

		return sum;
		/*		for (; symbol > 0; symbol &= symbol - 1)
					sum += arr[symbol - 1];*/

		return sum;
	}

	void increment(uint32_t symbol)
	{
		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;
		symbol |= symbol + 1;

		if (symbol >= N_SYMBOLS)	return;
		arr[symbol] += ADDER;

		/*		for (; symbol < N_SYMBOLS; symbol |= symbol + 1)
					arr[symbol] += delta;*/
	}

	// Convert counts in arr into Fenwick tree
	void make_fenwick_tree(void)
	{
		for (uint32_t i = 0; i < N_SYMBOLS; i++)
		{
			uint32_t j = i | (i + 1);

			if (j < N_SYMBOLS)
				arr[j] += arr[i];
		}
	}

	// Convert Fenwick tree in arr into array of counts
	void unmake_fenwick_tree(void)
	{
		for (uint32_t i = N_SYMBOLS; i-- > 0;)
		{
			uint32_t j = i | (i + 1);

			if (j < N_SYMBOLS)
				arr[j] -= arr[i];
		}
	}

	uint32_t value(uint32_t symbol)
	{
		uint32_t sum = arr[symbol];
		uint32_t j = symbol & (symbol + 1);

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];
		symbol &= symbol - 1;

		if (symbol <= j)		return sum;
		sum -= arr[symbol - 1];

		return sum;
	}

	// *******************
	void rescale()
	{
		unmake_fenwick_tree();

		while (total >= MAX_TOTAL)
		{
			total = 0;
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			{
				arr[i] = (arr[i] + 1) / 2;
				total += arr[i];
			}
		}

		make_fenwick_tree();
	}

public:
	freq_model_fenwick_fixed() : total(0)
	{
		static_assert(N_SYMBOLS <= 256);
	};

	~freq_model_fenwick_fixed()
	{
	};

	freq_model_fenwick_fixed(const freq_model_fenwick_fixed& c)
	{
		for (uint32_t i = 0; i < N_SYMBOLS; ++i)
			arr[i] = c.arr[i];
		total = c.total;
	}

	freq_model_fenwick_fixed& operator=(const freq_model_fenwick_fixed& c)
	{
		if (this != &c)
		{
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
				arr[i] = c.arr[i];
			total = c.total;
		}

		return *this;
	}

	void init(const uint32_t* _init_stats)
	{
		if (_init_stats)
			for (uint32_t i = 0; i < N_SYMBOLS; ++i)
				arr[i] = _init_stats[i];
		else
			std::fill_n(arr, N_SYMBOLS, 1);

		total = std::accumulate(arr, arr + N_SYMBOLS, 0u);
		//		rescale();

		make_fenwick_tree();
	}

	void init(const freq_model_fenwick_fixed& c)
	{
		std::copy_n(c.arr, N_SYMBOLS, arr);
		total = c.total;
	}

	void get_freq(uint32_t symbol, uint32_t& sym_freq, uint32_t& left_freq, uint32_t& totf)
	{
		left_freq = prefix_sum(symbol);
		sym_freq = value(symbol);
		totf = total;
	}

	void update(uint32_t symbol)
	{
		increment(symbol);
		total += ADDER;

		if (total >= MAX_TOTAL)
			rescale();
	}

	uint32_t get_sym(uint32_t left_freq)
	{
		uint32_t i = 0;
		uint32_t j = N_SYMBOLS;

		j = LSB[j];

		for (; j > 0; j >>= 1)
		{
			if (i + j <= N_SYMBOLS && arr[i + j - 1] <= left_freq)
			{
				left_freq -= arr[i + j - 1];
				i += j;
			}
		}

		return i;
	}

	/*	void get_sym_freq_and_update(int& left_freq, int& sym, int& sym_freq)
		{
		}*/

	uint32_t get_total()
	{
		return total;
	}
};

#undef REFRESH_RC_CASE
} // namespace refresh

#endif
// EOF
