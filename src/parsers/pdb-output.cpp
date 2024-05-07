#include "pdb-output.h"
#include "entries.h"

#include <algorithm>
#include <string>

using namespace std;

size_t PdbOutput::store() {

	char* p = fileBuffer.data();
	//	char* p0 = p;

	for (int ie = 0; ie < entries.size(); ) {
		if (!entries[ie]->isLoop) {
			const BlockEntry* be = dynamic_cast<const BlockEntry*>(entries[ie]);
			//			realloc_filebuf_if_necessary(p, be->size);
			copy_n(be->data, be->size, p);
			p += be->size;
			++ie;
		}
		else {
			// support of ATOM/HETATM interleaved with SIGATM, ANISOU, SIGUIJ
			std::vector<const LoopEntry*> loops;
			
			while (ie < entries.size()) {
				const LoopEntry* le = dynamic_cast<const LoopEntry*>(entries[ie]);
				if (le != nullptr) {
					loops.push_back(le);
					++ie;
				}
				else {
					break;
				}
			}

			if (loops.size() == 1) {
				// only one loop
				const LoopEntry* le = loops.front();
				// save rows
				for (int ir = 0; ir < le->getRowCount(); ++ir) {
					store_row(le, ir, p);
				}
			}
			else {
				// multiple interleaved loops
				std::vector<int> rowIndices(loops.size());
				std::vector<NumericColumn*> atom_cols(loops.size());
				const int ATOM_COLUMN_ID = 1;
				
				for (int i = 0; i < loops.size(); ++i) {
					atom_cols[i] = dynamic_cast<NumericColumn*>(loops[i]->getColumns()[ATOM_COLUMN_ID]);
				}
				
				while (true) {

					// get smallest atom id from loops
					std::pair<int, int> mini(-1, std::numeric_limits<int>::max());

					for (int i = 0; i < rowIndices.size(); ++i) {
						int ir = rowIndices[i];
						int val = atom_cols[i]->getValueAtRow(ir); // return MAX_INT if out of range

						if (val < mini.second) {
							mini.first = i;
							mini.second = val;
						}
					}

					// no more rows to output
					if (mini.first == -1) {
						break;
					}

					store_row(loops[mini.first], rowIndices[mini.first], p);
					++rowIndices[mini.first];
				}
			}
		}
	}

	fileBufferBytes = p - fileBuffer.data();
	return fileBufferBytes;
}

void PdbOutput::store_row(const LoopEntry* entry, int ir, char*& p) {
	
	auto type = entry->getType();

	// rescue mode was used
	if (type == LoopEntry::Type::Standard) {
		type = LoopEntry::str2type(entry->name);
	}

	if (type != LoopEntry::Type::Standard) {
		const auto& col_defs = COLUMN_DEFS.at(type);

		char* beg = p;
		for (int ic = 0; ic < (int)entry->getColumns().size(); ++ic) {
			//					realloc_filebuf_if_necessary(p);
			while (p < beg + col_defs[ic].start) {
				*p = ' ';
				++p;
			}

			auto& column = entry->getColumns()[ic];
			//					realloc_filebuf_if_necessary(p);
			store_element(column, ir, p);
		}
		*p++ = '\n';
	}
}

void PdbOutput::store_element(const AbstractColumn* column, int ir, char*& p) {
	// special handling of atom id column
	if (column->name == Columns::auth_atom_id) {
		char buf[16];
		char* tmp = buf;
		column->output(ir, tmp);
		*tmp = 0;

		tmp = buf;
		int len = 0;

		// omit preceding spaces if any
		while (*tmp == ' ') {
			++tmp;
		}

		char* s = tmp;
		
		// measure length (do not put space if equals to 4)
		while (*tmp != 0 && *tmp != ' ') {
			++tmp;
			++len;
		}

		tmp = s;

		if (len < 4) {
			// one-symbol chemical elements - insert space to output
			if (*tmp == 'N' || *tmp == 'C' || *tmp == 'O' || (tmp[0] == 'S' && tmp[1] != 'E') || *tmp == 'P' || *tmp == 'I' || *tmp == 'H' ||
				(tmp[0] == 'F' && tmp[1] != 'E')) { // not an iron 
				*p = ' ';
				++p;
			}
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