#pragma once

#include <archive.h>
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
class CPSACompressor : public CPSABase
{
	shared_ptr<archive_buffered_io> out_archive;

public:
	CPSACompressor(CParams& params) :
		CPSABase(params)
	{}

	bool create(const string& file_name);
	bool copy_from_existing();
	bool add_new();
	bool close();
};

// EOF