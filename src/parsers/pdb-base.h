#pragma once

#include <unordered_set>
#include <vector>
#include <utility>
#include <string>

#include "../compressors/struct-base.h"

class PdbBase {
	public:
	struct column_def_t {
		std::string name;
		int start;
		int width;
		bool isNumeric;
	};

	// (id, start, length, is_numeric)
	const std::vector<column_def_t> COLUMN_DEFS{
		{StructFileBase::Columns::group_PDB, 0, 6, false},									// critical - ATOM
		{StructFileBase::Columns::id, 6, 5, true},											// critical - atom no. (1, 2, 3, ...)
		{StructFileBase::Columns::auth_atom_id, 12, 4, false},								// critical - atom label (N, CA, CG1, ...)
		{StructFileBase::Columns::label_alt_id, 16, 1, false},
		{StructFileBase::Columns::auth_comp_id, 17, 3, false},								// critical - AA name (PHY, HIS, ...)
		{StructFileBase::Columns::auth_asym_id, 21, 1, false},								// critical - chain id (A, B, ...)
		{StructFileBase::Columns::auth_seq_id, 22, 4, true},								// critical - AA no. (1, 2, 3, ...)
		{StructFileBase::Columns::pdbx_PDB_ins_code, 26, 1, false},
		{StructFileBase::Columns::Cartn_x, 30, 8, true},									// critical - X coord
		{StructFileBase::Columns::Cartn_y, 38, 8, true },									// critical - Y coord
		{StructFileBase::Columns::Cartn_z, 46, 8, true },									// critical - Z coord
		{StructFileBase::Columns::occupancy, 54, 6, true },
		{StructFileBase::Columns::B_iso_or_equiv, 60, 6, true },							// critical - B-factor (59.86, 61.56, ...)
		{StructFileBase::Columns::segment, 72, 4, false },
		{StructFileBase::Columns::type_symbol, 76, 2, false },								// critical - atom symbol (C, N, O, ...)
		{StructFileBase::Columns::pdbx_formal_charge, 78, 2, false },
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

	PdbBase() = default;
};