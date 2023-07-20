#pragma once

#include <string>
#include <vector>

using namespace std;

enum class working_mode_t { none, add, get, list, info };
enum class file_type_t { CIF = 0, PDB = 1, PAE = 2, CONF = 3, other = 4, ALL = 5, none = 6};
const uint32_t no_file_types = 3;

// *****************************************************************
struct CParams
{
	working_mode_t working_mode = working_mode_t::none;
	file_type_t file_type = file_type_t::none;
	string input_archive;
	string output_archive;
	string infile;
	string indir;
	string indir_recursive;
	string inlist;
	string intar;
	string outdir;
	vector<pair<string, file_type_t>> input_file_name_types;
	int no_threads = 0;
	bool minimal = false;
	bool lossy = false;
	bool max_compression = true;
	int max_error_bb = 0;
	int max_error_sc = 0;
	int pae_lossy_level = 0;
	bool single_bf = false;
	int verbose = 1;
	bool show_file_info = false;
	bool get_all = false;

	string cmd;
};

// EOF