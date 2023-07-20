#include "input.h"

#include <fstream>
#include <stdexcept>

#include "../core/io.h"

/*****************************************************************************/
size_t StructFile::load(const std::string& fileName) {

	// clear entries if any
	clearEntries();

	if (load_file(fileName, fileBuffer))
	{
		fileBufferBytes = fileBuffer.size();
		fileBuffer.resize(fileBufferBytes + 1); // for trailing 0
		dataBuffer.resize(fileBufferBytes + 1);
		
		// reset buffer pointer
		dataBufferPos = dataBuffer.data();
		fileBuffer[fileBufferBytes] = 0;
	}
	else {
		throw std::runtime_error("File not found: " + fileName);
	}

	return fileBufferBytes;
}

/*****************************************************************************/
size_t StructFile::load(const std::vector<uint8_t>& file_data)
{
	clearEntries();

	fileBuffer.reserve(file_data.size() + 1);
	fileBuffer.assign(file_data.begin(), file_data.end());

	fileBufferBytes = fileBuffer.size();
	fileBuffer.resize(fileBufferBytes + 1); // for trailing 0
	dataBuffer.resize(fileBufferBytes + 1);

	// reset buffer pointer
	dataBufferPos = dataBuffer.data();
	fileBuffer[fileBufferBytes] = 0;

	return fileBufferBytes;
}
