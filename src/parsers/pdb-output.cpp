#include "pdb-output.h"
#include "entries.h"

#include <algorithm>
#include <string>

using namespace std;

size_t PdbOutput::store() {

	char* p = fileBuffer.data();
	//	char* p0 = p;

	for (const Entry* e : entries) {
		if (!e->isLoop) {
			const BlockEntry* be = dynamic_cast<const BlockEntry*>(e);
			//			realloc_filebuf_if_necessary(p, be->size);
			copy_n(be->data, be->size, p);
			p += be->size;
		}
		else {
			const LoopEntry* le = dynamic_cast<const LoopEntry*>(e);

			// save rows
			for (int ir = 0; ir < le->getRowCount(); ++ir) {
				char* beg = p;
				for (int ic = 0; ic < (int)le->getColumns().size(); ++ic) {
					//					realloc_filebuf_if_necessary(p);
					while (p < beg + COLUMN_DEFS[ic].start) {
						*p = ' ';
						++p;
					}

					auto& column = le->getColumns()[ic];

					//					realloc_filebuf_if_necessary(p);

										// special handling of atom id column
					if (column->name == Columns::auth_atom_id) {
						char buf[16];
						char* tmp = buf;
						column->output(ir, tmp);
						*tmp = 0;

						tmp = buf;
						// omit preceding spaces if any
						while (*tmp == ' ') {
							++tmp;
						}

						// one-symbol chemical elements - insert space to output
						if (*tmp == 'N' || *tmp == 'C' || *tmp == 'O' || *tmp == 'S' || *tmp == 'P' || *tmp == 'I' ||
							(tmp[0] == 'F' && tmp[1] != 'E')) { // not an iron
							*p = ' ';
							++p;
						}

						// copy to output buffer
						while (*tmp != 0 && *tmp != ' ') {
							*p = *tmp;
							++p;
							++tmp;
						}
					}
					else {
						column->output(ir, p);
					}

				}
				*p++ = '\n';
			}
		}
	}

	fileBufferBytes = p - fileBuffer.data();
	return fileBufferBytes;
}