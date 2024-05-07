#pragma once
#include <vector>
#include <cinttypes>
#include <string>
#include <map>
#include <set>
#include <cmath>
#include <utility>

//#include "../parsers/cif.h"
//#include "../parsers/pdb.h"
#include "../parsers/entries.h"

#include "../common/defs.h"
#include "../common/aa_atoms.h"
#include "../core/utils.h"

using namespace std;

struct atom_short_desc_t
{
	int id;
	int_coords_t coords;
	int B_factor;
	char type_symbol;

	atom_short_desc_t(int id = 0, int_coords_t coords = { 0,0,0 }, int B_factor = 0, char type_symbol = ' ') :
		id(id),
		coords(coords),
		B_factor(B_factor),
		type_symbol(type_symbol)
	{}
};

struct atom_desc_t
{
	int id;
	atom_t type;
	int_coords_t coords;
	int B_factor;
	char type_symbol;
	string type_str;

	atom_desc_t(int id, atom_t type = atom_t::unknown, int_coords_t coords = { 0,0,0 }, int B_factor = 0, const char type_symbol = ' ', const string& type_str = "") :
		id(id),
		type(type),
		coords(coords),
		B_factor(B_factor),
		type_symbol(type_symbol),
		type_str(type_str)
	{}

	atom_desc_t(atom_t type = atom_t::unknown, atom_short_desc_t short_desc = {}, const string & type_str = "") :
		id(short_desc.id),
		type(type),
		coords(short_desc.coords),
		B_factor(short_desc.B_factor),
		type_symbol(short_desc.type_symbol),
		type_str(type_str)
	{}

	atom_short_desc_t short_desc()
	{
		return atom_short_desc_t(id, coords, B_factor, type_symbol);
	}
};

struct typicality_t
{
	static constexpr int id_shift = 0;
	static constexpr int ao_shift = 1;
	static constexpr int bf_shift = 2;
	static constexpr int ts_shift = 3;
	static constexpr int oxt_shift = 4;
	static constexpr int aa_shift = 5;
	static constexpr uint32_t id_mask = 1u << id_shift;
	static constexpr uint32_t ao_mask = 1u << ao_shift;
	static constexpr uint32_t bf_mask = 1u << bf_shift;
	static constexpr uint32_t ts_mask = 1u << ts_shift;
	static constexpr uint32_t oxt_mask = 1u << oxt_shift;
	static constexpr uint32_t aa_mask = 1u << aa_shift;
	static constexpr uint32_t all_mask = id_mask + ao_mask + bf_mask + ts_mask + oxt_mask + aa_mask;
	static constexpr uint32_t contents_mask = id_mask + ao_mask + bf_mask + ts_mask + oxt_mask;
	static constexpr uint32_t basic_mask = id_mask + ao_mask + ts_mask;

	uint32_t state;

	typicality_t() :
		state(0)
	{}

	typicality_t(uint32_t state) :
		state(state)
	{}

	typicality_t(bool typical_ids, bool typical_atom_order, bool typical_B_factor, bool typical_type_symbol, bool typical_oxt, bool typical_aa) :
		state(typical_ids * id_mask +
			typical_atom_order * ao_mask +
			typical_B_factor * bf_mask +
			typical_type_symbol * ts_mask +
			typical_oxt * oxt_mask +
			typical_aa * aa_mask)
	{}

	typicality_t(const typicality_t& rhs)
	{
		state = rhs.state;
	}

	bool check_all() const
	{
		return state == all_mask;
	}

	bool check_basic() const
	{
		return (state & basic_mask) == basic_mask;
	}

	bool check_ids() const
	{
		return (bool)(state & id_mask);
	}

	bool check_atom_order() const
	{
		return (bool)(state & ao_mask);
	}

	bool check_B_factor() const
	{
		return (bool)(state & bf_mask);
	}

	bool check_oxt() const
	{
		return (bool)(state & oxt_mask);
	}

	bool check_type_symbol() const
	{
		return (bool)(state & ts_mask);
	}

	bool check_aa() const
	{
		return (bool)(state & aa_mask);
	}

	void set(uint32_t mask)
	{
		state |= mask;
	}

	void reset(uint32_t mask)
	{
		state &= ~mask;
	}

	void set_all()
	{
		state = all_mask;
	}

	void reset_all()
	{
		state = 0;
	}

	bool check(uint32_t mask) const
	{
		return (state & mask) == mask;
	}

	typicality_t& operator&=(const typicality_t& rhs)
	{
		state &= rhs.state;

		return *this;
	}

	typicality_t& operator|=(const typicality_t& rhs)
	{
		state |= rhs.state;

		return *this;
	}
};

struct aa_desc_t
{
	aa_t type;
	int seq_id;
	vector<atom_desc_t> atoms;
	typicality_t typical;
	string type_str;

	aa_desc_t(aa_t type, int seq_id, vector<atom_desc_t>& atoms, bool typical_ids, bool typical_atom_order, bool typical_B_factor, bool typical_type_symbol, bool typical_oxt, bool typical_aa, const string &type_str) :
		type(type),
		seq_id(seq_id),
		atoms(atoms),
		typical(typical_ids, typical_atom_order, typical_B_factor, typical_type_symbol, typical_oxt, typical_aa),
		type_str(type_str)
	{}

	aa_desc_t(aa_t type = aa_t::unknown, int seq_id = 0, const vector<atom_desc_t>& atoms = {}, typicality_t typical = typicality_t{}, const string &type_str = "") :
		type(type),
		seq_id(seq_id),
		atoms(atoms),
		typical(typical),
		type_str(type_str)
	{}

	aa_desc_t(aa_desc_t&& rhs) = default;
	aa_desc_t(const aa_desc_t& rhs) = default;

	aa_desc_t& operator=(aa_desc_t&& rhs) noexcept
	{
		type = rhs.type;
		seq_id = rhs.seq_id;
		atoms = move(rhs.atoms);
		typical = rhs.typical;
		type_str = rhs.type_str;
		rhs.type = aa_t::unknown;
		rhs.type_str = "";

		return *this;
	}

	aa_desc_t& operator=(const aa_desc_t& rhs)
	{
		type = rhs.type;
		seq_id = rhs.seq_id;
		atoms = rhs.atoms;
		typical = rhs.typical;
		type_str = rhs.type_str;

		return *this;
	}
};

struct chain_desc_t
{
	vector<aa_desc_t> aa;
	typicality_t typical;
	char id;

	chain_desc_t(char id = ' ', const vector<aa_desc_t>&aa = {}, typicality_t typical = typicality_t{}) :
		aa(aa),
		typical(typical),
		id(id)
	{}

	bool check_seq_id() const
	{
		for (size_t i = 1; i < aa.size(); ++i)
			if (aa[i - 1].seq_id + 1 != aa[i].seq_id)
				return false;

		return true;
	}
}; 

const int cart_working_precision = 6;		// Must be consistent with CifCompressor!
//const int cart_working_precision = 3;		// Must be consistent with CifCompressor!

// *****************************************************************
int coords_distance(const int_coords_t& a, const int_coords_t& b);
int64_t coords_distance2(const int_coords_t& a, const int_coords_t& b);
//int64_t coords_distance2_precision(const int_coords_t& a, const int_coords_t& b, int64_t precision);
double est_delta_coding_cost(const int_coords_t& a, const int_coords_t& b);

// *****************************************************************
class Protein
{
	vector<chain_desc_t> v_chains;
	int cart_precision = 0;

	void check_typicality();

public:
	Protein() = default;
	~Protein() = default;

	Protein(Protein&& rhs) noexcept
	{
		v_chains = move(rhs.v_chains);
		cart_precision = rhs.cart_precision;
	}

	Protein& operator=(Protein&& rhs) noexcept
	{
		v_chains = move(rhs.v_chains);
		cart_precision = rhs.cart_precision;

		return *this;
	}

	bool parse_chains(const LoopEntry* le);

	size_t no_chains();
	vector<chain_desc_t>& get_chains();
	chain_desc_t& get_chain(int id);
	
	void expand_decimals(int requested_precision);
	
	int get_cart_precision()
	{
		return cart_precision;
	}

	bool all_chains_valid_seq_id() const
	{
		for (const auto& chain : v_chains)
			if (!chain.check_seq_id())
				return false;

		return true;
	}
};

// EOF
