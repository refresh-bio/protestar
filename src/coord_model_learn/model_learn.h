#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <array>
#include <random>
#include <mutex>

#include "../common/atom_extractor.h"
#include "../common/model.h"
#include "kmeans.h"

using namespace std;

#ifdef MODEL_LEARN_VERBOSE
#include <iostream>
#endif

// *****************************************************************
struct hash_pair {
	template <class T1, class T2>
	size_t operator()(const pair<T1, T2>& p) const
	{
		auto hash1 = hash<T1>{}(p.first);
		auto hash2 = hash<T2>{}(p.second);

		if (hash1 != hash2) {
			return hash1 ^ hash2;
		}

		return hash1;
	}
};

// *****************************************************************
class ModelLearn : public Model
{
	const size_t max_no_records = 1000000;
	const size_t no_kmeans_repetitions = 10;
	int no_threads = 4;
	int no_threads2 = 8;
	const uint32_t max_init_cnt = 1000;

	unordered_map<atom_ctx, unordered_map<atom_t, vector<int>>, hash_pair> atom_distances;
//	map<atom_ctx, unordered_map<atom_t, vector<int>>> atom_distances;
	unordered_map<atom_ctx, array<tuple<atom_t, int, int>, 3>, hash_pair> reference_atoms;
//	map<atom_ctx, array<tuple<atom_t, int, int>, 3>> reference_atoms;
	unordered_map<atom_ctx, int, hash_pair> prediction_precision;
//	map<atom_ctx, int> prediction_precision;

	map<atom_ctx, vector<dist6_t>> tetrahedrons;
	map<atom_ctx, vector<dist6_t>> centroids;
	map<atom_ctx, vector<uint32_t>> centroid_counts;

	mt19937 mt;

	mutex mtx;

	void add_chain_for_ref_atoms(const chain_desc_t& chain);
	void add_chain_for_tetrahedrons(const chain_desc_t& chain);

	void add_to_model_for_ref_atoms(const aa_t aa, const atom_t atom_cur, const atom_t atom_ref, const int dist);
	void add_to_model_for_terahedrons(const aa_t aa, const atom_t atom_cur, const dist6_t& dist);

	int atom_pos(const aa_t aa, const atom_t atom);

	tuple<vector<dist6_t>, int, vector<uint32_t>> find_centroids(const vector<dist6_t>& vec);

	template <class T, int NDims, int NLast>
	T calculate_cost(
		const table_t<T, NDims>& points,
		const table_t<T, NDims>& centroids,
		const std::vector<int>& assignments) {

		T cost = 0;

		TetrahedronPrediction<T, NDims> calc;

		for (int j = 0; j < (int) points.size(); j++) {
			const auto& p = points[j];

			const auto& c = centroids[assignments[j]];

			cost += calc(p, c);
		}

		return cost / points.size();
	}

	double est_entropy(const vector<int>& assignments, int k)
	{
		vector<int> n_ass(k);

		for (const auto x : assignments)
			++n_ass[x];

		double ent = 0;
		double tot = (double)assignments.size();

		for (const auto x : n_ass)
			ent += -log2((double)x / tot) * x / tot;

		return ent;
	}

public:
	ModelLearn() : Model()
	{}
	~ModelLearn() = default;

	void set_no_threads(int _no_threads, int _no_threads2)
	{
		no_threads = _no_threads;
		no_threads2 = _no_threads2;
	}
	void clear();
	void parse_for_ref_atoms(Protein& protein);
	void parse_for_tetrahedrons(Protein& protein);

	void choose_ref_atoms();
	void calculate_centroids();

	void serialize_model(const string &out_name);
};

// EOF
