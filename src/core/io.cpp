#include "../core/utils.h"

#include <zlib.h>

// ************************************************************************************
bool is_gzipped(const string& file_name)
{
	if (file_name.length() < 4)
		return false;

	return file_name.substr(file_name.length() - 3, 3) == ".gz";
}

// EOF
