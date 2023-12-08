#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <random>

#include "../common/atom_extractor.h"

using namespace std;

#define MODEL_LEARN_VERBOSE

// *****************************************************************
class Model
{
protected:
	using atom_ctx = pair<aa_t, atom_t>;
	using dist6_t = array<int, 6>;
	using dist3_t = array<int, 3>;

	uint32_t packed_atom_ctx(const atom_ctx x)
	{
		return ((int)x.first << 6) + (int)x.second;
	}

	pair<aa_t, atom_t> unpack_atom_ctx(uint32_t x)
	{
		return make_pair((aa_t)(x >> 6), (atom_t)(x & 0x3fu));
	}

	vector<vector<dist6_t>> packed_centroids;

public:
	const int max_no_centroids = 20;
	const uint32_t max_packed_atom_ctx = 32 << 6;

	Model() = default;
	~Model() = default;

	uint32_t packed_atom_ctx(const aa_t aa, const atom_t atom)
	{
		return ((int)aa << 6) + (int)atom;
	}
};

// EOF
