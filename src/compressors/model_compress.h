#pragma once

#include "../common/model.h"

// *****************************************************************
class ModelCompress : public Model
{
	int64_t res_backbone = 1;
	int64_t res_sidechain = 1;
	bool max_compression = false;

	vector<array<atom_t, 3>> reference_atoms;
	vector<vector<dist6_t>> centroids;
	vector<int> prediction_precision;

	aa_desc_t aa_prev;
	aa_desc_t aa_curr;

	unordered_map<atom_t, atom_short_desc_t> atoms_prev;
	unordered_map<atom_t, atom_short_desc_t> atoms_curr;

	int64_t res_backbone_cleaned = 0;
	int64_t res_sidechain_cleaned = 0;
	set<int_coords_t> q_dict;

	int select_best_centroid(const vector<dist6_t> &curr_centroid, const dist6_t &dist);

	// Returns squared differences (see dist_diff)
	int64_t dist_diff2(const dist6_t& a, const dist6_t& b)
	{
		int64_t d0 = a[0] - b[0];
		int64_t d1 = a[1] - b[1];
		int64_t d2 = a[2] - b[2];
		int64_t d3 = a[3] - b[3];
		int64_t d4 = a[4] - b[4];
		int64_t d5 = a[5] - b[5];

		return d0 * d0 + d1 * d1 + d2 * d2 + d3 * d3 + d4 * d4 + d5 * d5;
	}

	// Calculates differences between two tetrahedrons (described as 6 edge distances)
	int64_t dist_diff(const dist6_t& a, const dist6_t& b)
	{
		return (int64_t)sqrt(dist_diff2(a, b));
	}

	bool calc_curr_dist(const array<atom_t, 3> &ref_atoms, int atom_pos, array<int_coords_t, 3>& ref_coords, dist6_t &curr_d);
	bool calc_curr_dist(const array<atom_t, 3> &ref_atoms, atom_t atom, array<int_coords_t, 3>& ref_coords, dist3_t &curr_d);

	void calc_deltas(const int_coords_t &curr_atom_coords, const double_coords_t* rc, const vector<dist6_t>& curr_centroids, int centroid_id, int64_t precision, bool& first, int64_t& dx, int64_t& dy, int64_t& dz);

	bool check_in_dict(const int_coords_t& ic);
	void add_to_dict(const int_coords_t& ic);
	bool need_run_clean();

public:
	ModelCompress() : Model()
	{
		init_centroids();
		init_reference_atoms();
		init_prediction_precision();
	}

	~ModelCompress() = default;

	void init_centroids();
	void init_reference_atoms();
	void init_prediction_precision();

	void restart_chain();

	// Tells the model that we switch to new AA
	void new_aa(const aa_desc_t& aa_desc)
	{
		swap(aa_prev, aa_curr);
		aa_curr = aa_desc;

		swap(atoms_prev, atoms_curr);
		atoms_curr.clear();
	}

	void start_aa(aa_t aa)
	{
		swap(aa_prev, aa_curr);
		aa_curr.type = aa;
		aa_curr.atoms.clear();

		swap(atoms_prev, atoms_curr);
		atoms_curr.clear();
	}

	aa_desc_t get_aa();
	void set_coords(const atom_t atom, const int_coords_t& coords);

	size_t get_no_centroids(const aa_t aa, const atom_t atom);

	uint32_t calc_ctx(const aa_t aa, const atom_t atom);
	uint32_t get_max_ctx();

	void set_resolution(int64_t _res_backbone, int64_t _res_sidechain)
	{
		res_backbone = _res_backbone;
		res_sidechain = _res_sidechain;
	}

	void set_max_compression(bool _max_compression)
	{
		max_compression = _max_compression;
	}

	int64_t round_coord(int64_t val, int64_t resolution)
	{
		if(val >= 0)
			return (val + resolution / 2) / resolution * resolution;
		else
			return (val - resolution / 2) / resolution * resolution;
	}

	int64_t round_coord(double val, int64_t resolution)
	{
		return round_coord((int64_t)val, resolution);
	}

	int64_t reduce_coord(int64_t val, int64_t resolution)
	{
		if(val >= 0)
			return (val + resolution / 2) / resolution;
		else
			return (val - resolution / 2) / resolution;
	}

	int64_t reduce_coord(double val, int64_t resolution)
	{
		return reduce_coord((int64_t)val, resolution);
	}

	void clean_model();

	bool predict(int atom_pos, int &centroid_id, int &no_centroids, bool &first, int64_t&dx, int64_t&dy, int64_t&dz);
	bool decode(atom_t atom, int centroid_id, bool first, int64_t dx, int64_t dy, int64_t dz, int_coords_t& dc);
};

// EOF
