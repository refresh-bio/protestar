#ifndef _RC_META_SWITCH
#define _RC_META_SWITCH
#include <cinttypes>

namespace refresh
{
template<unsigned N>
struct rc_switch_impl
{};

template<>
struct rc_switch_impl<10>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 9: left_freq += stats[8]; [[fallthrough]];
		case 8: left_freq += stats[7]; [[fallthrough]];
		case 7: left_freq += stats[6]; [[fallthrough]];
		case 6: left_freq += stats[5]; [[fallthrough]];
		case 5: left_freq += stats[4]; [[fallthrough]];
		case 4: left_freq += stats[3]; [[fallthrough]];
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};

template<>
struct rc_switch_impl<9>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 8: left_freq += stats[7]; [[fallthrough]];
		case 7: left_freq += stats[6]; [[fallthrough]];
		case 6: left_freq += stats[5]; [[fallthrough]];
		case 5: left_freq += stats[4]; [[fallthrough]];
		case 4: left_freq += stats[3]; [[fallthrough]];
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};


template<>
struct rc_switch_impl<8>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 7: left_freq += stats[6]; [[fallthrough]];
		case 6: left_freq += stats[5]; [[fallthrough]];
		case 5: left_freq += stats[4]; [[fallthrough]];
		case 4: left_freq += stats[3]; [[fallthrough]];
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};

template<>
struct rc_switch_impl<7>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 6: left_freq += stats[5]; [[fallthrough]];
		case 5: left_freq += stats[4]; [[fallthrough]];
		case 4: left_freq += stats[3]; [[fallthrough]];
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};


template<>
struct rc_switch_impl<6>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 5: left_freq += stats[4]; [[fallthrough]];
		case 4: left_freq += stats[3]; [[fallthrough]];
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};


template<>
struct rc_switch_impl<5>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 4: left_freq += stats[3]; [[fallthrough]];
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};


template<>
struct rc_switch_impl<4>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 3: left_freq += stats[2]; [[fallthrough]];
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};

template<>
struct rc_switch_impl<3>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 2: left_freq += stats[1]; [[fallthrough]];
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};


template<>
struct rc_switch_impl<2>
{
	static void get_freq(uint32_t symbol, uint32_t& left_freq, uint32_t* stats)
	{
		switch (symbol)
		{
		case 1: left_freq += stats[0]; [[fallthrough]];
		case 0: break;
		}
	}
};


template<unsigned N>
struct rc_if_impl
{};

template<>
struct rc_if_impl<10>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		t += stats[3];
		if (left_freq < t)		return 3;
		t += stats[4];
		if (left_freq < t)		return 4;
		t += stats[5];
		if (left_freq < t)		return 5;
		t += stats[6];
		if (left_freq < t)		return 6;
		t += stats[7];
		if (left_freq < t)		return 7;
		t += stats[8];
		if (left_freq < t)		return 8;
		return 9;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		t += stats[3];
		if (left_freq < t) { left_freq = t - stats[3];  return 3; }
		t += stats[4];
		if (left_freq < t) { left_freq = t - stats[4];  return 4; }
		t += stats[5];
		if (left_freq < t) { left_freq = t - stats[5];  return 5; }
		t += stats[6];
		if (left_freq < t) { left_freq = t - stats[6];  return 6; }
		t += stats[7];
		if (left_freq < t) { left_freq = t - stats[7];  return 7; }
		t += stats[8];
		if (left_freq < t) { left_freq = t - stats[8];  return 8; }
		left_freq = t;
		return 9;
	}
};

template<>
struct rc_if_impl<9>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		t += stats[3];
		if (left_freq < t)		return 3;
		t += stats[4];
		if (left_freq < t)		return 4;
		t += stats[5];
		if (left_freq < t)		return 5;
		t += stats[6];
		if (left_freq < t)		return 6;
		t += stats[7];
		if (left_freq < t)		return 7;
		return 8;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		t += stats[3];
		if (left_freq < t) { left_freq = t - stats[3];  return 3; }
		t += stats[4];
		if (left_freq < t) { left_freq = t - stats[4];  return 4; }
		t += stats[5];
		if (left_freq < t) { left_freq = t - stats[5];  return 5; }
		t += stats[6];
		if (left_freq < t) { left_freq = t - stats[6];  return 6; }
		t += stats[7];
		if (left_freq < t) { left_freq = t - stats[7];  return 7; }
		left_freq = t;
		return 8;
	}
};

template<>
struct rc_if_impl<8>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		t += stats[3];
		if (left_freq < t)		return 3;
		t += stats[4];
		if (left_freq < t)		return 4;
		t += stats[5];
		if (left_freq < t)		return 5;
		t += stats[6];
		if (left_freq < t)		return 6;
		return 7;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		t += stats[3];
		if (left_freq < t) { left_freq = t - stats[3];  return 3; }
		t += stats[4];
		if (left_freq < t) { left_freq = t - stats[4];  return 4; }
		t += stats[5];
		if (left_freq < t) { left_freq = t - stats[5];  return 5; }
		t += stats[6];
		if (left_freq < t) { left_freq = t - stats[6];  return 6; }
		left_freq = t;
		return 7;
	}
};

template<>
struct rc_if_impl<7>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		t += stats[3];
		if (left_freq < t)		return 3;
		t += stats[4];
		if (left_freq < t)		return 4;
		t += stats[5];
		if (left_freq < t)		return 5;
		return 6;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		t += stats[3];
		if (left_freq < t) { left_freq = t - stats[3];  return 3; }
		t += stats[4];
		if (left_freq < t) { left_freq = t - stats[4];  return 4; }
		t += stats[5];
		if (left_freq < t) { left_freq = t - stats[5];  return 5; }
		left_freq = t;
		return 6;
	}
};

template<>
struct rc_if_impl<6>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		t += stats[3];
		if (left_freq < t)		return 3;
		t += stats[4];
		if (left_freq < t)		return 4;
		return 5;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		t += stats[3];
		if (left_freq < t) { left_freq = t - stats[3];  return 3; }
		t += stats[4];
		if (left_freq < t) { left_freq = t - stats[4];  return 4; }
		left_freq = t;
		return 5;
	}
};

template<>
struct rc_if_impl<5>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		t += stats[3];
		if (left_freq < t)		return 3;
		return 4;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		t += stats[3];
		if (left_freq < t) { left_freq = t - stats[3];  return 3; }
		left_freq = t;
		return 4;
	}
};

template<>
struct rc_if_impl<4>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		t += stats[2];
		if (left_freq < t)		return 2;
		return 3;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		t += stats[2];
		if (left_freq < t) { left_freq = t - stats[2];  return 2; }
		left_freq = t;
		return 3;
	}
};

template<>
struct rc_if_impl<3>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t)		return 0;
		t += stats[1];
		if (left_freq < t)		return 1;
		return 2;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		t += stats[1];
		if (left_freq < t) { left_freq = t - stats[1];  return 1; }
		left_freq = t;
		return 2;
	}
};

template<>
struct rc_if_impl<2>
{
	static uint32_t get_sym(uint32_t left_freq, uint32_t* stats)
	{
		if (left_freq < stats[0])		return 0;
		return 1;
	}

	static uint32_t get_sym_lf(uint32_t& left_freq, uint32_t* stats)
	{
		uint32_t t = stats[0];
		if (left_freq < t) { left_freq = t - stats[0];  return 0; }
		left_freq = t;
		return 1;
	}
};

}
#endif
