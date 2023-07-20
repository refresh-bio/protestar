#ifndef _RC_CODERS_H
#define _RC_CODERS_H

#include "rc_engine.h"
#include "rc_models.h"

namespace refresh
{
	using rc_context_t = uint64_t;

// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM, unsigned MAX_TOTAL, unsigned ADDER> class rc_simple
{
	union {
		rc_encoder<T_IO_STREAM>* rce;
		rc_decoder<T_IO_STREAM>* rcd;
	} rc;

	freq_model_simple<MAX_TOTAL, ADDER> simple_model;

	uint32_t no_symbols;

public:
	rc_simple(rc_engine<T_IO_STREAM>* rcb, uint32_t _no_symbols, uint32_t* _init, bool compress) :
		no_symbols(_no_symbols)
	{
		simple_model.init(no_symbols, _init);

		if (compress)
			rc.rce = (rc_encoder<T_IO_STREAM>*) (rcb);
		else
			rc.rcd = (rc_decoder<T_IO_STREAM>*) (rcb);
	}

	rc_simple(const rc_simple& c)
	{
		simple_model.init(c.simple_model);
		rc = c.rc;

		no_symbols = c.no_symbols;	
		//		totf = c.totf;
		//		rescale = c.rescale;
	}

	~rc_simple()
	{
	}

	void encode(const uint32_t x)
	{
		uint32_t syfreq, ltfreq, totf;
		simple_model.get_freq(x, syfreq, ltfreq, totf);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	void encode_excluding(const uint32_t x, const uint32_t exc)
	{
		uint32_t syfreq, ltfreq, totf;
		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	void encode_excluding(const uint32_t x, const uint32_t exc1, const uint32_t exc2)
	{
		uint32_t syfreq, ltfreq, totf;
		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	void encode_excluding(const uint32_t x, const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		uint32_t syfreq, ltfreq, totf;
		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2, exc3);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	uint32_t decode()
	{
		uint32_t syfreq, ltfreq, totf;

		totf = simple_model.get_total();
		ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym(ltfreq);

		simple_model.get_freq(x, syfreq, ltfreq, totf);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc)
	{
		uint32_t syfreq, ltfreq, totf;

		totf = simple_model.get_total_exc(exc);
		ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_esc(ltfreq, exc);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc1, const uint32_t exc2)
	{
		uint32_t syfreq, ltfreq, totf;

		totf = simple_model.get_total_exc(exc1, exc2);
		ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc1, exc2);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		uint32_t syfreq, ltfreq, totf;

		totf = simple_model.get_total_exc(exc1, exc2, exc3);
		ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc1, exc2, exc3);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2, exc3);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}

	void init(uint32_t* init)
	{
		simple_model.init(no_symbols, init);
	}
};

// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM, unsigned N_SYMBOLS, unsigned MAX_TOTAL, unsigned ADDER> class rc_simple_fixed
{
	union {
		rc_encoder<T_IO_STREAM>* rce;
		rc_decoder<T_IO_STREAM>* rcd;
	} rc;

	freq_model_simple_fixed<N_SYMBOLS, MAX_TOTAL, ADDER> simple_model;

public:
	rc_simple_fixed(rc_engine<T_IO_STREAM>* rcb, uint32_t* _init, bool compress)
	{
		simple_model.init(_init);

		if (compress)
			rc.rce = (rc_encoder<T_IO_STREAM>*) (rcb);
		else
			rc.rcd = (rc_decoder<T_IO_STREAM>*) (rcb);
	}

	rc_simple_fixed(const rc_simple_fixed& c)
	{
		simple_model.init(c.simple_model);
		rc = c.rc;
	}

	~rc_simple_fixed()
	{
	}

	void encode(const uint32_t x)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq(x, syfreq, ltfreq, totf);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	void encode_excluding(const uint32_t x, const uint32_t exc)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	void encode_excluding(const uint32_t x, const uint32_t exc1, const uint32_t exc2)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	void encode_excluding(const uint32_t x, const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2, exc3);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);
	}

	uint32_t decode()
	{
		uint32_t syfreq, ltfreq;
		uint32_t totf = simple_model.get_total();
		uint32_t x = 0;

		ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		//		int x = simple_model.get_sym(ltfreq);
		//		simple_model.get_freq(x, syfreq, ltfreq, totf);

		simple_model.get_sym_freq_and_update(ltfreq, x, syfreq);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		//		simple_model.update(x);

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc)
	{
		uint32_t syfreq;
		uint32_t totf = simple_model.get_total_exc(exc);
		uint32_t ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc1, const uint32_t exc2)
	{
		uint32_t syfreq;
		uint32_t totf = simple_model.get_total_exc(exc1, exc2);
		uint32_t ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc1, exc2);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		uint32_t syfreq;
		uint32_t totf = simple_model.get_total_exc(exc1, exc2, exc3);
		uint32_t ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc1, exc2, exc3);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2, exc3);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		return x;
	}
};


// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM, unsigned N_SYMBOLS, unsigned MAX_TOTAL, unsigned ADDER> class rc_simple_fixed_counting
{
	union {
		rc_encoder<T_IO_STREAM>* rce;
		rc_decoder<T_IO_STREAM>* rcd;
	} rc;

	freq_model_simple_fixed<N_SYMBOLS, MAX_TOTAL, ADDER> simple_model;
	size_t counter;

public:
	rc_simple_fixed_counting(rc_engine<T_IO_STREAM>* rcb, uint32_t* _init, bool compress)
	{
		simple_model.init(_init);

		counter = 0;

		if (compress)
			rc.rce = (rc_encoder<T_IO_STREAM>*) (rcb);
		else
			rc.rcd = (rc_decoder<T_IO_STREAM>*) (rcb);
	}

	rc_simple_fixed_counting(const rc_simple_fixed_counting& c)
	{
		simple_model.init(c.simple_model);
		rc = c.rc;
		counter = c.counter;
	}

	~rc_simple_fixed_counting()
	{
	}

	void encode(const uint32_t x)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq(x, syfreq, ltfreq, totf);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);

		++counter;
	}

	void encode_excluding(const uint32_t x, const uint32_t exc)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);

		++counter;
	}

	void encode_excluding(const uint32_t x, const uint32_t exc1, const uint32_t exc2)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);

		++counter;
	}

	void encode_excluding(const uint32_t x, const uint32_t exc1, const uint32_t exc2, const uint32_t exc3)
	{
		uint32_t syfreq, ltfreq, totf;

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2, exc3);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		simple_model.update(x);

		++counter;
	}

	uint32_t decode()
	{
		uint32_t syfreq, ltfreq;
		uint32_t totf = simple_model.get_total();
		uint32_t x = 0;

		ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		//		int x = simple_model.get_sym(ltfreq);
		//		simple_model.get_freq(x, syfreq, ltfreq, totf);

		simple_model.get_sym_freq_and_update(ltfreq, x, syfreq);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		//		simple_model.update(x);

		++counter;

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc)
	{
		uint32_t syfreq;
		uint32_t totf = simple_model.get_total_exc(exc);
		uint32_t ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		++counter;

		return x;
	}

	uint32_t decode_excluding(const uint32_t exc1, const uint32_t exc2)
	{
		uint32_t syfreq;
		uint32_t totf = simple_model.get_total_exc(exc1, exc2);
		uint32_t ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = simple_model.get_sym_exc(ltfreq, exc1, exc2);

		simple_model.get_freq_exc(x, syfreq, ltfreq, totf, exc1, exc2);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		simple_model.update(x);

		++counter;

		return x;
	}

	void force_rescale()
	{
		simple_model.force_rescale();
	}

	size_t get_counter()
	{
		return counter;
	}

	void set_counter(size_t _counter = 0)
	{
		counter = _counter;
	}
};


// *******************************************************************************************
//
// *******************************************************************************************
template<typename T_IO_STREAM, unsigned N_SYMBOLS, unsigned MAX_TOTAL, unsigned ADDER> class rc_fenwick_fixed
{
	union {
		rc_encoder<T_IO_STREAM>* rce;
		rc_decoder<T_IO_STREAM>* rcd;
	} rc;

	freq_model_fenwick_fixed<N_SYMBOLS, MAX_TOTAL, ADDER> fenwick_tree_model;

public:
	rc_fenwick_fixed(rc_engine<T_IO_STREAM>* rcb, uint32_t* _init, bool compress)
	{
		fenwick_tree_model.init(_init);

		if (compress)
			rc.rce = (rc_encoder<T_IO_STREAM>*) (rcb);
		else
			rc.rcd = (rc_decoder<T_IO_STREAM>*) (rcb);
	}

	rc_fenwick_fixed(const rc_fenwick_fixed& c)
	{
		fenwick_tree_model.init(c.fenwick_tree_model);
		rc = c.rc;
	}

	~rc_fenwick_fixed()
	{
	}

	void encode(const uint32_t x)
	{
		uint32_t syfreq, ltfreq, totf;

		fenwick_tree_model.get_freq(x, syfreq, ltfreq, totf);
		rc.rce->encode_frequency(syfreq, ltfreq, totf);

		fenwick_tree_model.update(x);
	}

	uint32_t decode()
	{
		uint32_t syfreq;
		uint32_t totf = fenwick_tree_model.get_total();
		uint32_t ltfreq = static_cast<uint32_t>(rc.rcd->get_cumulative_freq(totf));

		uint32_t x = fenwick_tree_model.get_sym(ltfreq);

		fenwick_tree_model.get_freq(x, syfreq, ltfreq, totf);
		rc.rcd->update_frequency(syfreq, ltfreq, totf);
		fenwick_tree_model.update(x);

		return x;
	}
};

}

#endif
