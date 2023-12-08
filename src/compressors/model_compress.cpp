// add --type pdb --infile AF-A0A385XJ53-F1-model_v4.pdb -t 1 --out aa.psa --lossy --max-error-bb 500 --max-error-sc 500 --minimal
// add --type pdb --infile AF-A0A385XJ53-F1-model_v4.pdb -t 1 --out aa.psa --lossy --max-error-bb 50 --max-error-sc 50 --minimal
// get --type pdb --file AF-A0A385XJ53-F1-model_v4 -t 16 --in aa.psa --outdir out/
// get --type pdb --file AF-A0A385XJ53-F1-model_v4 -t 16 --in aa.psa --outdir out/
// get --type pdb --file AF-A0A385XJ53-F1-model_v4 -t 16 --in ecoli.psa_500_500 --outdir out/


#include "model_compress.h"

#include "../common/init_model.hpp"
#include "../common/tetrahedron.h"

#include <iostream>
#include <set>
#include <numeric>

// *****************************************************************
// Restarts the model when new chain starts
void ModelCompress::restart_chain()
{
	aa_curr.atoms.clear();
	aa_prev.atoms.clear();

	atoms_prev.clear();
	atoms_curr.clear();
}

// *****************************************************************
// Takes the distances for the current atom and looks for the most similar tetrahedron in centroids
int ModelCompress::select_best_centroid(const vector<dist6_t>& curr_centroid, const dist6_t& dist)
{
	int64_t best_d2 = dist_diff2(curr_centroid[0], dist);
	int best_id = 0;

	for (int i = 1; i < (int) curr_centroid.size(); ++i)
	{
		int64_t curr_dist2 = dist_diff2(curr_centroid[i], dist);
		if (curr_dist2 < best_d2)
		{
			best_d2 = curr_dist2;
			best_id = i;
		}
	}

	return best_id;
}

// *****************************************************************
size_t ModelCompress::get_no_centroids(const aa_t aa, const atom_t atom)
{
	uint32_t ctx = packed_atom_ctx(aa, atom);

	return centroids[ctx].size();
}

// *****************************************************************
uint32_t ModelCompress::calc_ctx(const aa_t aa, const atom_t atom)
{
	return prediction_precision[packed_atom_ctx(aa, atom)] / 3;

//	return packed_atom_ctx(aa, atom);
}

// *****************************************************************
uint32_t ModelCompress::get_max_ctx()
{
	return max_packed_atom_ctx;
}

// *****************************************************************
// Calculates distances between the current atom and the references
// Returns false if calculation is not possible
bool ModelCompress::calc_curr_dist(const array<atom_t, 3>& ref_atoms, int atom_pos, array<int_coords_t, 3>& ref_coords, dist6_t& curr_d)
{
	unordered_map<atom_t, atom_short_desc_t>::iterator ref_iters[3];

	for (int i = 0; i < 3; ++i)
	{
		atom_t ra = ref_atoms[i];

		if (is_prev_aa_atom(ra))
		{
			ref_iters[i] = atoms_prev.find(atom_conv_from_prev(ra));
			if (ref_iters[i] == atoms_prev.end())
				return false;
		}
		else
		{
			ref_iters[i] = atoms_curr.find(ra);
			if (ref_iters[i] == atoms_curr.end())
				return false;
		}
	}

	const int_coords_t& curr_coords = aa_curr.atoms[atom_pos].coords;

	ref_coords[0] = ref_iters[0]->second.coords;
	ref_coords[1] = ref_iters[1]->second.coords;
	ref_coords[2] = ref_iters[2]->second.coords;

	curr_d[0] = coords_distance(ref_iters[0]->second.coords, ref_iters[1]->second.coords);
	curr_d[1] = coords_distance(ref_iters[0]->second.coords, ref_iters[2]->second.coords);
	curr_d[2] = coords_distance(ref_iters[1]->second.coords, ref_iters[2]->second.coords);
	curr_d[3] = coords_distance(ref_iters[0]->second.coords, curr_coords);
	curr_d[4] = coords_distance(ref_iters[1]->second.coords, curr_coords);
	curr_d[5] = coords_distance(ref_iters[2]->second.coords, curr_coords);

	return true;
}

// *****************************************************************
// Calculates distances between the current atom and the references
// Returns false if calculation is not possible
bool ModelCompress::calc_curr_dist(const array<atom_t, 3>& ref_atoms, atom_t atom, array<int_coords_t, 3>& ref_coords, dist3_t& curr_d)
{
	unordered_map<atom_t, atom_short_desc_t>::iterator ref_iters[3];

	for (int i = 0; i < 3; ++i)
	{
		atom_t ra = ref_atoms[i];

		if (is_prev_aa_atom(ra))
		{
			ref_iters[i] = atoms_prev.find(atom_conv_from_prev(ra));
			if (ref_iters[i] == atoms_prev.end())
				return false;
		}
		else
		{
			ref_iters[i] = atoms_curr.find(ra);
			if (ref_iters[i] == atoms_curr.end())
				return false;
		}
	}

	ref_coords[0] = ref_iters[0]->second.coords;
	ref_coords[1] = ref_iters[1]->second.coords;
	ref_coords[2] = ref_iters[2]->second.coords;

	curr_d[0] = coords_distance(ref_iters[0]->second.coords, ref_iters[1]->second.coords);
	curr_d[1] = coords_distance(ref_iters[0]->second.coords, ref_iters[2]->second.coords);
	curr_d[2] = coords_distance(ref_iters[1]->second.coords, ref_iters[2]->second.coords);

	return true;
}

// *****************************************************************
void ModelCompress::calc_deltas(const int_coords_t& curr_atom_coords, const double_coords_t* rc, const vector<dist6_t>& curr_centroids, int centroid_id, int64_t precision, 
	bool &first, bool& equal_q, int64_t& dx, int64_t &dy, int64_t& dz)
{
	tetrahedron_t th(rc[0], rc[1], rc[2], curr_centroids[centroid_id][3], curr_centroids[centroid_id][4], curr_centroids[centroid_id][5]);

	auto q_both = th.get_q_both();

	int_coords_t qc[2] = {
		{round_coord(q_both.first.x, precision), round_coord(q_both.first.y, precision), round_coord(q_both.first.z, precision)},
		{round_coord(q_both.second.x, precision), round_coord(q_both.second.y, precision), round_coord(q_both.second.z, precision)} };

	if (coords_distance2(qc[0], curr_atom_coords) < coords_distance2(qc[1], curr_atom_coords))
//	if (est_delta_coding_cost(qc[0], curr_atom_coords) <= est_delta_coding_cost(qc[1], curr_atom_coords))
	{
		first = true;
		dx = qc[0].x - curr_atom_coords.x;
		dy = qc[0].y - curr_atom_coords.y;
		dz = qc[0].z - curr_atom_coords.z;
	}
	else
	{
		first = false;
		dx = qc[1].x - curr_atom_coords.x;
		dy = qc[1].y - curr_atom_coords.y;
		dz = qc[1].z - curr_atom_coords.z;
	}

	equal_q = qc[0].x == qc[1].x && qc[0].y == qc[1].y && qc[0].z == qc[1].z;

	dx /= precision;
	dy /= precision;
	dz /= precision;
}

// *****************************************************************
// Find centroid that minimizes the cost of coding
// In max_compresion mode requires prediction for many candidates
// In default mode just picks the most similar tetrahedron
bool ModelCompress::predict(int atom_pos, int& centroid_id, int& no_centroids, bool& first, bool &equal_q, int64_t& dx, int64_t& dy, int64_t& dz)
{
	if (aa_curr.type == aa_t::unknown || aa_curr.atoms[atom_pos].type == atom_t::unknown)
		return false;

	uint32_t ctx = packed_atom_ctx(aa_curr.type, aa_curr.atoms[atom_pos].type);

	const auto& curr_centroids = centroids[ctx];
	const auto& curr_centroid_counts = centroid_counts[ctx];
	const auto& curr_references = reference_atoms[ctx];
	array<int_coords_t, 3> ref_coords;

	no_centroids = (int) curr_centroids.size();

	dist6_t curr_dist;

	bool dist_found = calc_curr_dist(curr_references, atom_pos, ref_coords, curr_dist);

	atoms_curr.emplace(aa_curr.atoms[atom_pos].type, aa_curr.atoms[atom_pos].short_desc());

	if (!dist_found)
		return false;

	double_coords_t rc[3] = {
		{ (double)ref_coords[0].x, (double)ref_coords[0].y, (double)ref_coords[0].z },
		{ (double)ref_coords[1].x, (double)ref_coords[1].y, (double)ref_coords[1].z },
		{ (double)ref_coords[2].x, (double)ref_coords[2].y, (double)ref_coords[2].z } };

	auto precision = is_backbone_atom(aa_curr.atoms[atom_pos].type) ? res_backbone : res_sidechain;
	int_coords_t& curr_atom_coords = aa_curr.atoms[atom_pos].coords;

	if (max_compression)
	{
		double best_cost = numeric_limits<double>::max();
		int best_centroid_id = -1;
		int64_t best_dx = 0;
		int64_t best_dy = 0;
		int64_t best_dz = 0;
		bool best_first = true;
		bool best_equal_q = true;

		auto sum_counts = accumulate(curr_centroid_counts.begin(), curr_centroid_counts.end(), 0u);

		for (int i = 0; i < no_centroids; ++i)
		{
			calc_deltas(curr_atom_coords, rc, curr_centroids, i, precision, first, equal_q, dx, dy, dz);

			double cost = log2(1 + abs(dx)) + log2(1 + abs(dy)) + log2(1 + abs(dz)) - log2(curr_centroid_counts[i] / (double) sum_counts);

			if (cost < best_cost)
			{
				best_cost = cost;
				best_centroid_id = i;
				best_first = first;
				best_equal_q = equal_q;
				best_dx = dx;
				best_dy = dy;
				best_dz = dz;
			}
		}

		centroid_id = best_centroid_id;
		first = best_first;
		equal_q = best_equal_q;
		dx = best_dx;
		dy = best_dy;
		dz = best_dz;
	}
	else
	{
		centroid_id = select_best_centroid(curr_centroids, curr_dist);				// Find most similar tetrahedron
		calc_deltas(curr_atom_coords, rc, curr_centroids, centroid_id, precision, first, equal_q, dx, dy, dz);
	}

	return true;
}

// *****************************************************************
aa_desc_t ModelCompress::get_aa()
{
	return aa_curr;
}

// *****************************************************************
void ModelCompress::set_coords(const atom_t atom, const int_coords_t& coords)
{
	aa_curr.atoms.emplace_back(0, atom, coords, 0, ' ');

	atoms_curr[atom].coords = coords;
}

// *****************************************************************
bool ModelCompress::decode_part1(atom_t atom, int centroid_id, pair<int_coords_t, int_coords_t> &q_both)
{
	uint32_t ctx = packed_atom_ctx(aa_curr.type, atom);

	const auto& curr_centroids = centroids[ctx];
	const auto& curr_references = reference_atoms[ctx];
	array<int_coords_t, 3> ref_coords;

	dist3_t curr_dist;

	bool dist_found = calc_curr_dist(curr_references, atom, ref_coords, curr_dist);

	if (!dist_found)
		return false;

	double_coords_t rc[3] = {
		{ (double) ref_coords[0].x, (double)ref_coords[0].y, (double)ref_coords[0].z },
		{ (double)ref_coords[1].x, (double)ref_coords[1].y, (double)ref_coords[1].z },
		{ (double)ref_coords[2].x, (double)ref_coords[2].y, (double)ref_coords[2].z } };

	tetrahedron_t th(rc[0], rc[1], rc[2], curr_centroids[centroid_id][3], curr_centroids[centroid_id][4], curr_centroids[centroid_id][5]);

	auto q = th.get_q_both();

	auto precision = is_backbone_atom(atom) ? res_backbone : res_sidechain;

	q_both.first.x = round_coord(q.first.x, precision);
	q_both.first.y = round_coord(q.first.y, precision);
	q_both.first.z = round_coord(q.first.z, precision);

	q_both.second.x = round_coord(q.second.x, precision);
	q_both.second.y = round_coord(q.second.y, precision);
	q_both.second.z = round_coord(q.second.z, precision);

	return true;
}

// *****************************************************************
void ModelCompress::decode_part2(atom_t atom, const int_coords_t& q, int64_t dx, int64_t dy, int64_t dz, int_coords_t& dc)
{
	auto precision = is_backbone_atom(atom) ? res_backbone : res_sidechain;

	dc.x = q.x - dx * precision;
	dc.y = q.y - dy * precision;
	dc.z = q.z - dz * precision;

	aa_curr.atoms.emplace_back(0, atom, dc, 0, ' ');
	atoms_curr[atom].coords = dc;
}

// *****************************************************************
int ModelCompress::check_in_dict(const int_coords_t& ic)
{
	auto p = q_dict.find(ic);

	if (p == q_dict.end())
		return -1;
	else
		return p->second;
}

// *****************************************************************
void ModelCompress::add_to_dict(const int_coords_t& ic, int id)
{
	int max_dif = 2;
	int max_dist = 4;

	for(int dx = -max_dif; dx <= max_dif; ++dx)
		for(int dy = -max_dif; dy <= max_dif; ++dy)
			for (int dz = -max_dif; dz <= max_dif; ++dz)
				if (abs(dx) + abs(dy) + abs(dz) <= max_dist)
					q_dict.emplace(int_coords_t{ ic.x + dx, ic.y + dy, ic.z + dx }, id);
}

// *****************************************************************
bool ModelCompress::need_run_clean()
{
	return res_backbone != res_backbone_cleaned || res_sidechain != res_sidechain_cleaned;		
}

// *****************************************************************
bool ModelCompress::clean_model()
{
	if (!need_run_clean())
		return false;

	bool was_cleaned = false;

	res_backbone_cleaned = res_backbone;
	res_sidechain_cleaned = res_sidechain;

	centroids.clear();
	init_centroids();
	init_centroid_counts();

	vector<dist6_t> filtered_centroids;
	vector<uint32_t> filtered_centroid_counts;

	for (uint32_t i = 0; i < (uint32_t) centroids.size(); ++i)
	{
		int64_t res = is_backbone_atom(unpack_atom_ctx(i).second) ? res_backbone : res_sidechain;

		filtered_centroids.clear();
		filtered_centroid_counts.clear();

		auto& centr = centroids[i];
		auto& centr_counts = centroid_counts[i];

		if (centr.size() <= 1)
			continue;

		q_dict.clear();

		for (uint32_t j = 0; j < centr.size(); ++j)
		{
			tetrahedron_t tetr(centr[j][0], centr[j][1], centr[j][2], centr[j][3], centr[j][4], centr[j][5]);
			
			auto pred = tetr.get_q_both();

			int_coords_t q1{ reduce_coord(pred.first.x, res), reduce_coord(pred.first.y, res), reduce_coord(pred.first.z, res) };
			int_coords_t q2{ reduce_coord(pred.second.x, res), reduce_coord(pred.second.y, res), reduce_coord(pred.second.z, res) };

			auto q1_id = check_in_dict(q1);
			auto q2_id = check_in_dict(q2);
//			auto q_id = max(q1_id, q2_id);	
			auto q_id = -1;	

			if (q1_id == q2_id)
				q_id = q1_id;

			if (q_id >= 0)
			{
				// Merging centroids
				auto sum_counts = filtered_centroid_counts[q_id] + centr_counts[j];

				int64_t w1 = filtered_centroid_counts[q_id];
				int64_t w2 = centr_counts[j];

				for (int k = 0; k < 6; ++k)
					filtered_centroids[q_id][k] = (int)((filtered_centroids[q_id][k] * w1 + centr[j][k] * w2 + sum_counts / 2) / sum_counts);

				filtered_centroid_counts[q_id] = sum_counts;
				was_cleaned = true;

				continue;
			}
			
			if (q1_id < 0)
				add_to_dict(q1, (int) filtered_centroids.size());
			if (q2_id < 0)
				add_to_dict(q2, (int) filtered_centroids.size());

			filtered_centroids.emplace_back(centr[j]);
			filtered_centroid_counts.emplace_back(centr_counts[j]);
		}

		swap(centroids[i], filtered_centroids);
		swap(centroid_counts[i], filtered_centroid_counts);
	}

	return was_cleaned;
}

// EOF
