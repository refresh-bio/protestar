#include <iostream>
#include <vector>
#include <string>

#include "../lib-cxx/protestar-api.h"

using namespace std;

CPSAFile psafile;
vector<string> samples;
vector<char> sample_data;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cerr << "Usage:\n";
		cerr << "example-api <psa_file>\n";
		return 0;
	}

	bool opened = psafile.Open(argv[1]);

	cout << (opened ? "File opened\n" : "Archive does not exists or is corrupted\n");

	if (!opened)
		return 0;

	cout << "Archive contents:\n";
	cout << "  No. CIFs:  " << psafile.GetNoFilesCIF() << endl;
	cout << "  No. PDBs:  " << psafile.GetNoFilesPDB() << endl;
	cout << "  No. PAEs:  " << psafile.GetNoFilesPAE() << endl;
	cout << "  No. CONFs: " << psafile.GetNoFilesCONF() << endl;

	cout << "\nFile lists:" << endl;

	cout << "CIFs:" << endl;
	psafile.ListFilesCIF(samples);
	for (const auto& fn : samples)
		cout << "  " << fn << endl;

	if (!samples.empty())
	{
		psafile.GetFileCIF(samples.front(), sample_data);
		cout << string(sample_data.begin(), sample_data.end()) << endl;
	}

	cout << "PDBs:" << endl;
	psafile.ListFilesPDB(samples);
	for (const auto& fn : samples)
		cout << "  " << fn << endl;

	if (!samples.empty())
	{
		cerr << "Decompressing: " << samples.front() << endl;
		psafile.GetFilePDB(samples.front(), sample_data);
		cerr << "Raw size: " << sample_data.size() << endl;
		cout << string(sample_data.begin(), sample_data.end()) << endl;
	}

	cout << "PAEs:" << endl;
	psafile.ListFilesPAE(samples);
	for (const auto& fn : samples)
		cout << "  " << fn << endl;

	if (!samples.empty())
	{
		psafile.GetFilePAE(samples.front(), sample_data);
		cout << string(sample_data.begin(), sample_data.end()) << endl;
	}

	cout << "CONFs:" << endl;
	psafile.ListFilesCONF(samples);
	for (const auto& fn : samples)
		cout << "  " << fn << endl;

	if (!samples.empty())
	{
		psafile.GetFileCONF(samples.front(), sample_data);
		cout << string(sample_data.begin(), sample_data.end()) << endl;
	}

	psafile.Close();

	return 0;
}
