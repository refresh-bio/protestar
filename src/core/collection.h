#pragma once

#include <string>
#include <vector>
#include <map>
#include <utility>
#include <mutex>
#include <cinttypes>
#include <array>
#include <ctime>

#include "../core/utils.h"
#include "../core/params.h"
#include "../compressors/serializer.h"
#include "../core/version.h"
#include <zstd_wrapper.h>
#include <archive.h>

using namespace std;

using data_t = vector<uint8_t>;

// *****************************************************************
struct file_desc_t
{
	file_type_t file_type = file_type_t::none;
	string name;
	size_t raw_size = 0;
	int id = -1;

	file_desc_t() = default;

	file_desc_t(const file_type_t file_type, const string& name, size_t raw_size, int id) :
		file_type(file_type),
		name(name),
		raw_size(raw_size),
		id(id)
	{}

	file_desc_t(const file_desc_t&) = default;
	file_desc_t& operator=(const file_desc_t&) = default;

	void serialize(data_t& data);
	void deserialize(data_t::iterator& p);

	string file_type_str()
	{
		if (file_type == file_type_t::CIF)
			return "CIF";
		else if (file_type == file_type_t::PDB)
			return "PDB";
		else if (file_type == file_type_t::PAE)
			return "PAE";
		else if (file_type == file_type_t::CONF)
			return "CONF";
		else
			return "unknown";
	}

	virtual string str() = 0;
};

// *****************************************************************
struct CIF_file_desc_t : public file_desc_t
{
	bool lossy = false;
	int max_error_bb = 0;
	int max_error_sc = 0;
	bool minimal = false;
	bool single_bf = false;

	CIF_file_desc_t() = default;

	CIF_file_desc_t(string name, size_t raw_size, int id, bool lossy = false, int max_error_bb = 0, int max_error_sc = 0, bool minimal = false, bool single_bf = false) :
		file_desc_t(file_type_t::CIF, name, raw_size, id),
		lossy(lossy),
		max_error_bb(max_error_bb),
		max_error_sc(max_error_sc),
		minimal(minimal),
		single_bf(single_bf)
	{}

	CIF_file_desc_t(const CIF_file_desc_t&) = default;
	CIF_file_desc_t& operator=(const CIF_file_desc_t&) = default;

protected:
	CIF_file_desc_t(file_type_t file_type, string name, size_t raw_size, int id, bool lossy = false, int max_error_bb = 0, int max_error_sc = 0, bool minimal = false, bool single_bf = false) :
		file_desc_t(file_type, name, raw_size, id),
		lossy(lossy),
		max_error_bb(max_error_bb),
		max_error_sc(max_error_sc),
		minimal(minimal),
		single_bf(single_bf)
	{}

public:
	void serialize(data_t& data);
	void deserialize(data_t::iterator& p);
	
	virtual string str()
	{
		return
			name + " " +
			file_type_str() + " " +
			"size: " + to_string(raw_size) +
			(lossy ? " lossy [max.err.bb: "s + to_string(max_error_bb) + "mA  max.err.sc: " + to_string(max_error_sc) + "mA]" : "") +
			(minimal ? " minimal" : "") +
			(single_bf ? " single B-factor" : "");
	}
};

// *****************************************************************
struct PDB_file_desc_t : public CIF_file_desc_t
{
	PDB_file_desc_t() = default;

	PDB_file_desc_t(string name, size_t raw_size, int id, bool lossy = false, int max_error_bb = 0, int max_error_sc = 0, bool minimal = false, bool single_bf = false) :
		CIF_file_desc_t(file_type_t::PDB, name, raw_size, id, lossy, max_error_bb, max_error_sc, minimal, single_bf)
	{}

	void serialize(data_t& data);
	void deserialize(data_t::iterator& p);

	virtual string str()
	{
		return
			name + " " +
			file_type_str() + " " +
			"size: " + to_string(raw_size) +
			(lossy ? " lossy [max.err.bb: "s + to_string(max_error_bb) + "mA  max.err.sc: " + to_string(max_error_sc) + "mA]" : "") +
			(minimal ? " minimal" : "") +
			(single_bf ? " single B-factor" : "");
	}
};

// *****************************************************************
struct PAE_file_desc_t : public file_desc_t
{
	uint32_t lossy_level = 0;

	PAE_file_desc_t() = default;

	PAE_file_desc_t(string name, size_t raw_size, int id, uint32_t lossy_level = 0) :
		file_desc_t(file_type_t::PAE, name, raw_size, id),
		lossy_level(lossy_level)
	{}

	void serialize(data_t& data);
	void deserialize(data_t::iterator& p);

	virtual string str()
	{
		return
			name + " " +
			file_type_str() + " " +
			"size: " + to_string(raw_size) +
			(lossy_level != 0 ? " lossy-level: " + to_string(lossy_level) : "");
	}
};

// *****************************************************************
struct CONF_file_desc_t : public file_desc_t
{
	CONF_file_desc_t() = default;

	CONF_file_desc_t(string name, size_t raw_size, int id) :
		file_desc_t(file_type_t::PAE, name, raw_size, id)
	{}

	void serialize(data_t& data);
	void deserialize(data_t::iterator& p);

	virtual string str()
	{
		return
			name + " " +
			file_type_str() +
			" size: " + to_string(raw_size);
	}
};

// *****************************************************************
class CCollection
{
	mutex mtx;

	const uint32_t n_stream_per_type = 1024;

	vector<map<string, CIF_file_desc_t>> vm_CIF;
	vector<map<string, PDB_file_desc_t>> vm_PDB;
	vector<map<string, PAE_file_desc_t>> vm_PAE;
	vector<map<string, CONF_file_desc_t>> vm_CONF;

	vector<string> v_cmd;

	vector<int> vm_stream_CIF;
	vector<int> vm_stream_PDB;
	vector<int> vm_stream_PAE;
	vector<int> vm_stream_CONF;

	string coll_stream_name(const int id, file_type_t file_type)
	{
		if (file_type == file_type_t::CIF)
			return "col-CIF-" + to_string(id);
		else if (file_type == file_type_t::PDB)
			return "col-PDB-" + to_string(id);
		else if (file_type == file_type_t::PAE)
			return "col-PAE-" + to_string(id);
		else if (file_type == file_type_t::CONF)
			return "col-CONF-" + to_string(id);
		else
			return "col-" + to_string(id);
	}

	shared_ptr<refresh::archive_buffered_io> in_archive;
	shared_ptr<refresh::archive_buffered_io> out_archive;

	uint32_t archive_version = ARCHIVE_VERSION;
	bool archive_version_already_deserialized = false;

	Serializer serializer;
	refresh::zstd_in_memory zim;

	bool serialize_mCIF(uint32_t id, data_t& data_packed);
	bool serialize_mPDB(uint32_t id, data_t& data_packed);
	bool serialize_mPAE(uint32_t id, data_t& data_packed);
	bool serialize_mCONF(uint32_t id, data_t& data_packed);

	bool deserialize_mCIF(uint32_t id, data_t& data_packed);
	bool deserialize_mPDB(uint32_t id, data_t& data_packed);
	bool deserialize_mPAE(uint32_t id, data_t& data_packed);
	bool deserialize_mCONF(uint32_t id, data_t& data_packed);

	// Portable hasher is necessary to support various OS/compilers
	uint64_t hasher(const string& str)
	{
		uint64_t r = str.length();
		for (const auto c : str)
			r = r * 127 + (uint64_t)c;
		
		return r;
	}

public:
	CCollection()
	{
		vm_CIF.resize(n_stream_per_type);
		vm_PDB.resize(n_stream_per_type);
		vm_PAE.resize(n_stream_per_type);
		vm_CONF.resize(n_stream_per_type);
	}
	
	CCollection(const CCollection&) = delete;

	~CCollection() = default;

	bool set_archives(shared_ptr<refresh::archive_buffered_io> _in_archive, shared_ptr<refresh::archive_buffered_io> _out_archive);

	void add_cmd_line(const string& cmd);
	vector<string> get_cmd_lines();
	uint32_t get_archive_version();

	bool is_collection_stream_name(const string& str) const;

	bool serialize();
	bool deserialize();
	bool deserialize_archive_version();
	bool deserialize_init();
	bool deserialize_seg(file_type_t file_type, int id);

	bool close();

	bool empty();

	size_t get_no_entries(const file_type_t file_type);
	
	int add_file_CIF(CIF_file_desc_t& file_desc);
	pair<int, int> add_file_CIF_fast(CIF_file_desc_t& file_desc, data_t &packed);
	int add_file_PDB(PDB_file_desc_t& file_desc);
	pair<int, int> add_file_PDB_fast(PDB_file_desc_t& file_desc, data_t& packed);
	int add_file_PAE(PAE_file_desc_t& file_desc);
	pair<int, int> add_file_PAE_fast(PAE_file_desc_t& file_desc, data_t& packed);
	int add_file_CONF(CONF_file_desc_t& file_desc);
	pair<int, int> add_file_CONF_fast(CONF_file_desc_t& file_desc, data_t& packed);

	int get_file_CIF(const string& name, CIF_file_desc_t& file_desc);
	int get_file_PDB(const string& name, PDB_file_desc_t& file_desc);
	int get_file_PAE(const string& name, PAE_file_desc_t& file_desc);
	int get_file_CONF(const string& name, CONF_file_desc_t& file_desc);

	vector<CIF_file_desc_t> list_files_CIT();
	vector<PDB_file_desc_t> list_files_PDB();
	vector<PAE_file_desc_t> list_files_PAE();
	vector<CONF_file_desc_t> list_files_CONF();

	string stream_name(const int hv, file_type_t file_type)
	{
		if (file_type == file_type_t::CIF)
			return "CIF-" + to_string(hv);
		else if (file_type == file_type_t::PDB)
			return "PDB-" + to_string(hv);
		else if (file_type == file_type_t::PAE)
			return "PAE-" + to_string(hv);
		else if (file_type == file_type_t::CONF)
			return "CONF-" + to_string(hv);
		else
			return "s-" + to_string(hv);
	}

	string stream_name(const string& str, file_type_t file_type)
	{
		uint32_t hv = hasher(str) % n_stream_per_type;

		if (file_type == file_type_t::CIF)
			return "CIF-" + to_string(hv);
		else if (file_type == file_type_t::PDB)
			return "PDB-" + to_string(hv);
		else if (file_type == file_type_t::PAE)
			return "PAE-" + to_string(hv);
		else if (file_type == file_type_t::CONF)
			return "CONF-" + to_string(hv);
		else
			return "s-" + to_string(hv);
	}
};

// EOF
