#include "cif-output.h"
#include "conversion.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

/*****************************************************************************/
size_t CifOutput::store()
{
	char* p = fileBuffer.data();
	LoopEntry::Type prevType = LoopEntry::Type::Standard;

	for (const Entry* e : entries) {
		if (!e->isLoop) {
			const BlockEntry* be = dynamic_cast<const BlockEntry*>(e);
			//			realloc_filebuf_if_necessary(p, be->size);
			//			copy_n(be->data, be->size, p);
			replace_copy(be->data, be->data + be->size, p, StringColumn::MULTILINE_VALUE_MARKER, ';');

			p += be->size;
		}
		else {
			const LoopEntry* le = dynamic_cast<const LoopEntry*>(e);
			//			realloc_filebuf_if_necessary(p);

						// store loop header when current or previous type is standard 
			if (le->getType() == LoopEntry::Type::Standard || prevType == LoopEntry::Type::Standard) {

				p += Conversions::String2PChar("loop_\n", p);

				// save column names 
				for (const auto c : le->getColumns()) {
					//					realloc_filebuf_if_necessary(p);
					p += Conversions::String2PChar(e->name, p);
					*p++ = '.';
					p += Conversions::String2PChar(c->name, p);
					*p++ = '\n';
				}
			}

			// save rows
			for (int ir = 0; ir < le->getRowCount(); ++ir) {
				for (int ic = 0; ic < (int)le->getColumns().size(); ++ic) {
					//					realloc_filebuf_if_necessary(p);
					auto& col = le->getColumns()[ic];

					char* token = p;
					col->output(ir, p);

					// if multiline text field - add a new line
					if (*(token + 1) == ';') {

					}
					// add seperator for unaligned columns
					else if (col->width == 0) {
						*p++ = ' ';
					}
				}
				//				realloc_filebuf_if_necessary(p);
				*p++ = '\n';
			}

			prevType = le->getType();
		}
	}

	fileBufferBytes = p - fileBuffer.data();
	return fileBufferBytes;
}