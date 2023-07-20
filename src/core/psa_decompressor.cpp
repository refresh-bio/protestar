#include "psa_decompressor.h"
#include <filesystem>
#include "../parsers/cif.h"
#include "../parsers/pdb.h"
#include "../parsers/json.h"
#include "../compressors/cif-compressor.h"
#include "../compressors/pdb-compressor.h"
#include "../compressors/pae-compressor.h"
#include "../compressors/conf-compressor.h"

// *****************************************************************
bool CPSADecompressor::get_files()
{
	if (collection.empty())
		return false;

	auto t1 = chrono::high_resolution_clock::now();

	atomic<uint64_t> a_id = 0;

	vector<thread> thr_workers;

	thr_workers.reserve(params.no_threads);

	for (int i = 0; i < params.no_threads; ++i)
		thr_workers.emplace_back([&, i] {

		CifCompressor cif_compressor(false);
		PDBCompressor pdb_compressor(false);
		PAECompressor pae_compressor;
		ConfCompressor conf_compressor;

		data_t data_packed;
		uint64_t metadata;

		Cif cif_out(params.minimal);
		Pdb pdb_out(params.minimal);
		JSON_PAE pae_out;
		data_t data_unpacked;

		while (true)
		{
			uint64_t curr_id = a_id.fetch_add(1);

			if (curr_id >= params.input_file_name_types.size())
				break;

			if (params.verbose > 0)
				if (curr_id % 100 == 0)
					log("Processing " + to_string(curr_id) + " of " + to_string(params.input_file_name_types.size()) + "\r", 1, params.verbose == 1);

			const auto& name_ft = params.input_file_name_types[curr_id];

			if (params.file_type == file_type_t::CIF || params.file_type == file_type_t::ALL)
			{
				CIF_file_desc_t cif_desc;

				int seg_id = collection.get_file_CIF(name_ft.first, cif_desc);

				if (seg_id >= 0)
				{
					string stream_name = collection.stream_name(name_ft.first, file_type_t::CIF);

					int stream_id = in_archive->get_stream_id(stream_name);

					if (!in_archive->get_part(stream_id, cif_desc.id, data_packed, metadata))
					{
						if (is_app_mode)
						{
							cerr << "Archive internal error for: " << name_ft.first << endl;
							exit(1);
//							return false;
						}
					}

					string struct_name = name_ft.first;

					cif_compressor.decompress(&cif_out, data_packed, cif_desc.raw_size, struct_name);
					cif_out.store();
					cif_out.save(params.outdir + struct_name + ".cif");
				}
				else
				{
					if (is_app_mode && params.file_type != file_type_t::ALL)
						cerr << "No CIF file in archive: " << name_ft.first << endl;
				}
			}

			if (params.file_type == file_type_t::PDB || params.file_type == file_type_t::ALL)
			{
				PDB_file_desc_t pdb_desc;

				int seg_id = collection.get_file_PDB(name_ft.first, pdb_desc);

				if (seg_id >= 0)
				{
					string stream_name = collection.stream_name(name_ft.first, file_type_t::PDB);

					int stream_id = in_archive->get_stream_id(stream_name);

					if (!in_archive->get_part(stream_id, pdb_desc.id, data_packed, metadata))
					{
						if (is_app_mode)
						{
							cerr << "Archive internal error for: " << name_ft.first << endl;
							exit(1);
//							return false;
						}
					}

					string struct_name = name_ft.first;

					pdb_compressor.decompress(&pdb_out, data_packed, pdb_desc.raw_size, struct_name);
					pdb_out.store();
					pdb_out.save(params.outdir + struct_name + ".pdb");
				}
				else
				{
					if (is_app_mode && params.file_type != file_type_t::ALL)
						cerr << "No PDB file in archive: " << name_ft.first << endl;
				}
			}

			if (params.file_type == file_type_t::PAE || params.file_type == file_type_t::ALL)
			{
				PAE_file_desc_t pae_desc;
				int seg_id = collection.get_file_PAE(name_ft.first, pae_desc);

				if (seg_id >= 0)
				{
					string stream_name = collection.stream_name(name_ft.first, file_type_t::PAE);

					int stream_id = in_archive->get_stream_id(stream_name);

					if (!in_archive->get_part(stream_id, pae_desc.id, data_packed, metadata))
					{
						if (is_app_mode)
						{
							cerr << "Archive internal error for: " << name_ft.first << endl;
							exit(1);
//							return false;
						}
					}

					matrix_t matrix;
					double max_declared_pae;
					uint32_t lossy_level;

					pae_compressor.decompress(data_packed, matrix, max_declared_pae, lossy_level);
					pae_out.save(params.outdir + name_ft.first + ".json", matrix, max_declared_pae);
				}
				else
				{
					if (is_app_mode && params.file_type != file_type_t::ALL)
						exit(1);
					//						cerr << "No PAE file in archive: " << name_ft.first << endl;
				}
			}

			if (params.file_type == file_type_t::CONF || params.file_type == file_type_t::ALL)
			{
				CONF_file_desc_t conf_desc;
				int seg_id = collection.get_file_CONF(name_ft.first, conf_desc);

				if (seg_id >= 0)
				{
					string stream_name = collection.stream_name(name_ft.first, file_type_t::CONF);

					int stream_id = in_archive->get_stream_id(stream_name);

					if (!in_archive->get_part(stream_id, conf_desc.id, data_packed, metadata))
					{
						if (is_app_mode)
						{
							cerr << "Archive internal error for: " << name_ft.first << endl;
							exit(1);
//							return false;
						}
					}

					conf_compressor.decompress(data_packed, data_unpacked);
					ofstream ofs(params.outdir + name_ft.first + ".json", ios_base::binary);

					if (!ofs)
					{
						if (is_app_mode)
						{
							cerr << "Cannot create file: " << name_ft.first << endl;
							exit(1);
							//							return false;
						}
					}

					ofs.write((char*)data_unpacked.data(), data_unpacked.size());
				}
				else
				{
					if (is_app_mode && params.file_type != file_type_t::ALL)
						cerr << "No CONF file in archive: " << name_ft.first << endl;
				}
			}
		}
	});

	for (auto& thr : thr_workers)
		thr.join();

	if (params.verbose > 0)
		cerr << "Processed " + to_string(params.input_file_name_types.size()) + " files                                  " << endl;

	auto t2 = chrono::high_resolution_clock::now();
	chrono::duration<double> dur = t2 - t1;

	if (params.verbose > 0)
		cerr << "Getting time: " << dur.count() << "s" << endl;

	return true;
}

// EOF
