#include "pdb.h"
#include "entries.h"

#include <algorithm>
#include <string>

using namespace std;

void Pdb::parse() {
	char* fileEnd = fileBuffer.data() + fileBufferBytes;
	char* p = fileBuffer.data();

	std::vector<char*> rows;
	rows.reserve(count(p, fileEnd, '\n'));

	while (p < fileEnd) {
		
		string sectionName(p, std::find(p, p + 6, ' '));
		
		bool need_ommit = minimal_mode && !MINIMAL_SECTIONS.count(sectionName);
		auto type = LoopEntry::str2type(sectionName);

		if (type != LoopEntry::Type::Standard) {

			LoopEntry* entry = new LoopEntry(sectionName, type);

//			char* section = p;
			
			// get row pointers
			rows.clear();
			rows.push_back(p);
			
			bool ignore = false;
			bool agree = false;

			int lineLength = (int)(find(p, fileEnd, '\n') - p);

			do {
				char* q = find(p, fileEnd, '\n');
				
				// incorrect line length
				if (q - p != lineLength) {
					throw std::runtime_error("Invalid PDB file");
				}
				p = q;

				++p; // go to next line
				string localName(p, find(p, p + 6, ' '));
				
				ignore = IGNORED_SECTIONS.count(localName);
				agree = localName == sectionName;
				if (!ignore && agree) {
					rows.push_back(p);
				}
			
			} while (ignore || agree);



			for (const auto& def : COLUMN_DEFS) {

				AbstractColumn* col = nullptr;
				if (!def.isNumeric) {
					// determine special columns
					char*& dst = dataBufferPos;
					col = new StringColumn(def.name, rows, def.width, def.start, dst);
				}
				else {
					col = NumericColumn::create(def.name, rows, def.width, def.start);
					if (col == nullptr) {
						char*& dst = dataBufferPos;
						col = new StringColumn(def.name, rows, def.width, def.start, dst);
						// rescue mode - change loop type to standard
						entry->setType(LoopEntry::Type::Standard); 
					}
				}
				entry->addColumn(col);
			}

			addEntry(entry);
		}
		else {
			// process block section
			char* section = p;
			
			do {
				p = find(p, fileEnd, '\n');
				
				// go to next line
				if (p != fileEnd) {
					++p;
				}

			} while (std::equal(p, p + sectionName.size(), sectionName.c_str()));

			if (need_ommit)
				continue;

			int lineLength = (int)(find(p, fileEnd, '\n') - p);

			copy(section, p, dataBufferPos);
			size_t size = p - section;
			BlockEntry* entry = new BlockEntry(
				string(section, find(section, section + lineLength, ' ')),
				size,
				dataBufferPos);
			addEntry(entry);
		}
	}
}


size_t Pdb::store() {

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
				for (int ic = 0; ic < (int) le->getColumns().size(); ++ic) {
//					realloc_filebuf_if_necessary(p);
					while (p < beg + COLUMN_DEFS[ic].start) {
						*p = ' ';
						++p;
					}
					
//					realloc_filebuf_if_necessary(p);
					le->getColumns()[ic]->output(ir, p);
				}
				*p++ = '\n';
			}
		}
	}

	fileBufferBytes = p - fileBuffer.data();
	return fileBufferBytes;
}