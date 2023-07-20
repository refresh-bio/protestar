#include "psa_compressor.h"
#include "../compressors/cif-compressor.h"
#include "../compressors/pdb-compressor.h"
#include "../compressors/pae-compressor.h"
#include "../compressors/conf-compressor.h"
#include "../parsers/cif.h"
#include "../parsers/pdb.h"
#include "../parsers/json.h"
#include "io.h"
#include "../libs/refresh/tar/tar.h"
#include "../core/utils.h"

#include <filesystem>
#include <chrono>

// *****************************************************************
bool CPSACompressor::create(const string& file_name)
{
	out_archive = make_shared<refresh::archive_buffered_io>(false, 32u << 20);
	
	bool r = out_archive->open(file_name);
	if (!r)
		out_archive.reset();

	return r;
}

// *****************************************************************
bool CPSACompressor::copy_from_existing()
{
	if (!in_archive || !out_archive)
		return false;

	collection.set_archives(in_archive, out_archive);

	collection.deserialize_archive_version();
	auto archive_ver = collection.get_archive_version();
	if (archive_ver < MIN_SUPPORTED_ARCHIVE_VERSION || archive_ver > MAX_SUPPORTED_ARCHIVE_VERSION)
	{
		cerr << UNSUPPORTED_ARCHIVE_INFO(archive_ver);
		return false;
	}

	collection.deserialize();

	vector<refresh::archive_buffered_io::stream_t> v_streams;

	in_archive->list_streams(v_streams);

	data_t dat;
	uint64_t metadata;

	for (const auto& stream : v_streams)
	{
		if (collection.is_collection_stream_name(stream.stream_name))
			continue;

		auto stream_id = out_archive->register_stream(stream.stream_name);

//		int part_id = 0;
		while (in_archive->get_part(stream_id, dat, metadata))
			out_archive->add_part(stream_id, dat, metadata);
	}

	return true;
}

// *****************************************************************
bool CPSACompressor::add_new()
{
	if (params.input_archive.empty())
		collection.set_archives(in_archive, out_archive);		// In non-append mode it is necessary to configure collection here

	auto t1 = chrono::high_resolution_clock::now();

	refresh::tar_archive tar_archive;
	mutex mtx_tar;
	bool is_tar = !params.intar.empty();

	if (is_tar)
	{
		if (!tar_archive.open(params.intar))
			return false;
	}

	atomic<uint64_t> a_id = 0;

	vector<thread> thr_workers;

	thr_workers.reserve(params.no_threads);

	atomic<size_t> total_raw_cif{ 0 };
	atomic<size_t> total_raw_pdb{ 0 };
	atomic<size_t> total_raw_pae{ 0 };
	atomic<size_t> total_raw_conf{ 0 };
	atomic<size_t> total_pck_cif{ 0 };
	atomic<size_t> total_pck_pdb{ 0 };
	atomic<size_t> total_pck_pae{ 0 };
	atomic<size_t> total_pck_conf{ 0 };
	atomic<size_t> n_cif{ 0 };
	atomic<size_t> n_pdb{ 0 };
	atomic<size_t> n_pae{ 0 };
	atomic<size_t> n_conf{ 0 };

	bounded_queue<tar_item_t> tar_queue(params.no_threads * 64);

	int no_workers = params.no_threads;

	if (is_tar && is_gzipped(params.intar) && no_workers >= 4)
		--no_workers;

	for (int i = 0; i < no_workers; ++i)
		thr_workers.emplace_back([&, i] {
		
		CifCompressor cif_compressor(true);
		PDBCompressor pdb_compressor(true);
		PAECompressor pae_compressor;
		ConfCompressor conf_compressor;

		size_t my_total_raw_cif{ 0 };
		size_t my_total_raw_pdb{ 0 };
		size_t my_total_raw_pae{ 0 };
		size_t my_total_raw_conf{ 0 };
		size_t my_total_pck_cif{ 0 };
		size_t my_total_pck_pdb{ 0 };
		size_t my_total_pck_pae{ 0 };
		size_t my_total_pck_conf{ 0 };
		size_t my_n_cif{ 0 };
		size_t my_n_pdb{ 0 };
		size_t my_n_pae{ 0 };
		size_t my_n_conf{ 0 };

		data_t packed;

		const pair<int, int> already_exits_flag(-1, -1);

		Cif cif(params.minimal);
		Pdb pdb(params.minimal);
		JSON_PAE pae;
		matrix_t matrix;

		refresh::tar_item_t tar_item;

		string file_name;
		file_type_t file_type;
		bool in_tar_gzipped;

		while (true)
		{
			if(is_tar)
			{
/* {
					unique_lock<mutex> lck(mtx_tar);
					if (!tar_archive.get_next_item(tar_item))
						break;
				}*/
				if (!tar_queue.pop(tar_item))
					break;

				if (!tar_item.is_regular_file())
					continue;

				if (tar_item.data.empty())
					continue;

				uint64_t curr_id = a_id.fetch_add(1);

				if (params.verbose > 0)
					if (curr_id % 100 == 0)
						log("Processing " + to_string(curr_id) + "\r", 1, params.verbose == 1);

				file_name = tar_item.file_name();
				tie(file_type, in_tar_gzipped) = file_type_checker::check(file_name);
			}
			else
			{
				uint64_t curr_id = a_id.fetch_add(1);

				if (curr_id >= params.input_file_name_types.size())
					break;

				if (params.verbose > 0)
					if (curr_id % 100 == 0)
						log("Processing " + to_string(curr_id) + " of " + to_string(params.input_file_name_types.size()) + "\r", 1, params.verbose == 1);

				tie(file_name, file_type) = params.input_file_name_types[curr_id];
			}

			if (file_type == file_type_t::CIF)
			{
				if (is_tar && in_tar_gzipped)
				{
					data_t unpacked_data;

					if (!ungzip_from_buffer(tar_item.data.data(), tar_item.data.size(), unpacked_data))
					{
						cerr << "Error in reading: " + file_name + "\n";
						continue;
					}

					swap(unpacked_data, tar_item.data);
				}

				size_t bytes_load = is_tar ? cif.load(tar_item.data) : cif.load(file_name);

				if (bytes_load == 0)
				{
					cerr << "Error in reading: " + file_name + "\n";
					continue;
				}

				try {
					cif.parse();
				}
				catch (...)
				{
					cerr << "Invalid CIF file: " + file_name + "\n";
					continue;
				}

				cif_compressor.set_max_error(params.max_error_bb, params.max_error_sc);
				cif_compressor.set_max_compression(params.max_compression);
				cif_compressor.set_single_bf(params.single_bf);

				string struct_name = file_type_checker::extract_stem(file_name);

				cif_compressor.compress(&cif, packed, struct_name);

				CIF_file_desc_t fd(struct_name, bytes_load, 0, params.lossy, params.max_error_bb, params.max_error_sc, params.minimal, params.single_bf);

				if (collection.add_file_CIF_fast(fd, packed) == already_exits_flag)
					log("CIF file: " + file_name + " already present in archive", 1);
				else
				{
					log("Added CIF: " + file_name + " : " + to_string(bytes_load) + " -> " + to_string(packed.size()), 2);
					my_total_raw_cif += bytes_load;
					my_total_pck_cif += packed.size();
					my_n_cif++;
				}
			}
			else if (file_type == file_type_t::PDB)
			{
				if (is_tar && in_tar_gzipped)
				{
					data_t unpacked_data;

					if (!ungzip_from_buffer(tar_item.data.data(), tar_item.data.size(), unpacked_data))
					{
						cerr << "Error in reading: " + file_name + "\n";
						continue;
					}

					swap(unpacked_data, tar_item.data);
				}

				size_t bytes_load = is_tar ? pdb.load(tar_item.data) : pdb.load(file_name);

				if (bytes_load == 0)
				{
					cerr << "Error in reading: " + file_name + "\n";
					continue;
				}

				try {
					pdb.parse();
				}
				catch (...)
				{
					cerr << "Invalid PDB file: " + file_name + "\n";
					continue;
				}

				pdb_compressor.set_max_error(params.max_error_bb, params.max_error_sc);
				pdb_compressor.set_max_compression(params.max_compression);
				pdb_compressor.set_single_bf(params.single_bf);

				string struct_name = file_type_checker::extract_stem(file_name);

				pdb_compressor.compress(&pdb, packed, struct_name);

				PDB_file_desc_t fd(struct_name, bytes_load, 0, params.lossy, params.max_error_bb, params.max_error_sc, params.minimal, params.single_bf);

				if(collection.add_file_PDB_fast(fd, packed) == already_exits_flag)
					log("PDB file: " + file_name + " already present in archive", 1);
				else
				{
					log("Added PDB: " + file_name + " : " + to_string(bytes_load) + " -> " + to_string(packed.size()), 2);
					my_total_raw_pdb += bytes_load;
					my_total_pck_pdb += packed.size();
					my_n_pdb++;
				}
			}
			else if (file_type == file_type_t::PAE)
			{
				double max_declared_pae;
				size_t bytes_load = is_tar ? pae.load(tar_item.data, matrix, max_declared_pae) : pae.load(file_name, matrix, max_declared_pae);

				if (bytes_load == 0)
				{
					cerr << "Error in reading: " + file_name + "\n";
					continue;
				}

				pae_compressor.compress(matrix, max_declared_pae, params.pae_lossy_level, packed);

				PAE_file_desc_t fd(file_type_checker::extract_stem(file_name), bytes_load, 0, params.pae_lossy_level);

				if(collection.add_file_PAE_fast(fd, packed) == already_exits_flag)
					log("PAE file: " + file_name + " already present in archive", 1);
				else
				{
					log("Added PAE: " + file_name + " : " + to_string(bytes_load) + " -> " + to_string(packed.size()), 2);
					my_total_raw_pae += bytes_load;
					my_total_pck_pae += packed.size();
					my_n_pae++;
				}
			}
			else if (file_type == file_type_t::CONF)
			{
				data_t raw_data;

				if (is_tar)
				{
					raw_data = tar_item.data;
				}
				else
				{
					if (!load_file(file_name, raw_data))
					{
						cerr << "Error in reading: " + file_name + "\n";
						continue;
					}
				}

				conf_compressor.compress(raw_data, packed);

				CONF_file_desc_t fd(file_type_checker::extract_stem(file_name), raw_data.size(), 0);

				if(collection.add_file_CONF_fast(fd, packed) == already_exits_flag)
					log("CONF file: " + file_name + " already present in archive", 1);
				else
				{
					log("Added CONF: " + file_name + " : " + to_string(raw_data.size()) + " -> " + to_string(packed.size()), 2);
					my_total_raw_conf += raw_data.size();
					my_total_pck_conf += packed.size();
					my_n_conf++;
				}
			}
		}

		total_raw_cif += my_total_raw_cif;
		total_raw_pdb += my_total_raw_pdb;
		total_raw_pae += my_total_raw_pae;
		total_raw_conf += my_total_raw_conf;
		total_pck_cif += my_total_pck_cif;
		total_pck_pdb += my_total_pck_pdb;
		total_pck_pae += my_total_pck_pae;
		total_pck_conf += my_total_pck_conf;
		n_cif += my_n_cif;
		n_pdb += my_n_pdb;
		n_pae += my_n_pae;
		n_conf += my_n_conf;
		});

	// If tar archive, main thread loads files from tar to a queue
	if (is_tar)
	{
		tar_item_t tar_item;

		while (tar_archive.get_next_item(tar_item))
			tar_queue.push(tar_item);

		tar_queue.mark_completed();
	}

	for (auto& thr : thr_workers)
		thr.join();

	if (params.verbose > 0)
	{
		cerr << "Processed " + to_string(params.input_file_name_types.size()) + " files                                  " << endl;

		cerr << "***** Stats *****" << endl;
		if(n_cif)
			cerr << "CIFs:   no. files: " << n_cif << "   raw size: " << total_raw_cif << "   packed size: " << total_pck_cif << "   ratio: " << (double) total_pck_cif / total_raw_cif << endl;
		if(n_pdb)
			cerr << "PDBs:   no. files: " << n_pdb << "   raw size: " << total_raw_pdb << "   packed size: " << total_pck_pdb << "   ratio: " << (double) total_pck_pdb / total_raw_pdb << endl;
		if(n_pae)
			cerr << "PAEs:   no. files: " << n_pae << "   raw size: " << total_raw_pae << "   packed size: " << total_pck_pae << "   ratio: " << (double) total_pck_pae / total_raw_pae << endl;
		if(n_conf)
			cerr << "CONFs:  no. files: " << n_conf << "   raw size: " << total_raw_conf << "   packed size: " << total_pck_conf << "   ratio: " << (double) total_pck_conf / total_raw_conf << endl;

		size_t n_all = n_cif + n_pdb + n_pae + n_conf;
		size_t total_raw_all = total_raw_cif + total_raw_pdb + total_raw_pae + total_raw_conf;
		size_t total_pck_all = total_pck_cif + total_pck_pdb + total_pck_pae + total_pck_conf;
		cerr << "-----" << endl;
		cerr << "All:    no. files: " << n_all << "   raw size: " << total_raw_all << "   packed size: " << total_pck_all << "   ratio: " << (double)total_pck_all / total_raw_all << endl;
	}

	auto t2 = chrono::high_resolution_clock::now();
	chrono::duration<double> dur = t2 - t1;

	if (params.verbose > 0)
		cerr << "Adding time: " << dur.count() << "s" << endl;

	return true;
}

// *****************************************************************
bool CPSACompressor::close()
{
	if (!out_archive)
		return false;

	if(params.input_archive.empty())
		collection.set_archives(in_archive, out_archive);		// In non-append mode it is necessary to configure collection here

	collection.add_cmd_line(params.cmd);

	bool r = collection.serialize();

	in_archive.reset();
	out_archive.reset();

	return r;
}

// EOF
