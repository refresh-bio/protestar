#include "cif-input.h"
#include "conversion.h"

#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

using namespace std;

/*****************************************************************************/
void CifInput::parse() {

	char* fileEnd = fileBuffer.data() + fileBufferBytes;
	char* p = fileBuffer.data();

	std::vector<char*> table;
	table.reserve(count(p, fileEnd, '\n'));

	bool is_first = true;
	bool prev_was_added = false;

	// first pass - find multiline fields
	while ((p = std::find(p, fileEnd, ';')) != fileEnd) {
		if (*(p - 1) == '\n' || *(p - 1) == '\r') {
			*p = StringColumn::MULTILINE_VALUE_MARKER;

			//	std::cout << "B:" << string(p, p + 20) << endl;

				// find closing semicolon
			do {
				++p;
				p = std::find(p, fileEnd, ';');
				if (p == fileEnd) {
					throw std::runtime_error("Incorrectly formated CIF file (no matching semicolon for multiline text field)");
				}

			} while (*(p - 1) != '\n' && *(p - 1) != '\r');

			*p = StringColumn::MULTILINE_VALUE_MARKER;

			//	std::cout << "E:" << string(p - 20, p + 1) << endl;	
		}
		++p;
	}

	p = fileBuffer.data();
	// second pass
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

			// get column offsets
			struct Alignment {
				int leftOffset = -1;
				int rightOffset = -1;
				bool leftAligned = true;
				bool rightAligned = true;
			};

			std::vector<Alignment> alignments(colnames.size());

			// the loop below will be executed:
			// - once for standard tables,
			//  multiple times for atom table
			bool carryOn = true;
			while (carryOn) {
				table.clear();

				// get first token
				string firstToken(p, std::find_if(p, fileEnd, is_space));

				std::vector<int> widths;

				int numRows = 0;
				do {
					char* rowBegin = p;

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

							// disable alignment
							alignments[ic].leftAligned = false;
							alignments[ic].rightAligned = false;
						}
						else if (*p == StringColumn::MULTILINE_VALUE_MARKER) {

							++p;
							p = find(p, fileEnd, StringColumn::MULTILINE_VALUE_MARKER);
							++p;

							// disable alignment
							alignments[ic].leftAligned = false;
							alignments[ic].rightAligned = false;
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


						if (numRows == 0) {
							// first row - initialize offsets
							alignments[ic].leftOffset = token - rowBegin;
							alignments[ic].rightOffset = p - rowBegin;
						}
						else {
							// verify if offsets agree
							if (token - rowBegin != alignments[ic].leftOffset) {
								alignments[ic].leftAligned = false;
							}

							if (p - rowBegin != alignments[ic].rightOffset) {
								alignments[ic].rightAligned = false;
							}
						}

						// omit white spaces (moves to next line at the very end)
						p = find_if_not(p, fileEnd, is_space);
					}

					++numRows;

					if (*p == '#') {
						carryOn = false;
						break;
					}
					else if (isAtomLoop && !std::equal(firstToken.c_str(), firstToken.c_str() + firstToken.length(), p)) {
						break;
					}

				} while (true);

				LoopEntry* entry = new LoopEntry(loopName, LoopEntry::str2type(firstToken));
				int n_cols = (int)colnames.size();
				for (int ic = 0; ic < n_cols; ++ic) {
					// try to read as numeric column
					int width = 0;
					if (ic < n_cols - 1) {

						// left aligment has top priority
						if (alignments[ic].leftAligned) {
							width = table[ic] - table[ic + 1];
						}
						else {
							width = alignments[ic].rightAligned ? table[ic + 1] - table[ic] : 0;
						}
					}
					else {
						// last column - find newline character
						char* p = table[ic];
						while (*p != '\n' && *p != '\r') {
							++p;
						}

						if (alignments[ic].leftAligned) {
							width = table[ic] - p;
						}
						else {
							width = alignments[ic].rightAligned ? p - table[ic] : 0;
						}
					}

					AbstractColumn* col = NumericColumn::create(colnames[ic], table, width, ic, n_cols);

					if (col == nullptr) {
						col = new StringColumn(colnames[ic], table, width, ic, n_cols, dataBufferPos);
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
//			} while (std::equal(p, p + sectionName.size(), sectionName.c_str()));
//			} while (p != fileEnd && std::equal(p, p + sectionName.size(), sectionName.c_str()));
			} while (fileEnd - p >= sectionName.size() && std::equal(p, p + sectionName.size(), sectionName.c_str()));

			copy(section, p, dataBufferPos);
			size_t size = p - section;
			BlockEntry* entry = new BlockEntry(
				sectionName,
				size,
				dataBufferPos);

			if (!minimal_mode || MINIMAL_SECTIONS.count(sectionName) || is_first || (prev_was_added && sectionName == "#"s))
			{
				addEntry(entry);
				if (sectionName != "#"s)
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

