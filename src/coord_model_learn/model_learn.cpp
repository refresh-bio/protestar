#include <utility>
#include <algorithm>
#include <ostream>
#include <cmath>
#include <tuple>
#include <atomic>
#include <thread>
#include "model_learn.h"

// *****************************************************************
void ModelLearn::clear()
{
	atom_distances.clear();
}

// *****************************************************************
int ModelLearn::atom_pos(const aa_t aa, const atom_t atom)
{
	const auto p = m_aa_atom.find(aa);

	if (p == m_aa_atom.end())
		return -1;

	auto q = find(p->second.begin(), p->second.end(), atom);

	return (int) (q - p->second.begin());
}

// *****************************************************************
void ModelLearn::add_chain_for_ref_atoms(const chain_desc_t& chain)
{
	map<atom_t, int_coords_t> aa_prev;
	map<atom_t, int_coords_t> aa_curr_to_swap_with_prev;

	vector<tuple<aa_t, atom_t, atom_t, int>> v_to_add;

	for (const auto& aa : chain.aa)
	{
		auto aa_type = aa.type;

		aa_curr_to_swap_with_prev.clear();

		for (const auto& atom : aa.atoms)
		{
			if (is_main_chain_atom(atom.type))
			{
				for (const auto& atom_prev : aa_prev)
//					add_to_model_for_ref_atoms(aa_type, atom.type, atom_conv_to_prev(atom_prev.first), coords_distance(atom_prev.second, atom.coords));
					v_to_add.emplace_back(aa_type, atom.type, atom_conv_to_prev(atom_prev.first), coords_distance(atom_prev.second, atom.coords));

				aa_curr_to_swap_with_prev[atom.type] = atom.coords;
			}

			int atom_cur_pos = atom_pos(aa_type, atom.type);

			for (const auto& atom_ref : aa.atoms)
			{
				if (atom_ref.type == atom.type)
					break;

				int atom_ref_pos = atom_pos(aa_type, atom.type);

				if (atom_cur_pos < atom_ref_pos)
					continue;

//				add_to_model_for_ref_atoms(aa_type, atom.type, atom_ref.type, coords_distance(atom.coords, atom_ref.coords));
				v_to_add.emplace_back(aa_type, atom.type, atom_ref.type, coords_distance(atom.coords, atom_ref.coords));
			}
		}

		aa_prev = aa_curr_to_swap_with_prev;
	}

	lock_guard<mutex> lck(mtx);
	for (const auto& x : v_to_add)
		add_to_model_for_ref_atoms(get<0>(x), get<1>(x), get<2>(x), get<3>(x));
}

// *****************************************************************
void ModelLearn::add_chain_for_tetrahedrons(const chain_desc_t& chain)
{
	map<atom_t, int_coords_t> aa_prev;
	map<atom_t, int_coords_t> aa_curr_to_swap_with_prev;

	map<atom_t, int_coords_t> aa_curr;

	vector<tuple<aa_t, atom_t, dist6_t>> v_to_add;

	for (const auto& aa : chain.aa)
	{
		auto aa_type = aa.type;

		aa_curr_to_swap_with_prev.clear();

		aa_curr.clear();
//		aa_curr.insert(aa.second.begin(), aa.second.end());
		for (const auto& x : aa.atoms)
			aa_curr.emplace(x.type, x.coords);

		for (const auto& atom : aa.atoms)
		{
			if (is_main_chain_atom(atom.type))
				aa_curr_to_swap_with_prev[atom.type] = atom.coords;

			array<int_coords_t, 3> ref_coords;
			bool all_corrds_present = true;

			for (int i = 0; i < 3; ++i)
			{
				atom_t ra = get<0>(reference_atoms[make_pair(aa_type, atom.type)][i]);

				if (is_prev_aa_atom(ra))
				{
					auto p = aa_prev.find(atom_conv_from_prev(ra));
					if (p != aa_prev.end())
						ref_coords[i] = p->second;
					else
						all_corrds_present = false;
				}
				else
				{
					auto p = aa_curr.find(ra);
					if (p != aa_curr.end())
						ref_coords[i] = p->second;
					else
						all_corrds_present = false;
				}
			}

			if (all_corrds_present)
				v_to_add.emplace_back(aa_type, atom.type, dist6_t{
				coords_distance(ref_coords[0], ref_coords[1]),
					coords_distance(ref_coords[0], ref_coords[2]),
					coords_distance(ref_coords[1], ref_coords[2]),
					coords_distance(atom.coords, ref_coords[0]),
					coords_distance(atom.coords, ref_coords[1]),
					coords_distance(atom.coords, ref_coords[2]) });

/*			add_to_model_for_terahedrons(aa_type, atom.type, dist6_t{
			coords_distance(ref_coords[0], ref_coords[1]),
				coords_distance(ref_coords[0], ref_coords[2]),
				coords_distance(ref_coords[1], ref_coords[2]),
				coords_distance(atom.coords, ref_coords[0]),
				coords_distance(atom.coords, ref_coords[1]),
				coords_distance(atom.coords, ref_coords[2]) });*/
		}

		aa_prev = aa_curr_to_swap_with_prev;
	}

	lock_guard<mutex> lck(mtx);

	for (const auto& x : v_to_add)
		add_to_model_for_terahedrons(get<0>(x), get<1>(x), get<2>(x));
}

// *****************************************************************
void ModelLearn::add_to_model_for_ref_atoms(const aa_t aa, const atom_t atom_cur, const atom_t atom_ref, const int dist)
{
	vector<int>& a_d = atom_distances[make_pair(aa, atom_cur)][atom_ref];

	if (a_d.size() < max_no_records)
		a_d.emplace_back(dist);
}

// *****************************************************************
void ModelLearn::add_to_model_for_terahedrons(const aa_t aa, const atom_t atom_cur, const dist6_t& dist)
{
	vector<dist6_t>& a_d = tetrahedrons[make_pair(aa, atom_cur)];

	if (a_d.size() < max_no_records)
		a_d.emplace_back(dist);
}

// *****************************************************************
void ModelLearn::parse_for_ref_atoms(Protein& protein)
{
	for (const auto& chain : protein.get_chains())
		add_chain_for_ref_atoms(chain);
}

// *****************************************************************
void ModelLearn::parse_for_tetrahedrons(Protein& protein)
{
	for (const auto& chain : protein.get_chains())
		add_chain_for_tetrahedrons(chain);
}

// *****************************************************************
void ModelLearn::choose_ref_atoms()
{
	reference_atoms.clear();

	vector<atom_ctx> v_atom_ctx;

	for (const auto& ad : atom_distances)
		v_atom_ctx.emplace_back(ad.first);

	vector<thread> threads;
	atomic<uint64_t> a_id = 0;
	mutex mtx;

	for (int i = 0; i < no_threads; ++i)
		threads.emplace_back([&] {
		while (true)
		{
			uint64_t id = atomic_fetch_add(&a_id, 1);
			if (a_id >= v_atom_ctx.size())
				break;

			vector<pair<pair<int, int>, atom_t>> cand_atoms;

			auto ad = v_atom_ctx[id];

			for (const auto& ca : atom_distances[ad])
			{
				int64_t sum_d = 0;
				int64_t avg_d = 0;

				for (auto& d : ca.second)
					sum_d += d;

				if (!ca.second.empty())
					avg_d = sum_d / ca.second.size();
				else
					avg_d = numeric_limits<int>::max();

				int64_t var_d = 0;

				for (auto& d : ca.second)
					var_d += (d - avg_d) * (d - avg_d);

				if (!ca.second.empty())
					var_d /= ca.second.size();
				else
					var_d = numeric_limits<int>::max();

				//			cand_atoms.emplace_back(make_pair(var_d, avg_d), ca.first);
				cand_atoms.emplace_back(make_pair(avg_d, avg_d), ca.first);
			}

			std::sort(cand_atoms.begin(), cand_atoms.end());

			{
				lock_guard<mutex> lck(mtx);

				reference_atoms[ad] = array<tuple<atom_t, int, int>, 3>{
					make_tuple(cand_atoms[0].second, cand_atoms[0].first.second, cand_atoms[0].first.first),
					make_tuple(cand_atoms[1].second, cand_atoms[1].first.second, cand_atoms[1].first.first),
					make_tuple(cand_atoms[2].second, cand_atoms[2].first.second, cand_atoms[2].first.first)};
			}
		}
	});


	for (auto& t : threads)
		t.join();
	threads.clear();

/*
	for (const auto& ad : atom_distances)
	{
		vector<pair<pair<int, int>, atom_t>> cand_atoms;

		for (const auto& ca : ad.second)
		{
			int64_t sum_d = 0;
			int64_t avg_d = 0;

			for (auto& d : ca.second)
				sum_d += d;

			if (!ca.second.empty())
				avg_d = sum_d / ca.second.size();
			else
				avg_d = numeric_limits<int>::max();

			int64_t var_d = 0;

			for (auto& d : ca.second)
				var_d += (d - avg_d) * (d - avg_d);

			if (!ca.second.empty())
				var_d /= ca.second.size();
			else
				var_d = numeric_limits<int>::max();

//			cand_atoms.emplace_back(make_pair(var_d, avg_d), ca.first);
			cand_atoms.emplace_back(make_pair(avg_d, avg_d), ca.first);
		}

		sort(cand_atoms.begin(), cand_atoms.end());

		reference_atoms[ad.first] = array<tuple<atom_t, int, int>, 3>{
			make_tuple(cand_atoms[0].second, cand_atoms[0].first.second, cand_atoms[0].first.first),
				make_tuple(cand_atoms[1].second, cand_atoms[1].first.second, cand_atoms[1].first.first),
				make_tuple(cand_atoms[2].second, cand_atoms[2].first.second, cand_atoms[2].first.first)};
	}*/

#ifdef MODEL_LEARN_VERBOSE
	for (auto& ra : reference_atoms)
	{
		cerr << aa_to_str(ra.first.first) << " " << atom_to_str(ra.first.second) << " : ";
		for (int i = 0; i < 3; ++i)
			cerr << atom_to_str(get<0>(ra.second[i])) << " (" << get<1>(ra.second[i]) << "," << (int)sqrt(get<2>(ra.second[i])) << ")   ";
		cerr << endl;
	}

#endif
}

// *****************************************************************
void ModelLearn::calculate_centroids()
{
	centroids.clear();

	vector<thread> threads;
	vector<atom_ctx> v_atom_ctx;
	atomic<uint64_t> a_id = 0;

	for (const auto& tetr : tetrahedrons)
		v_atom_ctx.emplace_back(tetr.first);

	for (int i = 0; i < no_threads2; ++i)
		threads.emplace_back([&]() {
		while (true)
		{
			uint64_t id = atomic_fetch_add(&a_id, 1);
			if (id >= v_atom_ctx.size())
				break;
			cerr << aa_atom_to_str(v_atom_ctx[id]) + "                  \r";

			auto ctr = find_centroids(tetrahedrons[v_atom_ctx[id]]);

			{
				lock_guard<mutex> lck(mtx);
				centroids[v_atom_ctx[id]] = ctr.first;
				prediction_precision[v_atom_ctx[id]] = ctr.second;
			}
		}
	});

	for (auto& t : threads)
		t.join();
	threads.clear();

/*	for (const auto& tetr : tetrahedrons)
	{
		cerr << aa_atom_to_str(tetr.first) << "                  \r";

		tie(centroids[tetr.first], prediction_precision[tetr.first]) = find_centroids(tetr.second);
	}*/
}

// *****************************************************************
pair<vector<ModelLearn::dist6_t>, int> ModelLearn::find_centroids(const vector<dist6_t>& vec)
{
	if (vec.size() == 1)
		return make_pair(vec, 0);

	vector<array<double, 6>> vd;
	vd.reserve(vec.size());

	vector<array<double, 6>> found_centroids, best_centroids;
	vector<int> found_assignments;
//	int best_k;
	double best_cost = numeric_limits<double>::max();
	double best_raw_cost = numeric_limits<double>::max();

	for (const auto& x : vec)
		vd.emplace_back(array<double, 6>{(double)x[0], (double)x[1], (double)x[2], (double)x[3], (double)x[4], (double)x[5]});

	for (int k = 1; k <= max_no_centroids; ++k)
	{
//		kmeans<double, 6, SquaredEuclidean<double, 6>>(vd, found_centroids, found_assignments, k, 0.001, 100);
		kmeans<double, 6, Euclidean<double, 6>>(vd, found_centroids, found_assignments, k, 0.001, 200);
//		kmeans<double, 6, TetrahedronPrediction<double, 6>>(vd, found_centroids, found_assignments, k, 0.001, 100);
		double cost = calculate_cost<double, 6, 6>(vd, found_centroids, found_assignments);

		double ext_cost = cost + log2((double)k);

		if (ext_cost < best_cost)
		{
			best_cost = ext_cost;
			best_raw_cost = cost;
//			best_k = k;
			best_centroids = found_centroids;
		}
	}

	vector<dist6_t> ret;
	ret.reserve(best_centroids.size());

	for (const auto& x : best_centroids)
		ret.emplace_back(array<int, 6>{ (int)x[0], (int)x[1], (int)x[2], (int)x[3], (int)x[4], (int)x[5] });

	return make_pair(ret, (int)(best_raw_cost));
}

// *****************************************************************
// Store centroids in C++ file for use in compression
void ModelLearn::serialize_model(const string &out_name)
{
	ofstream ofs(out_name);

	if (!ofs)
	{
		cerr << "Cannot create " << out_name << " file\n";
		return;
	}

	// *** Centroids
	ofs << "// *****************************************************************\n\n";
	ofs << "void ModelCompress::init_centroids()\n";
	ofs << "{\n";

	ofs << "    centroids.resize(max_packed_atom_ctx);\n";

	for (const auto& me : centroids)
	{
		ofs << "    centroids[packed_atom_ctx(aa_t::" << aa_to_str(me.first.first) << ", atom_t::" << atom_to_str(me.first.second) << ")] = vector<dist6_t>{";

		string sep = "";

		for (const auto& cen : me.second)
		{
			ofs << sep << "{" << cen[0] << ", " << cen[1] << ", " << cen[2] << ", " << cen[3] << ", " << cen[4] << ", " << cen[5] << "}";
			sep = ",\n\t\t ";
		}

		ofs << "};\n";
	}

	ofs << "}\n\n";

	// *** Reference atoms
	ofs << "// *****************************************************************\n\n";
	ofs << "void ModelCompress::init_reference_atoms()\n";
	ofs << "{\n";

	ofs << "    reference_atoms.resize(max_packed_atom_ctx);\n";

	for (const auto& ra : reference_atoms)
	{
		ofs << "    reference_atoms[packed_atom_ctx(aa_t::" << aa_to_str(ra.first.first) << ", atom_t::" << atom_to_str(ra.first.second) << ")] = array<atom_t, 3>{"
			<< "atom_t::" << atom_to_str(get<0>(ra.second[0])) << ", "
			<< "atom_t::" << atom_to_str(get<0>(ra.second[1])) << ", "
			<< "atom_t::" << atom_to_str(get<0>(ra.second[2])) << "};\n";
	}

	ofs << "}\n\n";

	// *** Prediction precision
	ofs << "// *****************************************************************\n\n";
	ofs << "void ModelCompress::init_prediction_precision()\n";
	ofs << "{\n";

	ofs << "    prediction_precision.resize(max_packed_atom_ctx);\n";

	for (const auto& pp : prediction_precision)
		ofs << "    prediction_precision[packed_atom_ctx(aa_t::" << aa_to_str(pp.first.first) << ", atom_t::" << atom_to_str(pp.first.second) << ")] = " << pp.second << ";\n";

	ofs << "}\n\n";

	ofs << "// EOF\n";
}

// EOF