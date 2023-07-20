#include "../core/psa_decompressor_library.h"
#include "protestar-api.h"

// *****************************************************************
CPSAFile::CPSAFile()
{
	psa = nullptr;
//	psa = std::make_unique<CPSADecompressorLibrary>(false);
	is_opened = false;
}

// *****************************************************************
CPSAFile::~CPSAFile()
{
}

// *****************************************************************
/**
* Open archive
*
* @param file_name		file name
* @param prefetching	true to preload whole file into memory (faster if you plan series of sequence queries), false otherwise
*
* @return false for error
*/
bool CPSAFile::Open(const std::string& file_name, bool prefetching)
{
	CParams params;

	params.input_archive = file_name;
	params.working_mode = working_mode_t::info;

	psa = std::make_unique<CPSADecompressorLibrary>(params, false);

	is_opened = psa->open_existing(file_name);

	if (is_opened)
		is_opened = psa->Deserialize();

	return is_opened;
}

// *****************************************************************
/**
* Close archive
*
* @return			true for success and false for error
*/
bool CPSAFile::Close()
{
	if (!is_opened)
		return false;

	psa->close();
	is_opened = true;

	return true;
}

// *****************************************************************
/**
* Return no. of files of given type
*
* @return			no. of files of given type
*/
size_t CPSAFile::GetNoFilesCIF() const
{
	if (!is_opened)
		return 0;

	map<file_type_t, size_t> no_files;

	psa->GetNoFiles(no_files);

	auto p = no_files.find(file_type_t::CIF);

	if (p != no_files.end())
		return p->second;

	return 0;
}

// *****************************************************************
size_t CPSAFile::GetNoFilesPDB() const
{
	if (!is_opened)
		return 0;

	map<file_type_t, size_t> no_files;

	psa->GetNoFiles(no_files);

	auto p = no_files.find(file_type_t::PDB);

	if (p != no_files.end())
		return p->second;

	return 0;
}

// *****************************************************************
size_t CPSAFile::GetNoFilesPAE() const
{
	if (!is_opened)
		return 0;

	map<file_type_t, size_t> no_files;

	psa->GetNoFiles(no_files);

	auto p = no_files.find(file_type_t::PAE);

	if (p != no_files.end())
		return p->second;

	return 0;
}

// *****************************************************************
size_t CPSAFile::GetNoFilesCONF() const
{
	if (!is_opened)
		return 0;

	map<file_type_t, size_t> no_files;

	psa->GetNoFiles(no_files);

	auto p = no_files.find(file_type_t::CONF);

	if (p != no_files.end())
		return p->second;

	return 0;
}

// *****************************************************************
/**
* List file names of given type
*
* @param sample		list of file names of given type (return value)
*
* @return			true for success and false for error
*/
bool CPSAFile::ListFilesCIF(std::vector<std::string>& samples)
{
	samples.clear();

	if (!is_opened)
		return false;

	return psa->ListFiles(file_type_t::CIF, samples);
}

// *****************************************************************
bool CPSAFile::ListFilesPDB(std::vector<std::string>& samples)
{
	samples.clear();

	if (!is_opened)
		return false;

	return psa->ListFiles(file_type_t::PDB, samples);
}

// *****************************************************************
bool CPSAFile::ListFilesPAE(std::vector<std::string>& samples)
{
	samples.clear();

	if (!is_opened)
		return false;

	return psa->ListFiles(file_type_t::PAE, samples);
}

// *****************************************************************
bool CPSAFile::ListFilesCONF(std::vector<std::string>& samples)
{
	samples.clear();

	if (!is_opened)
		return false;

	return psa->ListFiles(file_type_t::CONF, samples);
}

// *****************************************************************
/**
* Retrive contents of asked file and type
*
* @file_name		file name to decompress
* @param data		contents of the asked file (return value)
*
* @return			true for success and false for error
*/
bool CPSAFile::GetFileCIF(const std::string& file_name, std::vector<char>& data)
{
	if (!is_opened)
		return false;

	return psa->GetFileData(file_type_t::CIF, file_name, data);
}

// *****************************************************************
bool CPSAFile::GetFilePDB(const std::string& file_name, std::vector<char>& data)
{
	if (!is_opened)
		return false;

	return psa->GetFileData(file_type_t::PDB, file_name, data);
}

// *****************************************************************
bool CPSAFile::GetFilePAE(const std::string& file_name, std::vector<char>& data)
{
	if (!is_opened)
		return false;

	return psa->GetFileData(file_type_t::PAE, file_name, data);
}

// *****************************************************************
bool CPSAFile::GetFileCONF(const std::string& file_name, std::vector<char>& data)
{
	if (!is_opened)
		return false;

	return psa->GetFileData(file_type_t::CONF, file_name, data);
}


// *****************************************************************
// Privates
// *****************************************************************


// EOF