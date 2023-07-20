#pragma once
#include "input.h"

#include <unordered_set>
#include <vector>
#include <utility>
#include <string>

class Pdb : public StructFile {

	struct column_def_t {
		std::string name;
		int start;
		int width;
		bool isNumeric;
	};

	// (id, start, length, is_numeric)
	const std::vector<column_def_t> COLUMN_DEFS {
		{Columns::group_PDB, 0, 6, false},									// critical - ATOM
		{Columns::id, 6, 5, true},											// critical - atom no. (1, 2, 3, ...)
		{Columns::auth_atom_id, 12, 4, false},								// critical - atom label (N, CA, CG1, ...)
		{Columns::label_alt_id, 16, 1, false},
		{Columns::auth_comp_id, 17, 3, false},								// critical - AA name (PHY, HIS, ...)
		{Columns::auth_asym_id, 21, 1, false},								// critical - chain id (A, B, ...)
		{Columns::auth_seq_id, 22, 4, true},								// critical - AA no. (1, 2, 3, ...)
		{Columns::pdbx_PDB_ins_code, 26, 1, false},
		{Columns::Cartn_x, 30, 8, true},									// critical - X coord
		{Columns::Cartn_y, 38, 8, true },									// critical - Y coord
		{Columns::Cartn_z, 46, 8, true },									// critical - Z coord
		{Columns::occupancy, 54, 6, true },								
		{Columns::B_iso_or_equiv, 60, 6, true },							// critical - B-factor (59.86, 61.56, ...)
		{Columns::segment, 72, 4, false },						
		{Columns::type_symbol, 76, 2, false },								// critical - atom symbol (C, N, O, ...)
		{Columns::pdbx_formal_charge, 78, 2, false },
	};

	const std::unordered_set<std::string> MINIMAL_SECTIONS
	{
		"TITLE",
		"ATOM",
		"TER"
	};

	const std::unordered_set<std::string> IGNORED_SECTIONS{
		"SIGATM",
		"ANISOU",
		"SIGUIJ"
	};

public:

	static constexpr const char* ENTRY_ATOM_SITE{ "ATOM" };

	const char* getAtomEntryName() const override { return ENTRY_ATOM_SITE; }

	Pdb(bool minimal_mode) :
		StructFile(minimal_mode)
	{}

	void parse() override;
	size_t store() override;
};