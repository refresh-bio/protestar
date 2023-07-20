#ifndef _RC_ENGINE_H
#define _RC_ENGINE_H
#pragma once

#include <iostream>
#include <cmath>
#include <cstdint>

#include <assert.h>

#define UNROLL_FREQUENCY_CODING

#define RC_64BIT

namespace refresh
{
// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM> class rc_engine
{
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM> class rc_encoder : public rc_engine<T_IO_STREAM>
{
public:
#ifdef RC_64BIT
	typedef uint64_t code_t;
	typedef uint64_t freq_t;

	const freq_t TopValue = 0x00ffffffffffffULL;
	const code_t Mask = 0xff00000000000000ULL;
#else
	typedef uint32_t code_t;
	typedef uint32_t freq_t;

	const freq_t TopValue = 0x00ffffULL;
	const code_t Mask = 0xff000000ULL;
#endif

	const uint32_t MAX_SHIFT = 8 * sizeof(code_t) - 8;
	const uint32_t CODE_BYTES = sizeof(code_t);

	code_t low;
	freq_t range;

	T_IO_STREAM& io_stream;

	rc_encoder(T_IO_STREAM& _io_stream) : low(0), range(0), io_stream(_io_stream)
	{}

	void start()
	{
		low = 0;
		range = Mask;
	}

	void encode_frequency(freq_t symFreq_, freq_t cumFreq_, freq_t totalFreqSum_)
	{
		assert(range > totalFreqSum_);
		range /= totalFreqSum_;
		low += range * cumFreq_;
		range *= symFreq_;

#ifndef UNROLL_FREQUENCY_CODING
		while (range <= TopValue)
		{
			assert(range != 0);
			if ((low ^ (low + range)) & Mask)
			{
				freq_t r = (freq_t)low;
				range = (r | TopValue) - r;
			}
			io_stream.PutByte(low >> MAX_SHIFT);
			low <<= 8, range <<= 8;
		}
#else
		if (range <= TopValue)
		{
			assert(range != 0);
			if ((low ^ (low + range)) & Mask)
			{
				freq_t r = (freq_t)low;
				range = (r | TopValue) - r;
			}
			io_stream.put_byte(low >> MAX_SHIFT);
			low <<= 8, range <<= 8;

			if (range <= TopValue)
			{
				assert(range != 0);
				if ((low ^ (low + range)) & Mask)
				{
					freq_t r = (freq_t)low;
					range = (r | TopValue) - r;
				}
				io_stream.put_byte(low >> MAX_SHIFT);
				low <<= 8, range <<= 8;

				if (range <= TopValue)
				{
					assert(range != 0);
					if ((low ^ (low + range)) & Mask)
					{
						freq_t r = (freq_t)low;
						range = (r | TopValue) - r;
					}
					io_stream.put_byte(low >> MAX_SHIFT);
					low <<= 8, range <<= 8;

					if (range <= TopValue)
					{
						assert(range != 0);
						if ((low ^ (low + range)) & Mask)
						{
							freq_t r = (freq_t)low;
							range = (r | TopValue) - r;
						}
						io_stream.put_byte(low >> MAX_SHIFT);
						low <<= 8, range <<= 8;

#ifdef RC_64BIT
						if (range <= TopValue)
						{
							assert(range != 0);
							if ((low ^ (low + range)) & Mask)
							{
								freq_t r = (freq_t)low;
								range = (r | TopValue) - r;
							}
							io_stream.put_byte(low >> MAX_SHIFT);
							low <<= 8, range <<= 8;

							if (range <= TopValue)
							{
								assert(range != 0);
								if ((low ^ (low + range)) & Mask)
								{
									freq_t r = (freq_t)low;
									range = (r | TopValue) - r;
								}
								io_stream.put_byte(low >> MAX_SHIFT);
								low <<= 8, range <<= 8;

								if (range <= TopValue)
								{
									assert(range != 0);
									if ((low ^ (low + range)) & Mask)
									{
										freq_t r = (freq_t)low;
										range = (r | TopValue) - r;
									}
									io_stream.put_byte(low >> MAX_SHIFT);
									low <<= 8, range <<= 8;

									if (range <= TopValue)
									{
										assert(range != 0);
										if ((low ^ (low + range)) & Mask)
										{
											freq_t r = (freq_t)low;
											range = (r | TopValue) - r;
										}
										io_stream.put_byte(low >> MAX_SHIFT);
										low <<= 8, range <<= 8;
									}
								}
							}
						}
#endif
					}
				}
			}
		}
#endif
	}

	void complete()
	{
		for (uint32_t i = 0; i < CODE_BYTES; i++)
		{
			io_stream.put_byte(low >> MAX_SHIFT);
			low <<= 8;
		}
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM> class rc_decoder : public rc_engine<T_IO_STREAM>
{
public:
#ifdef RC_64BIT
	typedef uint64_t code_t;
	typedef uint64_t freq_t;

	const freq_t TopValue = 0x00ffffffffffffULL;
	const code_t Mask = 0xff00000000000000ULL;
#else
	typedef uint32_t code_t;
	typedef uint32_t freq_t;

	const freq_t TopValue = 0x00ffffULL;
	const code_t Mask = 0xff000000ULL;
#endif

	const uint32_t MAX_SHIFT = 8 * sizeof(code_t) - 8;
	const uint32_t CODE_BYTES = sizeof(code_t);
	const uint32_t CODE_BITS = 8 * sizeof(code_t);

	code_t low;
	freq_t range;

	T_IO_STREAM& io_stream;

	rc_decoder(T_IO_STREAM& _io_stream) : low(0), range(0), io_stream(_io_stream)
	{
		buffer = 0;
	}

	void start()
	{
		if (io_stream.size() < CODE_BYTES)
			return;

		buffer = 0;
		for (uint32_t i = 1; i <= CODE_BYTES; ++i)
		{
			buffer |= (code_t)io_stream.get_byte() << (CODE_BITS - i * 8);
		}

		low = 0;
		range = Mask;
	}

	freq_t get_cumulative_freq(freq_t totalFreq_)
	{
		assert(totalFreq_ != 0);
		return (freq_t)(buffer / (range /= totalFreq_));
	}

	void update_frequency(freq_t symFreq_, freq_t lowEnd_, freq_t /*totalFreq_*/)
	{
		freq_t r = lowEnd_ * range;
		buffer -= r;
		low += r;
		range *= symFreq_;

#ifndef UNROLL_FREQUENCY_CODING
		while (range <= TopValue)
		{
			if ((low ^ (low + range)) & Mask)
			{
				freq_t r = (freq_t)low;
				range = (r | TopValue) - r;
			}

			buffer = (buffer << 8) + io_stream.GetByte();
			low <<= 8, range <<= 8;
		}
#else
		if (range <= TopValue)
		{
			if ((low ^ (low + range)) & Mask)
			{
				freq_t r = (freq_t)low;
				range = (r | TopValue) - r;
			}

			buffer = (buffer << 8) + io_stream.get_byte();
			low <<= 8, range <<= 8;

			if (range <= TopValue)
			{
				if ((low ^ (low + range)) & Mask)
				{
					freq_t r = (freq_t)low;
					range = (r | TopValue) - r;
				}

				buffer = (buffer << 8) + io_stream.get_byte();
				low <<= 8, range <<= 8;

				if (range <= TopValue)
				{
					if ((low ^ (low + range)) & Mask)
					{
						freq_t r = (freq_t)low;
						range = (r | TopValue) - r;
					}

					buffer = (buffer << 8) + io_stream.get_byte();
					low <<= 8, range <<= 8;

					if (range <= TopValue)
					{
						if ((low ^ (low + range)) & Mask)
						{
							freq_t r = (freq_t)low;
							range = (r | TopValue) - r;
						}

						buffer = (buffer << 8) + io_stream.get_byte();
						low <<= 8, range <<= 8;

#ifdef RC_64BIT
						if (range <= TopValue)
						{
							if ((low ^ (low + range)) & Mask)
							{
								freq_t r = (freq_t)low;
								range = (r | TopValue) - r;
							}

							buffer = (buffer << 8) + io_stream.get_byte();
							low <<= 8, range <<= 8;

							if (range <= TopValue)
							{
								if ((low ^ (low + range)) & Mask)
								{
									freq_t r = (freq_t)low;
									range = (r | TopValue) - r;
								}

								buffer = (buffer << 8) + io_stream.get_byte();
								low <<= 8, range <<= 8;

								if (range <= TopValue)
								{
									if ((low ^ (low + range)) & Mask)
									{
										freq_t r = (freq_t)low;
										range = (r | TopValue) - r;
									}

									buffer = (buffer << 8) + io_stream.get_byte();
									low <<= 8, range <<= 8;

									if (range <= TopValue)
									{
										if ((low ^ (low + range)) & Mask)
										{
											freq_t r = (freq_t)low;
											range = (r | TopValue) - r;
										}

										buffer = (buffer << 8) + io_stream.get_byte();
										low <<= 8, range <<= 8;
									}
								}
							}
						}
#endif
					}
				}
			}
		}
#endif
	}

	void complete()
	{}

private:
	code_t buffer;
};
} // namespace refresh

#endif

// EOF
