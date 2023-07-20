#include "cif.h"
#include "conversion.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;


/*****************************************************************************/
void Cif::parse() {

	char* fileEnd = fileBuffer.data() + fileBufferBytes;
	char* p = fileBuffer.data();

	std::vector<char*> table;
	table.reserve(count(p, fileEnd, '\n'));

	bool is_first = true;
	bool prev_was_added = false;

	while (p < fileEnd) {

		string sectionName(p, std::find_if(p, fileEnd, [](char c) { return c == '.' || c == '\r' || c == '\n'; }));

		if (sectionName == "loop_") {

			// go to end of line
			p = find(p, fileEnd, '\n');
			++p;

			// get loop name
			string loopName(p, std::find_if(p, fileEnd, [](char c) { return c == '.'; }));
			std::vector<string> colnames;

			auto is_space = [](char c) { return c == 0 || c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r'; };

			// iterate over column names
			do {
				// omit loop name
				p = find(p, fileEnd, '.');
				++p;

				// get column name
				char* colName = p;
				p = find_if(p, fileEnd, is_space);
				colnames.emplace_back(colName, p);

				// go to next line
				p = find(p, fileEnd, '\n');
				++p;


			} while (std::equal(p, p + loopName.size(), loopName.c_str()));

			// fill auxiliary pointer table
			bool isAtomLoop = (loopName == ENTRY_ATOM_SITE);
		
			// the loop below will be executed:
			// - once for standard tables,
			//  multiple times for atom table
			bool carryOn = true;
			while (carryOn) {
				table.clear();

				// get first token
				string firstToken(p, std::find_if(p, fileEnd, is_space));
				
				int numRows = 0;
				do {
					for (int ic = 0; ic < (int)colnames.size(); ++ic) {
						// extract token
						char* token = p;

						// if string token - find closing quote
						if (*p == '\'') {
							++p;
							p = find(p, fileEnd, '\'');
							++p;
						}
						else if (*p == ';') {
							++p;
							p = find(p, fileEnd, ';');
							++p;
						}
						else if (*p == '\"') {
							++p;
							p = find(p, fileEnd, '\"');
							++p;
						}

						else {
							p = find_if(p, fileEnd, is_space);
						}
						*p = 0;
						++p;
						table.push_back(token);

						// omit white spaces (moves to next line at the very end)
						p = find_if_not(p, fileEnd, is_space);
					}

					++numRows;

					if (*p == '#') {
						carryOn = false;
						break;
					} else if (isAtomLoop && !std::equal(firstToken.c_str(), firstToken.c_str() + firstToken.length(), p)) {
						break;
					}

				} while (true);

				LoopEntry* entry = new LoopEntry(loopName, LoopEntry::str2type(firstToken));
				int n_cols = (int)colnames.size();
				for (int ic = 0; ic < n_cols; ++ic) {
					// try to read as numeric column
					AbstractColumn* col = NumericColumn::create(colnames[ic], table, 0, ic, n_cols);

					if (col == nullptr) {
						col = new StringColumn(colnames[ic], table, 0, ic, n_cols, dataBufferPos);
					}

					entry->addColumn(col);
				}

				if (!minimal_mode || MINIMAL_SECTIONS.count(entry->name))
				{
					addEntry(entry);
					prev_was_added = true;
				}
				else
				{
					delete entry;
					prev_was_added = false;
				}
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
			} while (std::equal(p, p + sectionName.size(), sectionName.c_str()));

			copy(section, p, dataBufferPos);
			size_t size = p - section;
			BlockEntry* entry = new BlockEntry(
				sectionName,
				size,
				dataBufferPos);

			if (!minimal_mode || MINIMAL_SECTIONS.count(sectionName) || is_first || (prev_was_added && sectionName == "#"s))
			{
				addEntry(entry);
				if(sectionName != "#"s)
					prev_was_added = true;
				else
					prev_was_added = false;
			}
			else
			{
				delete entry;
				prev_was_added = false;
			}
		}

		is_first = false;
	}

	return;
}

/*****************************************************************************/
size_t Cif::store()
{
	char* p = fileBuffer.data();
	LoopEntry::Type prevType = LoopEntry::Type::Standard;

	for (const Entry* e: entries) {
		if (!e->isLoop) {
			const BlockEntry* be = dynamic_cast<const BlockEntry*>(e);
//			realloc_filebuf_if_necessary(p, be->size);
			copy_n(be->data, be->size, p);
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
				for (int ic = 0; ic < (int) le->getColumns().size(); ++ic) {
//					realloc_filebuf_if_necessary(p);
					le->getColumns()[ic]->output(ir, p);
					*p++ = ' ';
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