#pragma once
#include "../core/utils.h"

#include <vector>
#include <cinttypes>
#include <string>
#include <cstdio>

#include <zlib.h>

using namespace std;

bool is_gzipped(const string& file_name);

template <class char_t>
bool load_file(const string& file_name, vector<char_t>& file_data)
{
	file_data.clear();

	if (file_name.empty())
	{
		// STDIN
		file_data.clear();

		for (int c = getchar(); c != EOF; c = getchar())
			file_data.emplace_back((uint8_t)c);

		return true;
	}
	else if (is_gzipped(file_name))
	{
		auto fs = filesystem::file_size(filesystem::path(file_name));

		gzFile f = gzopen(file_name.c_str(), "r");

		if (!f)
			return false;

		file_data.clear();

		gzbuffer(f, min<unsigned int>((unsigned int)fs, 2 << 20));

		const size_t chunk_size = fs;
		int total_readed = 0;

		while (!gzeof(f))
		{
			file_data.resize(total_readed + chunk_size);
			auto readed = gzread(f, file_data.data() + total_readed, (unsigned int)chunk_size);

			total_readed += readed;
		}

		file_data.resize(total_readed);

		gzclose(f);

		return true;
	}
	else
	{
		auto fs = filesystem::file_size(filesystem::path(file_name));

		FILE* f = fopen(file_name.c_str(), "rb");

		if (!f)
			return false;

		file_data.resize(fs);
		auto readed = fread(file_data.data(), 1, fs, f);

		fclose(f);

		return readed == fs;
	}
}

// ************************************************************************************
template <class char_t>
bool save_file(const string& file_name, const vector<char_t>& file_data)
{
	if (file_name.empty())
	{
		// STDOUT
		// !!! For Windows set stdout to binary!

		fwrite(file_data.data(), 1, file_data.size(), stdout);

		return true;
	}
	else if (is_gzipped(file_name))
	{
		gzFile f = gzopen(file_name.c_str(), "w6");

		if (!f)
			return false;

		auto written = gzwrite(f, file_data.data(), (unsigned int)file_data.size());

		gzclose(f);

		return written == (int)file_data.size();
	}
	else
	{
		FILE* f = fopen(file_name.c_str(), "wb");

		if (!f)
			return false;

		auto written = fwrite(file_data.data(), 1, file_data.size(), f);

		fclose(f);

		return written == file_data.size();
	}
}

template <class char_t>
bool ungzip_from_buffer(const uint8_t *buf_data, const size_t buf_size, vector<char_t>& file_data)
{
	file_data.clear();

	z_stream strm;

	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	auto ret = inflateInit2(&strm, 15+32);

	if (ret != Z_OK)
		return false;

	strm.next_in = (Bytef*) buf_data;
	strm.avail_in = buf_size;

	const uint32_t chunk_size = 256 << 10;

	do
	{
		strm.avail_out = chunk_size;

		uint32_t occupied = file_data.size();
		file_data.resize(occupied + chunk_size);
		strm.avail_out = (uInt) chunk_size;
		strm.next_out = (Bytef*)(file_data.data() + occupied);

		ret = inflate(&strm, Z_NO_FLUSH);
		
		if (ret == Z_STREAM_ERROR)
			return false;

		uint32_t unpacked = chunk_size - strm.avail_out;

		file_data.resize(occupied + unpacked);

		switch (ret) {
		case Z_NEED_DICT:
			ret = Z_DATA_ERROR;     /* and fall through */
		case Z_DATA_ERROR:
		case Z_MEM_ERROR:
			(void)inflateEnd(&strm);
			return false;
		}
	} while (strm.avail_out == 0);

	inflateEnd(&strm);

	return ret == Z_STREAM_END;
}

// EOF