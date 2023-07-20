#pragma once

#include <vector>
#include <cinttypes>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <cmath>
#include <utility>
#include "defs.h"

using namespace std;

// *****************************************************************
enum class aa_t { unknown, ARG, HIS, LYS, ASP, GLU, SER, THR, ASN, GLN, CYS, SEC, GLY, PRO, ALA, VAL, ILE, LEU, MET, PHE, TYR, TRP };
enum class atom_t { unknown, _N, _C, _CA, _O, N, C, CA, O, CB, CG, CG1, CG2, CD, CD1, CD2, CE, CE1, CE2, CE3, CZ, CZ2, CZ3, CH2, ND1, ND2, NE, NE1, NE2, NH1, NH2, NZ, OG, OG1, OD1, OD2, OE1, OE2, OH, OXT, SG, SD, SE };

// *****************************************************************
const unordered_map<aa_t, vector<atom_t>> m_aa_atom = {
	{aa_t::unknown, {}},
	{aa_t::ARG, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::NE, atom_t::NH1, atom_t::NH2, atom_t::CZ, atom_t::OXT }},
	{aa_t::HIS, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD2, atom_t::ND1, atom_t::CE1, atom_t::NE2, atom_t::OXT }},
	{aa_t::LYS, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::CE, atom_t::NZ, atom_t::OXT }},
	{aa_t::ASP, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::OD1, atom_t::OD2, atom_t::OXT }},
	{aa_t::GLU, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::OE1, atom_t::OE2, atom_t::OXT }},
	{aa_t::SER, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::OG, atom_t::OXT }},
	{aa_t::THR, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG2, atom_t::OG1, atom_t::OXT }},
	{aa_t::ASN, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::ND2, atom_t::OD1, atom_t::OXT }},
	{aa_t::GLN, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::NE2, atom_t::OE1, atom_t::OXT }},
	{aa_t::CYS, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::SG, atom_t::OXT }},
	{aa_t::SEC, { atom_t::N, atom_t::CA, atom_t::CB, atom_t::SE, atom_t::C, atom_t::O, atom_t::OXT }},			// Na podstawie https://www.wwpdb.org/pdb?id=pdb_00001cc1
	{aa_t::GLY, { atom_t::N, atom_t::CA, atom_t::C, atom_t::O, atom_t::OXT }},
	{aa_t::PRO, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::OXT }},
	{aa_t::ALA, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::OXT }},
	{aa_t::VAL, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG1, atom_t::CG2, atom_t::OXT }},
	{aa_t::ILE, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG1, atom_t::CG2, atom_t::CD1, atom_t::OXT }},
	{aa_t::LEU, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::OXT }},
	{aa_t::MET, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::SD, atom_t::CE, atom_t::OXT }},
	{aa_t::PHE, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE1, atom_t::CE2, atom_t::CZ, atom_t::OXT }},
	{aa_t::TYR, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE1, atom_t::CE2, atom_t::OH, atom_t::CZ, atom_t::OXT }},
	{aa_t::TRP, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE2, atom_t::CE3, atom_t::NE1, atom_t::CH2, atom_t::CZ2, atom_t::CZ3, atom_t::OXT }} };

// *****************************************************************
const unordered_map<atom_t, char> atom_type_char = {
	{ atom_t::unknown, ' ' },
	{atom_t::C, 'C'}, {atom_t::CA, 'C'}, {atom_t::CB, 'C'}, {atom_t::CD, 'C'}, {atom_t::CD1, 'C'}, {atom_t::CD2, 'C'}, {atom_t::CE, 'C'}, {atom_t::CE1, 'C'}, {atom_t::CE2, 'C'},
	{atom_t::CE3, 'C'}, {atom_t::CG, 'C'}, {atom_t::CG1, 'C'}, {atom_t::CG2, 'C'}, {atom_t::CH2, 'C'}, {atom_t::CZ, 'C'}, {atom_t::CZ2, 'C'}, {atom_t::CZ3, 'C'},
	{atom_t::O, 'O'}, {atom_t::OD1, 'O'}, {atom_t::OD2, 'O'}, {atom_t::OE1, 'O'}, {atom_t::OE2, 'O'}, {atom_t::OG, 'O'}, {atom_t::OG1, 'O'}, {atom_t::OH, 'O'}, {atom_t::OXT, 'O'},
	{atom_t::N, 'N'}, {atom_t::ND1, 'N'}, {atom_t::ND2, 'N'}, {atom_t::NE, 'N'}, {atom_t::NE1, 'N'}, {atom_t::NE2, 'N'}, {atom_t::NH1, 'N'}, {atom_t::NH2, 'N'}, {atom_t::NZ, 'N'},
	{atom_t::SD, 'S'}, {atom_t::SE, 'S'}, {atom_t::SG, 'S'} };

// *****************************************************************
constexpr bool is_backbone_atom(atom_t atom)
{
	return atom == atom_t::N || atom == atom_t::C || atom == atom_t::CA;
}

// *****************************************************************
constexpr bool is_main_chain_atom(atom_t atom)
{
	return atom == atom_t::N || atom == atom_t::C || atom == atom_t::CA || atom == atom_t::O;
}

// *****************************************************************
constexpr bool is_prev_aa_atom(atom_t atom)
{
	return atom == atom_t::_N || atom == atom_t::_C || atom == atom_t::_CA || atom == atom_t::_O;
}

// *****************************************************************
constexpr atom_t atom_conv_to_prev(const atom_t atom)
{
	if (atom == atom_t::N)
		return atom_t::_N;
	else if (atom == atom_t::O)
		return atom_t::_O;
	else if (atom == atom_t::C)
		return atom_t::_C;
	else if (atom == atom_t::CA)
		return atom_t::_CA;

	return atom_t::unknown;
}

// *****************************************************************
constexpr atom_t atom_conv_from_prev(const atom_t atom)
{
	if (atom == atom_t::_N)
		return atom_t::N;
	else if (atom == atom_t::_O)
		return atom_t::O;
	else if (atom == atom_t::_C)
		return atom_t::C;
	else if (atom == atom_t::_CA)
		return atom_t::CA;

	return atom_t::unknown;
}

// *****************************************************************
constexpr char aa_type_char(aa_t type)
{
	switch (type)
	{
	case aa_t::unknown: return ' ';
	case aa_t::ARG: return 'R';
	case aa_t::HIS: return 'H';
	case aa_t::LYS: return 'K';
	case aa_t::ASP: return 'D';
	case aa_t::GLU: return 'E';
	case aa_t::SER: return 'S';
	case aa_t::THR: return 'T';
	case aa_t::ASN: return 'N';
	case aa_t::GLN: return 'Q';
	case aa_t::CYS: return 'C';
	case aa_t::SEC: return 'U';
	case aa_t::GLY: return 'G';
	case aa_t::PRO: return 'P';
	case aa_t::ALA: return 'A';
	case aa_t::VAL: return 'V';
	case aa_t::ILE: return 'I';
	case aa_t::LEU: return 'L';
	case aa_t::MET: return 'M';
	case aa_t::PHE: return 'F';
	case aa_t::TYR: return 'Y';
	case aa_t::TRP: return 'W';
	}

	return ' ';
}

// *****************************************************************
const unordered_map<aa_t, unordered_set<atom_t>> m_valid_aa = {
	{aa_t::unknown, {}},
	{aa_t::ARG, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::NE, atom_t::NH1, atom_t::NH2, atom_t::CZ, atom_t::OXT }},
	{aa_t::HIS, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD2, atom_t::ND1, atom_t::CE1, atom_t::NE2, atom_t::OXT }},
	{aa_t::LYS, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::CE, atom_t::NZ, atom_t::OXT }},
	{aa_t::ASP, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::OD1, atom_t::OD2, atom_t::OXT }},
	{aa_t::GLU, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::OE1, atom_t::OE2, atom_t::OXT }},
	{aa_t::SER, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::OG, atom_t::OXT }},
	{aa_t::THR, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG2, atom_t::OG1, atom_t::OXT }},
	{aa_t::ASN, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::ND2, atom_t::OD1, atom_t::OXT }},
	{aa_t::GLN, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::NE2, atom_t::OE1, atom_t::OXT }},
	{aa_t::CYS, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::SG, atom_t::OXT }},
	{aa_t::SEC, { atom_t::N, atom_t::CA, atom_t::CB, atom_t::SE, atom_t::C, atom_t::O, atom_t::OXT }},			// Na podstawie https://www.wwpdb.org/pdb?id=pdb_00001cc1
	{aa_t::GLY, { atom_t::N, atom_t::CA, atom_t::C, atom_t::O, atom_t::OXT }},
	{aa_t::PRO, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::OXT }},
	{aa_t::ALA, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::OXT }},
	{aa_t::VAL, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG1, atom_t::CG2, atom_t::OXT }},
	{aa_t::ILE, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG1, atom_t::CG2, atom_t::CD1, atom_t::OXT }},
	{aa_t::LEU, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::OXT }},
	{aa_t::MET, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::SD, atom_t::CE, atom_t::OXT }},
	{aa_t::PHE, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE1, atom_t::CE2, atom_t::CZ, atom_t::OXT }},
	{aa_t::TYR, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE1, atom_t::CE2, atom_t::OH, atom_t::CZ, atom_t::OXT }},
	{aa_t::TRP, { atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE2, atom_t::CE3, atom_t::NE1, atom_t::CH2, atom_t::CZ2, atom_t::CZ3, atom_t::OXT }} };

// *****************************************************************
const vector<unordered_set<atom_t>> v_valid_aa = {
	{},		// aa_t::unknown,
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::NE, atom_t::NH1, atom_t::NH2, atom_t::CZ, atom_t::OXT },		// aa_t::ARG
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD2, atom_t::ND1, atom_t::CE1, atom_t::NE2, atom_t::OXT },				// aa_t::HIS
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::CE, atom_t::NZ, atom_t::OXT },								// aa_t::LYS
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::OD1, atom_t::OD2, atom_t::OXT },											// aa_t::ASP
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::OE1, atom_t::OE2, atom_t::OXT },								// aa_t::GLU
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::OG, atom_t::OXT },																	// aa_t::SER
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG2, atom_t::OG1, atom_t::OXT },														// aa_t::THR
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::ND2, atom_t::OD1, atom_t::OXT },											// aa_t::ASN
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::NE2, atom_t::OE1, atom_t::OXT },								// aa_t::GLN
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::SG, atom_t::OXT },																	// aa_t::CYS
	{atom_t::N, atom_t::CA, atom_t::CB, atom_t::SE, atom_t::C, atom_t::O, atom_t::OXT },			// Na podstawie https://www.wwpdb.org/pdb?id=pdb_00001cc1		// aa_t::SEC
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::O, atom_t::OXT },																							// aa_t::GLY
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD, atom_t::OXT },														// aa_t::PRO
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::OXT },																				// aa_t::ALA
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG1, atom_t::CG2, atom_t::OXT },														// aa_t::VAL
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG1, atom_t::CG2, atom_t::CD1, atom_t::OXT },											// aa_t::ILE
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::OXT },											// aa_t::LEU
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::SD, atom_t::CE, atom_t::OXT },											// aa_t::MET
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE1, atom_t::CE2, atom_t::CZ, atom_t::OXT },	// aa_t::PHE
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE1, atom_t::CE2, atom_t::OH, atom_t::CZ, atom_t::OXT },		// aa_t::TYR
	{atom_t::N, atom_t::CA, atom_t::C, atom_t::CB, atom_t::O, atom_t::CG, atom_t::CD1, atom_t::CD2, atom_t::CE2, atom_t::CE3, atom_t::NE1, atom_t::CH2, atom_t::CZ2, atom_t::CZ3, atom_t::OXT}		// aa_t::TRP
};

// *****************************************************************
const unordered_map<string, aa_t> m_aa{
	{"ARG", aa_t::ARG}, {"HIS", aa_t::HIS}, {"LYS", aa_t::LYS}, {"ASP", aa_t::ASP}, {"GLU", aa_t::GLU}, {"SER", aa_t::SER},
	{"THR", aa_t::THR}, {"ASN", aa_t::ASN}, {"GLN", aa_t::GLN}, {"CYS", aa_t::CYS}, {"SEC", aa_t::SEC}, {"GLY", aa_t::GLY},
	{"PRO", aa_t::PRO}, {"ALA", aa_t::ALA}, {"VAL", aa_t::VAL}, {"ILE", aa_t::ILE}, {"LEU", aa_t::LEU}, {"MET", aa_t::MET},
	{"PHE", aa_t::PHE}, {"TYR", aa_t::TYR}, {"TRP", aa_t::TRP} };

// *****************************************************************
const unordered_map<string, atom_t> m_atom{
	{"_N", atom_t::_N}, {"_C", atom_t::_C}, {"_CA", atom_t::_CA}, {"_O", atom_t::_O},
	{"N", atom_t::N}, {"C", atom_t::C}, {"CA", atom_t::CA}, {"O", atom_t::O}, {"CB", atom_t::CB}, {"CG", atom_t::CG},
	{"CG1", atom_t::CG1}, {"CG2", atom_t::CG2}, {"CD", atom_t::CD}, {"CD1", atom_t::CD1}, {"CD2", atom_t::CD2}, {"CE", atom_t::CE},
	{"CE1", atom_t::CE1}, {"CE2", atom_t::CE2}, {"CE3", atom_t::CE3}, {"CZ", atom_t::CZ}, {"CZ2", atom_t::CZ2}, {"CZ3", atom_t::CZ3},
	{"CH2", atom_t::CH2}, {"ND1", atom_t::ND1}, {"ND2", atom_t::ND2}, {"NE", atom_t::NE}, {"NE1", atom_t::NE1}, {"NE2", atom_t::NE2},
	{"NH1", atom_t::NH1}, {"NH2", atom_t::NH2}, {"NZ", atom_t::NZ}, {"OG", atom_t::OG}, {"OG1", atom_t::OG1}, {"OD1", atom_t::OD1},
	{"OD2", atom_t::OD2}, {"OE1", atom_t::OE1}, {"OE2", atom_t::OE2}, {"OH", atom_t::OH}, {"OXT", atom_t::OXT}, {"SG", atom_t::SG},
	{"SD", atom_t::SD}, {"SE", atom_t::SE} };


// *****************************************************************
aa_t str_to_aa(const string& str);
atom_t str_to_atom(const string& str);
string aa_to_str(const aa_t aa);
string atom_to_str(const atom_t atom);
string aa_atom_to_str(const pair<aa_t, atom_t>& aa_atom);

// EOF
