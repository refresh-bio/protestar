#pragma once

#include "struct-compressor.h"
#include "../parsers/pdb.h"

//#define SHOW_CIF_COMPRESSOR_STATS

// *****************************************************************
class PDBCompressor : public StructCompressor
{
    const vector<string> v_words = {
        "TITLE", "ATOM", "TER", "HETATM"
    };

    const vector<string> aa_contents = {
        ""s,                                                                                       // unknown
        " N  \0 CA \0 C  \0 CB \0 O  "s,                                                          // ALA
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD \0 NE \0 NH1\0 NH2\0 CZ "s,                      // ARG
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 ND2\0 OD1"s,                                        // ASN
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 OD1\0 OD2"s,                                        // ASP
        " N  \0 CA \0 C  \0 CB \0 O  \0 SG "s,                                                    // CYS
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD \0 NE2\0 OE1"s,                                  // GLN
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD \0 OE1\0 OE2"s,                                  // GLU
        " N  \0 CA \0 C  \0 O  "s,                                                                // GLY
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD2\0 ND1\0 CE1\0 NE2"s,                            // HIS
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG1\0 CG2\0 CD1"s,                                        // ILE
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD1\0 CD2"s,                                        // LEU
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD \0 CE \0 NZ "s,                                  // LYS
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 SD \0 CE "s,                                        // MET
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD1\0 CD2\0 CE1\0 CE2\0 CZ "s,                      // PHE
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD "s,                                              // PRO
        " N  \0 CA \0 C  \0 CB \0 O  \0 OG "s,                                                    // SER
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG2\0 OG1"s,                                              // THR
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD1\0 CD2\0 CE2\0 CE3\0 NE1\0 CH2\0 CZ2\0 CZ3"s,    // TRP
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG \0 CD1\0 CD2\0 CE1\0 CE2\0 OH \0 CZ "s,                // TYR
        " N  \0 CA \0 C  \0 CB \0 O  \0 CG1\0 CG2"s                                               // VAL
    };

    const vector<string> aa_atoms = {
        " N\0 C\0 C\0 C\0 O"s,                                      // ALA
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 N\0 N\0 N\0 C"s,              // ARG
        " N\0 C\0 C\0 C\0 O\0 C\0 N\0 O"s,                          // ASN
        " N\0 C\0 C\0 C\0 O\0 C\0 O\0 O"s,                          // ASP
        " N\0 C\0 C\0 C\0 O\0 S"s,                                  // CYS
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 N\0 O"s,                      // GLN
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 O\0 O"s,                      // GLU
        " N\0 C\0 C\0 O"s,                                          // GLY
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 N\0 C\0 N"s,                  // HIS
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 C"s,                          // ILE
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 C"s,                          // LEU
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 C\0 N"s,                      // LYS
        " N\0 C\0 C\0 C\0 O\0 C\0 S\0 C"s,                          // MET
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 C\0 C\0 C\0 C"s,              // PHE
        " N\0 C\0 C\0 C\0 O\0 C\0 C"s,                              // PRO
        " N\0 C\0 C\0 C\0 O\0 O"s,                                  // SER
        " N\0 C\0 C\0 C\0 O\0 C\0 O"s,                              // THR
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 C\0 C\0 C\0 N\0 C\0 C\0 C"s,  // TRP
        " N\0 C\0 C\0 C\0 O\0 C\0 C\0 C\0 C\0 C\0 O\0 C"s,          // TYR
        " N\0 C\0 C\0 C\0 O\0 C\0 C"s                               // VAL
    };

public:
	PDBCompressor(bool compression_mode) :
		StructCompressor(compression_mode)
	{
        for (const auto& vec : { aa_atoms, aa_contents, v_atom, v_chem, v_col_names, v_words, v_common_words})
            for (const auto& x : vec)
            {
                dict_append(zstd_dict, x, 0);
            }

        // Pairs of residues
        set<pair<int, int>> pairs_added;
        int prev_id = 0;

        for (int i = 1; i < (int) v_chem.size(); i += 3)
            for (int j = 1; j < (int) v_chem.size(); j += 3)
                if(i != j)
                {
                    if (pairs_added.count(make_pair(i, j)))
                        continue;

                    zstd_dict.insert(zstd_dict.end(), v_chem[i].begin(), v_chem[i].end());
                    zstd_dict.emplace_back(0);
                    zstd_dict.insert(zstd_dict.end(), v_chem[j].begin(), v_chem[j].end());
                    zstd_dict.emplace_back(0);

                    pairs_added.emplace(make_pair(i, j));
                    pairs_added.emplace(make_pair(prev_id, i));
                    prev_id = j;
                }

        zstd_dict_ext = zstd_dict;
    }

	~PDBCompressor() = default;
};

// EOF
