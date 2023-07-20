#include "conf-compressor.h"

#include <cinttypes>

// ************************************************************************************
//
bool ConfCompressor::compress(vector<uint8_t>& raw_data, packed_data_t& packed_data)
{
	size_t req_size = raw_data.size() + zim.get_overhead(raw_data.size());
	packed_data.resize(req_size + 4);

	auto packed_size = zim.compress(raw_data.data(), raw_data.size(), packed_data.data() + 4, req_size, 19);

	packed_data.resize(packed_size + 4);

	uint32_t raw_size = (uint32_t)raw_data.size();

	packed_data[0] = raw_size >> 24;
	packed_data[1] = (raw_size >> 16) & 0xffu;
	packed_data[2] = (raw_size >> 8) & 0xffu;
	packed_data[3] = raw_size & 0xffu;

	return true;
}

// ************************************************************************************
bool ConfCompressor::decompress(packed_data_t& packed_data, vector<uint8_t>& unpacked_data)
{
	uint32_t raw_size = ((uint32_t)packed_data[0]) << 24;
	raw_size += ((uint32_t)packed_data[1]) << 16;
	raw_size += ((uint32_t)packed_data[2]) << 8;
	raw_size += ((uint32_t)packed_data[3]);

	unpacked_data.resize(raw_size);

	zim.decompress(packed_data.data() + 4, packed_data.size() - 4, unpacked_data.data(), raw_size);

	return true;
}

// EOF
