#pragma once

#include <archive.h>
#include <memory>
#include <thread>
#include <atomic>

#include "params.h"
#include "collection.h"

using namespace refresh;

// *****************************************************************
//
// *****************************************************************
class CPSABase
{
protected:
	CParams& params;

	shared_ptr<archive_buffered_io> in_archive;

	CCollection collection;

	void log(const string& str, int min_verbose_level = 1, bool no_eol = false)
	{
		if (params.verbose >= min_verbose_level)
			cerr << str + (no_eol ? ""s : "\n"s);
	}

public:
	CPSABase(CParams& params) :
		params(params) 
	{}

	bool open_existing(const string& file_name, size_t buffer_size = 1 << 20);
};

// EOF