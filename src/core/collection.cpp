#include "collection.h"

// *****************************************************************
void file_desc_t::serialize(data_t& data)
{
	data.insert(data.end(), name.begin(), name.end());
	data.emplace_back(0);
	SerializerStatic::append64u((uint64_t)raw_size, data);
	SerializerStatic::append32u((uint32_t)id, data);
}

// *****************************************************************
void file_desc_t::deserialize(data_t::iterator& p)
{
	file_type = file_type_t::none;

	name.clear();

	for (; *p != 0; ++p)
		name.push_back(*p);

	++p;			// skip null-terminator

	raw_size = SerializerStatic::load64u(p);
	id = SerializerStatic::load32u(p);
}

// *****************************************************************
void CIF_file_desc_t::serialize(data_t& data)
{
	file_desc_t::serialize(data);

	if (lossy)
	{
		data.emplace_back(max_error_bb & 0xffu);
		data.emplace_back(max_error_sc & 0xffu);
	}
	else
		data.emplace_back(255);

	uint8_t flags = 0;
	if (minimal)
		flags += 1;
	if (single_bf)
		flags += 2;

	data.emplace_back(flags);
}

// *****************************************************************
void CIF_file_desc_t::deserialize(data_t::iterator& p)
{
	file_desc_t::deserialize(p);

	if (*p == 255)
	{
		lossy = false;
		++p;
	}
	else
	{
		lossy = true;
		max_error_bb = (int)*p++;
		max_error_sc = (int)*p++;
	}

	uint8_t flags = *p++;

	minimal = (bool)(flags & 1);
	single_bf = (bool)(flags & 2);

	file_type = file_type_t::CIF;
}

// *****************************************************************
void PDB_file_desc_t::serialize(data_t& data)
{
	CIF_file_desc_t::serialize(data);

	file_type = file_type_t::PDB;
}

// *****************************************************************
void PDB_file_desc_t::deserialize(data_t::iterator& p)
{
	CIF_file_desc_t::deserialize(p);
}

// *****************************************************************
void PAE_file_desc_t::serialize(data_t& data)
{
	file_desc_t::serialize(data);
	data.emplace_back((char) lossy_level);
}

// *****************************************************************
void PAE_file_desc_t::deserialize(data_t::iterator& p)
{
	file_desc_t::deserialize(p);
	lossy_level = *p++;

	file_type = file_type_t::PAE;
}

// *****************************************************************
void CONF_file_desc_t::serialize(data_t& data)
{
	file_desc_t::serialize(data);
}

// *****************************************************************
void CONF_file_desc_t::deserialize(data_t::iterator& p)
{
	file_desc_t::deserialize(p);

	file_type = file_type_t::CONF;
}

// *****************************************************************
//
// *****************************************************************
bool CCollection::empty()
{
	return vm_CIF.empty() && vm_PAE.empty() && vm_PDB.empty() && vm_CONF.empty();
}

// *****************************************************************
bool CCollection::set_archives(shared_ptr<refresh::archive_buffered_io> _in_archive, shared_ptr<refresh::archive_buffered_io> _out_archive)
{
	lock_guard<mutex> lck(mtx);

	in_archive = _in_archive;
	out_archive = _out_archive;

	archive_version_already_deserialized = false;

	return true;
}

// *****************************************************************
size_t CCollection::get_no_entries(const file_type_t file_type)
{
	lock_guard<mutex> lck(mtx);

	size_t r = 0;

	if (file_type == file_type_t::CIF)
		for (const auto& x : vm_CIF)
			r += x.size();
	else if (file_type == file_type_t::PDB)
		for (const auto& x : vm_PDB)
			r += x.size();
	else if (file_type == file_type_t::PAE)
		for (const auto& x : vm_PAE)
			r += x.size();
	else if (file_type == file_type_t::CONF)
		for (const auto& x : vm_CONF)
			r += x.size();

	return r;
}

// *****************************************************************
bool CCollection::serialize_mCIF(uint32_t id, data_t& data_packed)
{
	data_packed.clear();

	auto& m_col = vm_CIF[id];

	if (m_col.empty())
		return false;

	vector<CIF_file_desc_t> tmp;
	tmp.resize(m_col.size());

	for (const auto& x : m_col)
		tmp[x.second.id] = x.second;

	for (auto& x : tmp)
		x.serialize(data_packed);

	return true;
}

// *****************************************************************
bool CCollection::serialize_mPDB(uint32_t id, data_t& data_packed)
{
	data_packed.clear();

	auto& m_col = vm_PDB[id];

	if (m_col.empty())
		return false;

	vector<PDB_file_desc_t> tmp;
	tmp.resize(m_col.size());

	for (const auto& x : m_col)
		tmp[x.second.id] = x.second;

	for (auto& x : tmp)
		x.serialize(data_packed);

	return true;
}

// *****************************************************************
bool CCollection::serialize_mPAE(uint32_t id, data_t& data_packed)
{
	data_packed.clear();

	auto& m_col = vm_PAE[id];

	if (m_col.empty())
		return false;

	vector<PAE_file_desc_t> tmp;
	tmp.resize(m_col.size());

	for (const auto& x : m_col)
		tmp[x.second.id] = x.second;

	for (auto& x : tmp)
		x.serialize(data_packed);

	return true;
}

// *****************************************************************
bool CCollection::serialize_mCONF(uint32_t id, data_t& data_packed)
{
	data_packed.clear();

	auto& m_col = vm_CONF[id];

	if (m_col.empty())
		return false;

	vector<CONF_file_desc_t> tmp;
	tmp.resize(m_col.size());

	for (const auto& x : m_col)
		tmp[x.second.id] = x.second;

	for (auto& x : tmp)
		x.serialize(data_packed);

	return true;
}

// *****************************************************************
bool CCollection::deserialize_mCIF(uint32_t id, data_t& data_packed)
{
//	lock_guard<mutex> lck(mtx);

	auto& m_col = vm_CIF[id];

	m_col.clear();

	auto p = data_packed.begin();
	
	CIF_file_desc_t cd;

	while (p != data_packed.end())
	{
		cd.deserialize(p);
		m_col[cd.name] = cd;
	}

	return true;
}

// *****************************************************************
bool CCollection::deserialize_mPDB(uint32_t id, data_t& data_packed)
{
//	lock_guard<mutex> lck(mtx);

	auto& m_col = vm_PDB[id];

	m_col.clear();

	auto p = data_packed.begin();

	PDB_file_desc_t cd;

	while (p != data_packed.end())
	{
		cd.deserialize(p);
		m_col[cd.name] = cd;
	}

	return true;
}

// *****************************************************************
bool CCollection::deserialize_mPAE(uint32_t id, data_t& data_packed)
{
//	lock_guard<mutex> lck(mtx);

	auto& m_col = vm_PAE[id];

	m_col.clear();

	auto p = data_packed.begin();

	PAE_file_desc_t cd;

	while (p != data_packed.end())
	{
		cd.deserialize(p);
		m_col[cd.name] = cd;
	}

	return true;
}

// *****************************************************************
bool CCollection::deserialize_mCONF(uint32_t id, data_t& data_packed)
{
//	lock_guard<mutex> lck(mtx);

	auto& m_col = vm_CONF[id];

	m_col.clear();

	auto p = data_packed.begin();

	CONF_file_desc_t cd;

	while (p != data_packed.end())
	{
		cd.deserialize(p);
		m_col[cd.name] = cd;
	}

	return true;
}

// *****************************************************************
bool CCollection::serialize()
{
	lock_guard<mutex> lck(mtx);

	if (!out_archive)
		return false;

	data_t data_ser;

	for (uint32_t i = 0; i < n_stream_per_type; ++i)
	{
		if (serialize_mCIF(i, data_ser))
		{
			auto stream_id = out_archive->register_stream(coll_stream_name(i, file_type_t::CIF));
			out_archive->add_part(stream_id, data_ser);
		}
		if (serialize_mPDB(i, data_ser))
		{
			auto stream_id = out_archive->register_stream(coll_stream_name(i, file_type_t::PDB));
			out_archive->add_part(stream_id, data_ser);
		}
		if (serialize_mPAE(i, data_ser))
		{
			auto stream_id = out_archive->register_stream(coll_stream_name(i, file_type_t::PAE));
			out_archive->add_part(stream_id, data_ser);
		}
		if (serialize_mCONF(i, data_ser))
		{
			auto stream_id = out_archive->register_stream(coll_stream_name(i, file_type_t::CONF));
			out_archive->add_part(stream_id, data_ser);
		}
	}

	Serializer ser;
	ser.append32u(archive_version);
	ser.append_vec_str(v_cmd);

	auto stream_id = out_archive->register_stream("ProteStAr-global");
	out_archive->add_part(stream_id, ser.get_data(), v_cmd.size());

	return true;
}

// *****************************************************************
bool CCollection::deserialize_archive_version()
{
	lock_guard<mutex> lck(mtx);

	if (!in_archive)
		return false;

	Serializer ser;
	data_t global_data;
	auto stream_id = in_archive->get_stream_id("ProteStAr-global");

	if (stream_id < 0)
		return false;

	uint64_t metadata;
	in_archive->get_part(stream_id, global_data, metadata);

	ser.set_data(move(global_data));
	archive_version = ser.load32u();
	v_cmd.clear();
	ser.load_vec_str(v_cmd, metadata);

	archive_version_already_deserialized = true;

	return true;
}

// *****************************************************************
bool CCollection::deserialize()
{
	lock_guard<mutex> lck(mtx);

	if (!in_archive)
		return false;

	vm_CIF.clear();
	vm_PDB.clear();
	vm_PAE.clear();
	vm_CONF.clear();

	vm_CIF.resize(n_stream_per_type);
	vm_PDB.resize(n_stream_per_type);
	vm_PAE.resize(n_stream_per_type);
	vm_CONF.resize(n_stream_per_type);

	data_t data_ser;
	uint64_t metadata;

	for (uint32_t i = 0; i < n_stream_per_type; ++i)
	{
		auto stream_id = in_archive->get_stream_id(coll_stream_name(i, file_type_t::CIF));

		if (stream_id >= 0)
		{
			if (!in_archive->get_part(stream_id, data_ser, metadata))
				return false;
			deserialize_mCIF(i, data_ser);
		}

		stream_id = in_archive->get_stream_id(coll_stream_name(i, file_type_t::PDB));

		if (stream_id >= 0)
		{
			if (!in_archive->get_part(stream_id, data_ser, metadata))
				return false;
			deserialize_mPDB(i, data_ser);
		}

		stream_id = in_archive->get_stream_id(coll_stream_name(i, file_type_t::PAE));

		if (stream_id >= 0)
		{
			if (!in_archive->get_part(stream_id, data_ser, metadata))
				return false;
			deserialize_mPAE(i, data_ser);
		}

		stream_id = in_archive->get_stream_id(coll_stream_name(i, file_type_t::CONF));

		if (stream_id >= 0)
		{
			if (!in_archive->get_part(stream_id, data_ser, metadata))
				return false;
			deserialize_mCONF(i, data_ser);
		}
	}

	if (archive_version_already_deserialized)
		return true;

	Serializer ser;
	data_t global_data;
	auto stream_id = in_archive->get_stream_id("ProteStAr-global");

	if (stream_id < 0)
		return false;

	in_archive->get_part(stream_id, global_data, metadata);

	ser.set_data(move(global_data));
	archive_version = ser.load32u();
	v_cmd.clear();
	ser.load_vec_str(v_cmd, metadata);

	return true;
}

// *****************************************************************
bool CCollection::deserialize_init()
{
	lock_guard<mutex> lck(mtx);

	if (!in_archive)
		return false;

	vm_CIF.clear();
	vm_PDB.clear();
	vm_PAE.clear();
	vm_CONF.clear();

	vm_CIF.resize(n_stream_per_type);
	vm_PDB.resize(n_stream_per_type);
	vm_PAE.resize(n_stream_per_type);
	vm_CONF.resize(n_stream_per_type);

	data_t data_ser;
	uint64_t metadata;

	Serializer ser;
	data_t global_data;
	auto stream_id = in_archive->get_stream_id("ProteStAr-global");

	if (stream_id < 0)
		return false;

	in_archive->get_part(stream_id, global_data, metadata);

	ser.set_data(move(global_data));
	archive_version = ser.load32u();
	v_cmd.clear();
	ser.load_vec_str(v_cmd, metadata);

	return true;
}

// *****************************************************************
int CCollection::add_file_CIF(CIF_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(file_desc.name) % n_stream_per_type;
	auto& m_col = vm_CIF[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return -1;							// already exists

	file_desc.id = (int) m_col.size();

	m_col[file_desc.name] = file_desc;

	return (int) hv;
}

// *****************************************************************
pair<int, int> CCollection::add_file_CIF_fast(CIF_file_desc_t& file_desc, data_t& packed)
{
	lock_guard<mutex> lck(mtx);

	int ret_stream_id = -1;
	int ret_part_id = -1;

	auto hv = hasher(file_desc.name) % n_stream_per_type;

	if (vm_stream_CIF.empty())
		vm_stream_CIF.resize(n_stream_per_type, -1);

	if (vm_stream_CIF[hv] < 0)
		vm_stream_CIF[hv] = out_archive->register_stream(stream_name((int) hv, file_type_t::CIF));

	ret_stream_id = vm_stream_CIF[hv];

	auto& m_col = vm_CIF[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return make_pair(-1, -1);							// already exists

	file_desc.id = (int) m_col.size();

	m_col[file_desc.name] = file_desc;

	ret_part_id = out_archive->add_part(ret_stream_id, packed);
	
	return make_pair(ret_stream_id, ret_part_id);
}

// *****************************************************************
pair<int, int> CCollection::add_file_PDB_fast(PDB_file_desc_t& file_desc, data_t& packed)
{
	lock_guard<mutex> lck(mtx);

	int ret_stream_id = -1;
	int ret_part_id = -1;

	// !!! Tymczasowo tu rozszerzanie
	if (vm_stream_PDB.empty())
		vm_stream_PDB.resize(n_stream_per_type, -1);

	auto hv = hasher(file_desc.name) % n_stream_per_type;

	if (vm_stream_PDB[hv] < 0)
		vm_stream_PDB[hv] = out_archive->register_stream(stream_name((int) hv, file_type_t::PDB));

	ret_stream_id = vm_stream_PDB[hv];

	auto& m_col = vm_PDB[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return make_pair(-1, -1);							// already exists

	file_desc.id = (int) m_col.size();

	m_col[file_desc.name] = file_desc;

	ret_part_id = out_archive->add_part(ret_stream_id, packed);
	
	return make_pair(ret_stream_id, ret_part_id);
}

// *****************************************************************
int CCollection::add_file_PDB(PDB_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(file_desc.name) % n_stream_per_type;
	auto& m_col = vm_PDB[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return -1;							// already exists

	file_desc.id = (int) m_col.size();

	m_col[file_desc.name] = file_desc;

	return (int) hv;
}

// *****************************************************************
pair<int, int> CCollection::add_file_PAE_fast(PAE_file_desc_t& file_desc, data_t& packed)
{
	lock_guard<mutex> lck(mtx);

	int ret_stream_id = -1;
	int ret_part_id = -1;

	// !!! Tymczasowo tu rozszerzanie
	if (vm_stream_PAE.empty())
		vm_stream_PAE.resize(n_stream_per_type, -1);

	auto hv = hasher(file_desc.name) % n_stream_per_type;

	if (vm_stream_PAE[hv] < 0)
		vm_stream_PAE[hv] = out_archive->register_stream(stream_name((int)hv, file_type_t::PAE));

	ret_stream_id = vm_stream_PAE[hv];

	auto& m_col = vm_PAE[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return make_pair(-1, -1);							// already exists

	file_desc.id = (int)m_col.size();

	m_col[file_desc.name] = file_desc;

	ret_part_id = out_archive->add_part(ret_stream_id, packed);

	return make_pair(ret_stream_id, ret_part_id);
}

// *****************************************************************
int CCollection::add_file_PAE(PAE_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(file_desc.name) % n_stream_per_type;
	auto& m_col = vm_PAE[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return -1;							// already exists

	file_desc.id = (int) m_col.size();

	m_col[file_desc.name] = file_desc;

	return (int) hv;
}

// *****************************************************************
pair<int, int> CCollection::add_file_CONF_fast(CONF_file_desc_t& file_desc, data_t& packed)
{
	lock_guard<mutex> lck(mtx);

	int ret_stream_id = -1;
	int ret_part_id = -1;

	// !!! Tymczasowo tu rozszerzanie
	if (vm_stream_CONF.empty())
		vm_stream_CONF.resize(n_stream_per_type, -1);

	auto hv = hasher(file_desc.name) % n_stream_per_type;

	if (vm_stream_CONF[hv] < 0)
		vm_stream_CONF[hv] = out_archive->register_stream(stream_name((int)hv, file_type_t::CONF));

	ret_stream_id = vm_stream_CONF[hv];

	auto& m_col = vm_CONF[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return make_pair(-1, -1);							// already exists

	file_desc.id = (int)m_col.size();

	m_col[file_desc.name] = file_desc;

	ret_part_id = out_archive->add_part(ret_stream_id, packed);

	return make_pair(ret_stream_id, ret_part_id);
}

// *****************************************************************
int CCollection::add_file_CONF(CONF_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(file_desc.name) % n_stream_per_type;
	auto& m_col = vm_CONF[hv];

	auto p = m_col.find(file_desc.name);
	if (p != m_col.end())
		return -1;							// already exists

	file_desc.id = (int)m_col.size();

	m_col[file_desc.name] = file_desc;

	return (int)hv;
}

// *****************************************************************
int CCollection::get_file_CIF(const string& name, CIF_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(name) % n_stream_per_type;
	auto& m_col = vm_CIF[hv];

	auto p = m_col.find(name);
	if (p == m_col.end())
		return -1;							// unknown file

	file_desc = p->second;

	return (int) hv;
}

// *****************************************************************
int CCollection::get_file_PDB(const string& name, PDB_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(name) % n_stream_per_type;
	auto& m_col = vm_PDB[hv];

	auto p = m_col.find(name);
	if (p == m_col.end())
		return -1;							// unknown file

	file_desc = p->second;

	return (int) hv;
}

// *****************************************************************
int CCollection::get_file_PAE(const string& name, PAE_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(name) % n_stream_per_type;
	auto& m_col = vm_PAE[hv];

	auto p = m_col.find(name);
	if (p == m_col.end())
		return -1;							// unknown file

	file_desc = p->second;

	return (int) hv;
}

// *****************************************************************
int CCollection::get_file_CONF(const string& name, CONF_file_desc_t& file_desc)
{
	lock_guard<mutex> lck(mtx);

	auto hv = hasher(name) % n_stream_per_type;
	auto& m_col = vm_CONF[hv];

	auto p = m_col.find(name);
	if (p == m_col.end())
		return -1;							// unknown file

	file_desc = p->second;

	return (int) hv;
}

// *****************************************************************
vector<CIF_file_desc_t> CCollection::list_files_CIT()
{
	vector<CIF_file_desc_t> vec;

	vec.reserve(get_no_entries(file_type_t::CIF));

	for (const auto& m_col : vm_CIF)
		for (const auto& x : m_col)
			vec.emplace_back(x.second);

	return vec;
}

// *****************************************************************
vector<PDB_file_desc_t> CCollection::list_files_PDB()
{
	vector<PDB_file_desc_t> vec;

	vec.reserve(get_no_entries(file_type_t::PDB));

	for (const auto& m_col : vm_PDB)
		for (const auto& x : m_col)
			vec.emplace_back(x.second);

	return vec;
}

// *****************************************************************
vector<PAE_file_desc_t> CCollection::list_files_PAE()
{
	vector<PAE_file_desc_t> vec;

	vec.reserve(get_no_entries(file_type_t::PAE));

	for (const auto& m_col : vm_PAE)
		for (const auto& x : m_col)
			vec.emplace_back(x.second);

	return vec;
}

// *****************************************************************
vector<CONF_file_desc_t> CCollection::list_files_CONF()
{
	vector<CONF_file_desc_t> vec;

	vec.reserve(get_no_entries(file_type_t::CONF));

	for (const auto& m_col : vm_CONF)
		for (const auto& x : m_col)
			vec.emplace_back(x.second);

	return vec;
}

// *****************************************************************
bool CCollection::close()
{
	lock_guard<mutex> lck(mtx);

	bool r = true;

	if (in_archive)
		in_archive.reset();

	if (out_archive)
	{
//		r &= serialize();
		out_archive.reset();
	}

	archive_version_already_deserialized = false;

	return r;
}

// *****************************************************************
bool CCollection::is_collection_stream_name(const string& str) const
{
	if (str.length() > 5)
	{
		string prefix = str.substr(0, 5);
		if (prefix == "coll-")
			return true;
	}

	return false;
}

// *****************************************************************
void CCollection::add_cmd_line(const string& cmd)
{
	lock_guard<mutex> lck(mtx);

	auto tc = time(nullptr);
	char tmp[64];
	string s_time;

	if (strftime(tmp, sizeof(tmp), "%A %c", std::gmtime(&tc)))
		s_time = tmp;

	v_cmd.push_back(s_time + " : " + cmd);
}

// *****************************************************************
vector<string> CCollection::get_cmd_lines()
{
	return v_cmd;
}

// *****************************************************************
uint32_t CCollection::get_archive_version()
{
	if (empty())
		return 0;

	return archive_version;
}

// EOF
