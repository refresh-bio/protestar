#include "pdb-input.h"
#include "entries.h"

#include <algorithm>
#include <string>

using namespace std;

void PdbInput::parse() {
	
	const int SECTION_NAME_LEN = 6;
	
	char* fileEnd = fileBuffer.data() + fileBufferBytes;
	char* p = fileBuffer.data();

	std::unordered_map<LoopEntry::Type, std::vector<char*>> entries2rows;

	int n_lines = count(p, fileEnd, '\n');
	
	// reserve memory for all pointer tables for all
	for (const auto& elem : COLUMN_DEFS) {
		entries2rows[elem.first].reserve(n_lines);
	}

	while (p < fileEnd) {

		string sectionName(p, std::find(p, p + SECTION_NAME_LEN, ' '));

		bool need_ommit = minimal_mode && !MINIMAL_SECTIONS.count(sectionName);
		LoopEntry::Type type = LoopEntry::str2type(sectionName);

		if (type != LoopEntry::Type::Standard) {

			for (auto& rows : entries2rows) {
				rows.second.clear();
			}

			if (!minimal_mode || MINIMAL_SECTIONS.count(sectionName)) {
				entries2rows[type].push_back(p);
			}
			
			int lineLength = (int)(find(p, fileEnd, '\n') - p);
			LoopEntry::Type localType = type;

			do {
				char* q = find(p, fileEnd, '\n');

				// incorrect line length
				if (q - p != lineLength) {
					throw std::runtime_error("Invalid PDB file");
				}
				p = q;

				++p; // go to next line
				string localName(p, find(p, p + SECTION_NAME_LEN, ' '));
				localType = LoopEntry::str2type(localName);

				bool ignore = minimal_mode && !MINIMAL_SECTIONS.count(localName);
				if (localType != LoopEntry::Type::Standard && !ignore) {
					entries2rows[localType].push_back(p);
				}

			} while (localType != LoopEntry::Type::Standard);

			
			// force section ordering
			LoopEntry::Type types[]{
				LoopEntry::Type::Atom, LoopEntry::Type::Hetatm, LoopEntry::Type::Sigatm,
				LoopEntry::Type::Anisou, LoopEntry::Type::Siguij };

			for (auto type : types) {
				auto& rows = entries2rows[type];

				// omit empty sections
				if (rows.empty() ) {
					continue;
				}

				LoopEntry* entry = new LoopEntry(LoopEntry::type2str(type), type);

				const auto& col_defs = COLUMN_DEFS.at(type);
				for (const auto& def : col_defs) {

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

		//	} while (std::equal(p, p + SECTION_NAME_LEN, section));
			} while (p != fileEnd && std::equal(p, p + SECTION_NAME_LEN, section));


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

