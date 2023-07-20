#ifndef PSA_API_H
#define PSA_API_H

#ifdef __cplusplus
// *******************************************************************************************
// C++ API (available only when using C++ compiler)
// *******************************************************************************************
#include <vector>
#include <string>
#include <memory>

// *******************************************************************************************
class CPSAFile
{
	std::unique_ptr<class CPSADecompressorLibrary> psa;
	bool is_opened;

public:
	CPSAFile();
	~CPSAFile();

	/**
	* Open archive
	* 
	* @param file_name		file name
	* @param prefetching	true to preload whole file into memory (faster if you plan series of sequence queries), false otherwise
	*
	* @return false for error
	*/
	bool Open(const std::string& file_name, bool prefetching = true);

	/**
	* Close archive
	* 
	* @return			true for success and false for error
	*/
	bool Close();

	/**
	* Return no. of files of given type
	* 
	* @return			no. of files of given type
	*/
	size_t GetNoFilesCIF() const;
	size_t GetNoFilesPDB() const;
	size_t GetNoFilesPAE() const;
	size_t GetNoFilesCONF() const;

	/**
	* List file names of given type
	* 
	* @param sample		list of file names of given type (return value)
	* 
	* @return			true for success and false for error
	*/
	bool ListFilesCIF(std::vector<std::string>& samples);
	bool ListFilesPDB(std::vector<std::string>& samples);
	bool ListFilesPAE(std::vector<std::string>& samples);
	bool ListFilesCONF(std::vector<std::string>& samples);

	/**
	* Retrive contents of asked file and type
	* 
	* @file_name		file name to decompress
	* @param data		contents of the asked file (return value)
	*
	* @return			true for success and false for error
	*/
	bool GetFileCIF(const std::string& file_name, std::vector<char> &data);
	bool GetFilePDB(const std::string& file_name, std::vector<char>& data);
	bool GetFilePAE(const std::string& file_name, std::vector<char>& data);
	bool GetFileCONF(const std::string& file_name, std::vector<char>& data);
};

#endif
#endif

// EOF
