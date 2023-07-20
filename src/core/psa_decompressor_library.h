#pragma once

//#include <archive.h>
#include <memory>
#include <thread>
#include <atomic>

#include "psa_base.h"
#include "params.h"
#include "collection.h"

using namespace refresh;

// *****************************************************************
//
// *****************************************************************
class CPSADecompressorLibrary : public CPSABase
{
protected:
	bool is_app_mode;

	bool GetCIFData(const string& file_name, vector<char>& file_data);
	bool GetPDBData(const string& file_name, vector<char>& file_data);
	bool GetPAEData(const string& file_name, vector<char>& file_data);
	bool GetCONFData(const string& file_name, vector<char>& file_data);

public:
	CPSADecompressorLibrary(CParams& params, bool is_app_mode) :
		CPSABase(params),
		is_app_mode(is_app_mode)
	{}

	bool GetCmdLines(vector<string>& v_cmd);
	uint32_t GetArchiveVersion();
	bool GetNoFiles(map<file_type_t, size_t>& no_files);
	bool ListFiles(file_type_t file_type, vector<string>& file_list);
	bool ListFilesExt(file_type_t file_type, vector<pair<string, file_type_t>>& file_list_ext);
	bool GetFilesInfo(file_type_t file_type, vector<unique_ptr<file_desc_t>>& file_desc);
	bool GetFileData(file_type_t file_type, const string& file_name, vector<char> &file_data);

	bool DeserializeArchiveVersion();
	bool Deserialize();
	bool DeserializeInit();

	bool close();
};

// EOF