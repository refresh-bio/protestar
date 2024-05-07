#include "atom_extractor.h"
#include "../core/utils.h"
#include "../parsers/input-base.h"
#include <limits>

// *****************************************************************
int64_t coords_distance2(const int_coords_t& a, const int_coords_t& b)
{
    int64_t dx = a.x - b.x;
    int64_t dy = a.y - b.y;
    int64_t dz = a.z - b.z;

    return dx * dx + dy * dy + dz * dz;
}

// *****************************************************************
int coords_distance(const int_coords_t& a, const int_coords_t& b)
{
    return (int)sqrt(coords_distance2(a, b));
}

// *****************************************************************
double est_delta_coding_cost(const int_coords_t& a, const int_coords_t& b)
{
    int64_t dx = abs(a.x - b.x);
    int64_t dy = abs(a.y - b.y);
    int64_t dz = abs(a.z - b.z);

    return log2(1 + dx) + log2(1 + dy) + log2(1 + dz);
}

// *****************************************************************
bool Protein::parse_chains(const LoopEntry *entry)
{
    v_chains.clear();

    auto col_group_PDB = entry->findColumn(StructFileBase::Columns::group_PDB);
    auto col_id = entry->findColumn(StructFileBase::Columns::id);
    auto col_auth_atom_id = entry->findColumn(StructFileBase::Columns::auth_atom_id);
    auto col_auth_comp_id = entry->findColumn(StructFileBase::Columns::auth_comp_id);
    auto col_auth_asym_id = entry->findColumn(StructFileBase::Columns::auth_asym_id);
    auto col_auth_seq_id = entry->findColumn(StructFileBase::Columns::auth_seq_id);
    auto col_Cartn_x = entry->findColumn(StructFileBase::Columns::Cartn_x);
    auto col_Cartn_y = entry->findColumn(StructFileBase::Columns::Cartn_y);
    auto col_Cartn_z = entry->findColumn(StructFileBase::Columns::Cartn_z);
    auto col_B_factor = entry->findColumn(StructFileBase::Columns::B_iso_or_equiv);
    auto col_type_symbol = entry->findColumn(StructFileBase::Columns::type_symbol);

    if (!col_group_PDB || !col_id || !col_auth_atom_id || !col_auth_comp_id || !col_auth_asym_id || !col_auth_seq_id || !col_Cartn_x || !col_Cartn_y || !col_Cartn_z || !col_B_factor || !col_type_symbol)
        return false;

    auto group_PDB = dynamic_cast<const StringColumn*>(col_group_PDB)->getValues();
    auto id = dynamic_cast<const NumericColumn*>(col_id)->getValues();
    auto auth_atom_id = dynamic_cast<const StringColumn*>(col_auth_atom_id)->getValues();
    auto auth_comp_id = dynamic_cast<const StringColumn*>(col_auth_comp_id)->getValues();
    auto auth_asym_id = dynamic_cast<const StringColumn*>(col_auth_asym_id)->getValues();
    auto auth_seq_id = dynamic_cast<const NumericColumn*>(col_auth_seq_id)->getValues();
    auto Cartn_x = dynamic_cast<const NumericColumn*>(col_Cartn_x)->getValues();
    auto Cartn_y = dynamic_cast<const NumericColumn*>(col_Cartn_y)->getValues();
    auto Cartn_z = dynamic_cast<const NumericColumn*>(col_Cartn_z)->getValues();
    auto B_factor = dynamic_cast<const NumericColumn*>(col_B_factor)->getValues();
    auto type_symbol = dynamic_cast<const StringColumn*>(col_type_symbol)->getValues();

    cart_precision = dynamic_cast<const NumericColumn*>(col_Cartn_x)->numDecimals;

    string asym_id = "";
    int seq_id = numeric_limits<int>::min();
    aa_t curr_aa = aa_t::unknown;

    for (size_t i = 0; i < group_PDB.size(); ++i)
    {
        if (auth_asym_id[i] != asym_id)
        {
            asym_id = auth_asym_id[i];
            v_chains.emplace_back(asym_id.front());
            seq_id = numeric_limits<int>::min();

            // Count no. of AA in chain
            size_t j;
            for (j = i + 1; j < group_PDB.size(); ++j)
                if (auth_asym_id[j] != asym_id)
                    break;
            if(auth_seq_id[j - 1] > 0)
                v_chains.back().aa.reserve(auth_seq_id[j - 1]);
        }

        if (auth_seq_id[i] != seq_id)
        {
            auto aci = trim_to_str(auth_comp_id[i]);

            curr_aa = str_to_aa(aci);
            seq_id = auth_seq_id[i];
            v_chains.back().aa.push_back(aa_desc_t(curr_aa, seq_id, {}, {}, aci));
            v_chains.back().aa.back().atoms.reserve(v_valid_aa[(int) curr_aa].size());
        }

        auto ati = trim_to_str(auth_atom_id[i]);

        atom_t atom_type = str_to_atom(ati);
        if (v_valid_aa[(int) curr_aa].count(atom_type) == 0)
            atom_type = atom_t::unknown;

        auto ts = trim_to_str(type_symbol[i]);

        v_chains.back().aa.back().atoms.emplace_back(id[i], atom_type, int_coords_t{ Cartn_x[i], Cartn_y[i], Cartn_z[i] }, B_factor[i], ts.front(), ati);
    }

    check_typicality();

    return true;
}

// *****************************************************************
size_t Protein::no_chains()
{
    return v_chains.size();
}

// *****************************************************************
vector<chain_desc_t>& Protein::get_chains()
{
    return v_chains;
}

// *****************************************************************
chain_desc_t& Protein::get_chain(int id)
{
    return v_chains[id];
}

// *****************************************************************
void Protein::expand_decimals(int requested_precision)
{
    int64_t factor = pow10<int64_t>(cart_working_precision - cart_precision);

    for(auto &chain : v_chains)
        for (auto& aa : chain.aa)
            for(auto &atom : aa.atoms)
            {
                atom.coords.x *= factor;
                atom.coords.y *= factor;
                atom.coords.z *= factor;
            }
}

// *****************************************************************
void Protein::check_typicality()
{
    for (auto& chain : v_chains)
    {
        chain.typical.set_all();

        for(size_t i = 0; i < chain.aa.size(); ++i)
        {
            auto& aa = chain.aa[i];

            aa.typical.set(typicality_t::all_mask);

            if (aa.type == aa_t::unknown)
            {
                aa.typical.reset_all();
                chain.typical.reset_all();
                continue;
            }

            const auto& atoms_typical = m_aa_atom.at(aa.type);
            bool oxt_needed = i + 1 == chain.aa.size();

            if (oxt_needed)
            {
                if (aa.atoms.size() == atoms_typical.size())
                    ;
                else if(aa.atoms.size() + 1 == atoms_typical.size())
                {
                    aa.typical.reset(typicality_t::oxt_mask);
                    chain.typical.reset(typicality_t::oxt_mask);
                }
                else
                {
                    aa.typical.reset(typicality_t::contents_mask);
                    chain.typical.reset(typicality_t::contents_mask);
                    continue;
                }
            }
            else
            {
                if (aa.atoms.size() + 1 != atoms_typical.size())
                {
                    aa.typical.reset(typicality_t::contents_mask);
                    chain.typical.reset(typicality_t::contents_mask);
                    continue;
                }
            }

            int bf = aa.atoms.front().B_factor;
            int id = aa.atoms.front().id;

            for (int i = 0; i < (int) aa.atoms.size(); ++i)
            {
                if (aa.atoms[i].type == atom_t::unknown)
                {
                    aa.typical.reset(typicality_t::contents_mask);
                    chain.typical.reset(typicality_t::contents_mask);
                    continue;
                }

                if (aa.atoms[i].type != atoms_typical[i])
                    aa.typical.reset(typicality_t::ao_mask);
                if (aa.atoms[i].id != id + i)
                    aa.typical.reset(typicality_t::id_mask);
                if (aa.atoms[i].B_factor != bf)
                    aa.typical.reset(typicality_t::bf_mask);
                if (aa.atoms[i].type_symbol != atom_type_char.at(aa.atoms[i].type))
                    aa.typical.reset(typicality_t::ts_mask);
            }

            chain.typical &= aa.typical;
        }
    }
}

// EOF
