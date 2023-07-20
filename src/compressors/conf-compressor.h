#pragma once

#include "../common/defs.h"

#include "../libs/refresh/compression/zstd_wrapper.h"

using namespace refresh;

// *****************************************************************
class ConfCompressor
{
	const int compression_level = 19;
	zstd_in_memory zim{ compression_level };

public:
	ConfCompressor() = default;
	~ConfCompressor() = default;

	bool compress(vector<uint8_t> &raw_data, packed_data_t& packed_data);
	bool decompress(packed_data_t& packed_data, vector<uint8_t> &unpacked_data);
};

// EOF
