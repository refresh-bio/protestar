#include "aa_atoms.h"

// *****************************************************************
aa_t str_to_aa(const string& str)
{
	const auto p = m_aa.find(str);
	if (p == m_aa.end())
		return aa_t::unknown;

	return p->second;
}

// *****************************************************************
atom_t str_to_atom(const string& str)
{
	const auto p = m_atom.find(str);
	if (p == m_atom.end())
		return atom_t::unknown;

	return p->second;
}

// *****************************************************************
string aa_to_str(const aa_t aa)
{
	for (const auto& x : m_aa)
		if (x.second == aa)
			return x.first;

	return "unknown";
}

// *****************************************************************
string atom_to_str(const atom_t atom)
{
	for (const auto& x : m_atom)
		if (x.second == atom)
			return x.first;

	return "unknown";
}

// *****************************************************************
string aa_atom_to_str(const pair<aa_t, atom_t>& aa_atom)
{
	return aa_to_str(aa_atom.first) + ":" + atom_to_str(aa_atom.second);
}

// EOF
