#include "psa_base.h"

// *****************************************************************
bool CPSABase::open_existing(const string& file_name, size_t buffer_size)
{
	in_archive = make_shared<refresh::archive_buffered_io>(true, buffer_size);

	bool r = in_archive->open(file_name);

	if (!r)
		in_archive.reset();

	return r;
}

// EOF
