#pragma once

#include <archive.h>
#include <memory>
#include <thread>
#include <atomic>

#include "psa_decompressor_library.h"
#include "params.h"
#include "collection.h"

using namespace refresh;

// *****************************************************************
//
// *****************************************************************
class CPSADecompressor : public CPSADecompressorLibrary
{
public:
	CPSADecompressor(CParams& params, bool is_app_mode) :
		CPSADecompressorLibrary(params, is_app_mode)
	{}

	bool get_files();
};


// EOF