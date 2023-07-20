#pragma once
#include <cstdio>
#include <string>
#include <cinttypes>
#include <vector>
#include <algorithm>
#include <zlib.h>

namespace refresh
{
	struct tar_header_t
	{
		char name[100];               /*   0 */
		char mode[8];                 /* 100 */
		char uid[8];                  /* 108 */
		char gid[8];                  /* 116 */
		char size[12];                /* 124 */
		char mtime[12];               /* 136 */
		char chksum[8];               /* 148 */
		char typeflag;                /* 156 */
		char linkname[100];           /* 157 */
		char magic[6];                /* 257 */
		char version[2];              /* 263 */
		char uname[32];               /* 265 */
		char gname[32];               /* 297 */
		char devmajor[8];             /* 329 */
		char devminor[8];             /* 337 */
		char prefix[155];             /* 345 */
		char fill[12];				  /* 500 */

		tar_header_t() = default;
		~tar_header_t() = default;
	};

	struct tar_item_t
	{
		tar_header_t header;
		std::vector<uint8_t> data;

		tar_item_t() = default;
		~tar_item_t() = default;

		std::string file_name()
		{
			return std::string(header.name);
		}

		size_t file_size()
		{
			size_t r = 0;

			for (int i = 0; i < 11; ++i)
				r = r * 8 + (size_t)(header.size[i] - '0');

			return r;
		}

		bool is_regular_file()
		{
			return header.typeflag == '0' || header.typeflag == 0;
		}
	};

	class tar_archive
	{
		size_t buffer_size = 16 << 20;
		const size_t block_size = 512;
//		std::string 
		bool is_gzipped = false;
		FILE* in = nullptr;
		gzFile gz_in;

		bool is_gzipped_file(const std::string& file_name)
		{
			if (file_name.length() < 4)
				return false;

			return file_name.substr(file_name.length() - 3, 3) == ".gz";
		}

		void _close()
		{
			if (in)
			{
				fclose(in);
				in = nullptr;
			}

			if (gz_in)
			{
				gzclose(gz_in);
				gz_in = nullptr;
			}
		}

	public:
		tar_archive(size_t buffer_size = 16 << 20) :
			buffer_size(buffer_size)
		{};
		~tar_archive() = default;

		bool open(std::string file_name)
		{
			_close();

			is_gzipped = is_gzipped_file(file_name);

			if (is_gzipped)
			{
				gz_in = gzopen(file_name.c_str(), "rb");
				if (!gz_in)
					return false;
				gzbuffer(gz_in, buffer_size);
			}
			else
			{
				in = fopen(file_name.c_str(), "rb");
				if (!in)
					return false;
				setvbuf(in, nullptr, _IOFBF, buffer_size);
			}
	
			return true;
		}

		void close()
		{
			_close();
		}

		bool get_next_item(tar_item_t& tar_item)
		{
			if (is_gzipped)
			{
				if (gzread(gz_in, &tar_item.header, block_size) != (int) block_size)
					return false;
			}
			else
			{
				if (fread(&tar_item.header, 1, block_size, in) != block_size)
					return false;
			}

			tar_item.data.clear();

			size_t n0 = (size_t) std::count((char*) & tar_item.header, (char*) &tar_item.header + block_size, 0);

			if (n0 == block_size)
				return false;			// EOF marker

			if (tar_item.is_regular_file())
			{
				size_t fs = tar_item.file_size();

				size_t to_read = (fs + block_size - 1) / block_size * block_size;

				tar_item.data.resize(to_read);

				if (is_gzipped)
				{
					if (gzread(gz_in, tar_item.data.data(), to_read) != (int) to_read)
						return false;
				}
				else
				{
					if (fread(tar_item.data.data(), 1, to_read, in) != to_read)
						return false;
				}

				tar_item.data.resize(fs);
			}

			return true;
		}

		bool eof()
		{
			if (is_gzipped)
				return gzeof(gz_in);
			else
				return (bool) feof(in);
		}
	};


};