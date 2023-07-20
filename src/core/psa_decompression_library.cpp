#include "psa_decompressor_library.h"
#include "../parsers/cif.h"
#include "../parsers/pdb.h"
#include "../parsers/json.h"
#include "../compressors/cif-compressor.h"
#include "../compressors/pdb-compressor.h"
#include "../compressors/pae-compressor.h"
#include "../compressors/conf-compressor.h"

#include <algorithm>

// *****************************************************************
bool CPSADecompressorLibrary::GetCmdLines(vector<string>& v_cmd)
{
	if (collection.empty())
		return false;

	v_cmd = collection.get_cmd_lines();

	return true;
}

// *****************************************************************
uint32_t CPSADecompressorLibrary::GetArchiveVersion()
{
	return collection.get_archive_version();
}

// *****************************************************************
bool CPSADecompressorLibrary::GetNoFiles(map<file_type_t, size_t>& no_files)
{
	if (collection.empty())
		return false;

	no_files[file_type_t::CIF] = collection.get_no_entries(file_type_t::CIF);
	no_files[file_type_t::PDB] = collection.get_no_entries(file_type_t::PDB);
	no_files[file_type_t::PAE] = collection.get_no_entries(file_type_t::PAE);
	no_files[file_type_t::CONF] = collection.get_no_entries(file_type_t::CONF);

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::ListFiles(file_type_t file_type, vector<string>& file_list)
{
	file_list.clear();

	if (collection.empty())
		return false;

	if (file_type == file_type_t::CIF || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_CIT();
		for (const auto& x : vec)
			file_list.emplace_back(x.name);
	}
	if (file_type == file_type_t::PDB || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_PDB();
		for (const auto& x : vec)
			file_list.emplace_back(x.name);
	}
	if (file_type == file_type_t::PAE || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_PAE();
		for (const auto& x : vec)
			file_list.emplace_back(x.name);
	}
	if (file_type == file_type_t::CONF || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_CONF();
		for (const auto& x : vec)
			file_list.emplace_back(x.name);
	}

	sort(file_list.begin(), file_list.end());

	if (file_type == file_type_t::ALL)
	{
		auto new_end = unique(file_list.begin(), file_list.end());
		file_list.erase(new_end, file_list.end());
	}

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::ListFilesExt(file_type_t file_type, vector<pair<string, file_type_t>>& file_list_ext)
{
	file_list_ext.clear();

	if (collection.empty())
		return false;

	if (file_type == file_type_t::CIF || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_CIT();
		for (const auto& x : vec)
			file_list_ext.emplace_back(x.name, file_type_t::CIF);
	}
	if (file_type == file_type_t::PDB || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_PDB();
		for (const auto& x : vec)
			file_list_ext.emplace_back(x.name, file_type_t::PDB);
	}
	if (file_type == file_type_t::PAE || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_PAE();
		for (const auto& x : vec)
			file_list_ext.emplace_back(x.name, file_type_t::PAE);
	}
	if (file_type == file_type_t::CONF || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_CONF();
		for (const auto& x : vec)
			file_list_ext.emplace_back(x.name, file_type_t::CONF);
	}

	sort(file_list_ext.begin(), file_list_ext.end());

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::GetFilesInfo(file_type_t file_type, vector<unique_ptr<file_desc_t>>& file_desc)
{
	file_desc.clear();

	if (collection.empty())
		return false;

	if (file_type == file_type_t::CIF || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_CIT();
		for (const auto& x : vec)
			file_desc.emplace_back(make_unique<CIF_file_desc_t>(x));
	}
	if (file_type == file_type_t::PDB || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_PDB();
		for (const auto& x : vec)
			file_desc.emplace_back(make_unique<PDB_file_desc_t>(x));
	}
	if (file_type == file_type_t::PAE || file_type == file_type_t::ALL)
	{
		auto vec = collection.list_files_PAE();
		for (const auto& x : vec)
			file_desc.emplace_back(make_unique<PAE_file_desc_t>(x));
	}

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::GetCIFData(const string& file_name, vector<char>& file_data)
{
	CifCompressor cif_compressor(false);
	CIF_file_desc_t cif_desc;
	Cif cif_out(false);

	vector<uint8_t> data_packed;
	uint64_t metadata;

	int seg_id = collection.get_file_CIF(file_name, cif_desc);

	if (seg_id >= 0)
	{
		string stream_name = collection.stream_name(file_name, file_type_t::CIF);

		int stream_id = in_archive->get_stream_id(stream_name);

		if (!in_archive->get_part(stream_id, cif_desc.id, data_packed, metadata))
			return false;

		cif_compressor.decompress(&cif_out, data_packed, cif_desc.raw_size, file_name);
		cif_out.store();
		cif_out.contents(file_data);
	}
	else
		return false;

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::GetPDBData(const string& file_name, vector<char>& file_data)
{
	PDBCompressor pdb_compressor(false);
	PDB_file_desc_t pdb_desc;
	Pdb pdb_out(false);

	vector<uint8_t> data_packed;
	uint64_t metadata;

	int seg_id = collection.get_file_PDB(file_name, pdb_desc);

	if (seg_id >= 0)
	{
		string stream_name = collection.stream_name(file_name, file_type_t::PDB);

		int stream_id = in_archive->get_stream_id(stream_name);

		if (!in_archive->get_part(stream_id, pdb_desc.id, data_packed, metadata))
			return false;

		pdb_compressor.decompress(&pdb_out, data_packed, pdb_desc.raw_size, file_name);
		pdb_out.store();
		pdb_out.contents(file_data);
	}
	else
		return false;

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::GetPAEData(const string& file_name, vector<char>& file_data)
{
	PAECompressor pae_compressor;
	JSON_PAE pae_out;
	PAE_file_desc_t pae_desc;

	vector<uint8_t> data_packed;
	uint64_t metadata;

	int seg_id = collection.get_file_PAE(file_name, pae_desc);

	if (seg_id >= 0)
	{
		string stream_name = collection.stream_name(file_name, file_type_t::PAE);

		int stream_id = in_archive->get_stream_id(stream_name);

		if (!in_archive->get_part(stream_id, pae_desc.id, data_packed, metadata))
			return false;

		matrix_t matrix;
		double max_declared_pae;
		uint32_t lossy_level;

		pae_compressor.decompress(data_packed, matrix, max_declared_pae, lossy_level);
		pae_out.contents(file_data, matrix, max_declared_pae);
	}
	else
		return false;

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::GetCONFData(const string& file_name, vector<char>& file_data)
{
	ConfCompressor conf_compressor;
	CONF_file_desc_t conf_desc;
	
	vector<uint8_t> data_packed;
	vector<uint8_t> data_unpacked;
	uint64_t metadata;

	int seg_id = collection.get_file_CONF(file_name, conf_desc);

	if (seg_id >= 0)
	{
		string stream_name = collection.stream_name(file_name, file_type_t::CONF);

		int stream_id = in_archive->get_stream_id(stream_name);

		if (!in_archive->get_part(stream_id, conf_desc.id, data_packed, metadata))
			return false;

		if (!conf_compressor.decompress(data_packed, data_unpacked))
			return false;

		file_data.assign(data_unpacked.begin(), data_unpacked.end());
	}
	else
		return false;

	return true;
}

// *****************************************************************
bool CPSADecompressorLibrary::GetFileData(file_type_t file_type, const string& file_name, vector<char>& file_data)
{
	if (collection.empty())
		return false;

	switch (file_type)
	{
	case file_type_t::CIF:
		return GetCIFData(file_name, file_data);
	case file_type_t::PDB:
		return GetPDBData(file_name, file_data);
	case file_type_t::PAE:
		return GetPAEData(file_name, file_data);
	case file_type_t::CONF:
		return GetCONFData(file_name, file_data);
	default:
		return false;
	}

	return false;
}

// *****************************************************************
bool CPSADecompressorLibrary::DeserializeArchiveVersion()
{
	if (!in_archive)
		return false;

	collection.set_archives(in_archive, nullptr);

	return collection.deserialize_archive_version();
}

// *****************************************************************
bool CPSADecompressorLibrary::Deserialize()
{
	if (!in_archive)
		return false;

	collection.set_archives(in_archive, nullptr);

	return collection.deserialize();
}

// *****************************************************************
bool CPSADecompressorLibrary::DeserializeInit()
{
	if (!in_archive)
		return false;

	collection.set_archives(in_archive, nullptr);

	return collection.deserialize_init();
}

// *****************************************************************
bool CPSADecompressorLibrary::close()
{
	if (!in_archive)
		return false;

	collection.close();
	in_archive.reset();

	return true;
}

// EOF
