#include <iostream>
#include <fstream>
#include <iterator>

#include "../core/version.h"
#include "../core/params.h"
#include "../core/utils.h"

#include <mimalloc-override.h>

#include "../core/psa_compressor.h"
#include "../core/psa_decompressor.h"

#include "../common/tetrahedron.h"

using namespace std;

void usage();
void usage_add();
void usage_get();
void usage_list();
void usage_info();
bool parse_params(int argc, char **argv);
bool parse_params_add(int argc, char **argv);
bool parse_params_get(int argc, char **argv);
bool parse_params_list(int argc, char **argv);
bool parse_params_info(int argc, char **argv);

bool do_add();
bool do_get();
bool do_list();
bool do_info();

CParams params;

// *****************************************************************
void usage()
{
    cerr <<
        APP_NAME << " " << APP_VERSION_STR << endl <<
        "Usage: protestar <command> <options>" << endl <<
        "Commands:" << endl <<
        "   add - add files to archive" << endl <<
        "   get - get files from archive" << endl <<
        "   list - list archive contents" << endl <<
        "   info - show some statistics of the compressed data" << endl;       
}

// *****************************************************************
void usage_add()
{
    cerr <<
        APP_NAME << " " << APP_VERSION_STR << endl <<
        "Usage: protestar add <options>" << endl <<
        "Options:" << endl <<
        "   --type <string> - file type: cif, pdb, pae, conf, ALL" << endl <<
        "   --in <string> - name of input archive" << endl <<
        "   --out <string> - name of output archive" << endl <<
        "   --infile <string> - file name to add" << endl <<
        "   --indir <string> - directory with files to add" << endl <<
        "   --indir-recursive <string> - directory (recursive) with files to add" << endl <<
        "   --inlist <string> - name of file with paths to files to add" << endl <<
        "   --intar <string> - name of tar file with files to add" << endl <<
        "   -t|--threads <int> - no. of threads" << endl <<
        "   --fast - slightly faster compression but slightly worse ratio (only for CIF|PDB files)" << endl <<
        "   --minimal - minimal mode (only most important fields from CIF|PDB files are stored)" << endl <<
        "   --lossy - turn-on lossy compression (only for CIF|PDB|PAE files)" << endl <<
        "   --max-error-bb - max error (in mA [0, 500]) of backbone atom coordinates (only for CIF|PDB files)" << endl <<
        "   --max-error-sc - max error (in mA [0, 500]) of side-chain atom coordinates (only for CIF|PDB files)" << endl <<
        "   --pae-lossy-level - lossy level from range [0, 4] (only for PAE files)" << endl <<
        "   --single-bf - enable single B-factor value (only for CIF|PDB files)" << endl <<
        "   -v|--verbose <int> - verbosity level" << endl;
}

// *****************************************************************
void usage_get()
{
    cerr <<
        APP_NAME << " " << APP_VERSION_STR << endl <<
        "Usage: protestar get <options>" << endl <<
        "   --type <string> - file type: cif, pdb, pae, conf, ALL" << endl <<
        "   --in <string> - name of input archive" << endl <<
        "   --outdir <string> - output directory" << endl <<
        "   --file <string> - file name to get" << endl <<
        "   --list <string> - name of file with file names to get" << endl <<
        "   --all - get all files" << endl <<
        "   -t|--threads <int> - no. of threads" << endl <<
        "   -v|--verbose <int> - verbosity level" << endl;
}

// *****************************************************************
void usage_list()
{
    cerr <<
        APP_NAME << " " << APP_VERSION_STR << endl <<
        "Usage: protestar list <options>" << endl <<
        "   --in <string> - name of input archive" << endl <<
        "   --type <string> - file type: cif, pdb, pae, conf, ALL" << endl <<
        "   --show-file-info - show some information about file types" << endl;
}

// *****************************************************************
void usage_info()
{
    cerr <<
        APP_NAME << " " << APP_VERSION_STR << endl <<
        "Usage: protestar info <options>" << endl <<
        "   --in <string> - name of input archive" << endl;
}

// *****************************************************************
bool parse_params(int argc, char** argv)
{
    if (argc < 2)
    {
        usage();
        return false;
    }

    string mode = argv[1];

    for (int i = 1; i < argc; ++i)
        params.cmd += string(argv[i]) + " ";
    params.cmd.pop_back();

    if (mode == "add")
    {
        if (parse_params_add(argc, argv))
            return true;

        usage_add();
        return false;
    }
    else if (mode == "get")
    {
        if (parse_params_get(argc, argv))
            return true;

        usage_get();
        return false;
    }
    else if(mode == "list")
    {
        if (parse_params_list(argc, argv))
            return true;

        usage_list();
        return false;
    }
    else if (mode == "info")
    {
        if (parse_params_info(argc, argv))
            return true;

        usage_info();
        return false;
    }

    usage();

    return false;
}

// *****************************************************************
bool parse_params_add(int argc, char** argv)
{
    params.working_mode = working_mode_t::add;

    for (int i = 2; i < argc; ++i)
    {
        if (argv[i] == "--type"s && i + 1 < argc)
        {
            if (argv[i + 1] == "cif"s)
                params.file_type = file_type_t::CIF;
            else if(argv[i + 1] == "pdb"s)
                params.file_type = file_type_t::PDB;
            else if(argv[i + 1] == "pae"s)
                params.file_type = file_type_t::PAE;
            else if (argv[i + 1] == "conf"s)
                params.file_type = file_type_t::CONF;
            else if (argv[i + 1] == "ALL"s)
                params.file_type = file_type_t::ALL;
            else
            {
                cerr << "Wrong file type: " << argv[i + 1] << endl;
                return false;
            }
            ++i;
        }
        else if (argv[i] == "--in"s && i + 1 < argc)
        {
            params.input_archive = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--out"s && i + 1 < argc)
        {
            params.output_archive = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--infile"s && i + 1 < argc)
        {
            params.infile = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--intar"s && i + 1 < argc)
        {
            params.intar = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--indir"s && i + 1 < argc)
        {
            params.indir = argv[i + 1];
            params.indir_recursive.clear();
            ++i;
        }
        else if (argv[i] == "--indir-recursive"s && i + 1 < argc)
        {
            params.indir_recursive = argv[i + 1];
            params.indir.clear();
            ++i;
        }
        else if (argv[i] == "--inlist"s && i + 1 < argc)
        {
            params.inlist = argv[i + 1];
            ++i;
        }
        else if ((argv[i] == "-t"s || argv[i] == "--threads"s) && i + 1 < argc)
        {
            params.no_threads = stoi(argv[i + 1]);
            ++i;
            if (params.no_threads == 0)
                params.no_threads = thread::hardware_concurrency();
            if (params.no_threads == 0)
                params.no_threads = 1;
        }
        else if (argv[i] == "--fast"s)
        {
            params.max_compression = false;
        }
        else if (argv[i] == "--minimal"s)
        {
            params.minimal = true;
        }
        else if (argv[i] == "--lossy"s)
        {
            params.lossy = true;
        }
        else if (argv[i] == "--max-error-bb"s && i + 1 < argc)
        {
            params.max_error_bb = stoi(argv[i + 1]);
            if (params.max_error_bb < 0)
            {
                cerr << "Too low max-error-bb value: " << params.max_error_bb << " changed to 0\n";
                params.max_error_bb = 0;
            }
            else if (params.max_error_bb > 500)
            {
                cerr << "Too high max-error-bb value: " << params.max_error_bb << " changed to 500\n";
                params.max_error_bb = 500;
            }
            ++i;
        }
        else if (argv[i] == "--max-error-sc"s && i + 1 < argc)
        {
            params.max_error_sc = stoi(argv[i + 1]);
            if (params.max_error_sc < 0)
            {
                cerr << "Too low max-error-sc value: " << params.max_error_sc << " changed to 0\n";
                params.max_error_sc = 0;
            }
            else if (params.max_error_sc > 500)
            {
                cerr << "Too high max-error-sc value: " << params.max_error_sc << " changed to 500\n";
                params.max_error_sc = 500;
            }
            ++i;
        }
        else if (argv[i] == "--pae-lossy-level"s && i + 1 < argc)
        {
            params.pae_lossy_level = stoi(argv[i + 1]);
            if (params.pae_lossy_level < 0)
            {
                cerr << "Too low pae-lossy-level value: " << params.pae_lossy_level << " changed to 0\n";
                params.pae_lossy_level = 0;
            }
            else if (params.pae_lossy_level > 4)
            {
                cerr << "Too high pae-lossy-level value: " << params.pae_lossy_level << " changed to 4\n";
                params.pae_lossy_level = 4;
            }
            ++i;
        }
        else if (argv[i] == "--single-bf"s)
        {
            params.single_bf = true;
        }
        else if ((argv[i] == "--verbose"s || argv[i] == "-v"s) && i + 1 < argc)
        {
            params.verbose = stoi(argv[i + 1]);
            ++i;
        }
        else
        {
            cerr << "Unknown parameter: " << argv[i] << endl;
            return false;
        }
    }

    if (!params.lossy)
    {
        params.max_error_bb = 0;
        params.max_error_sc = 0;
    }

    if (params.no_threads == 0)
        params.no_threads = thread::hardware_concurrency();
    if (params.no_threads == 0)
        params.no_threads = 1;

    int no_inputs = 0;

    no_inputs += !params.infile.empty();
    no_inputs += !params.indir.empty();
    no_inputs += !params.indir_recursive.empty();
    no_inputs += !params.inlist.empty();
    no_inputs += !params.intar.empty();

    if (no_inputs == 0)
    {
        cerr << "You have to specify any input data\n";
        return false;
    }
    else if (no_inputs > 1)
    {
        cerr << "You can specify only one of: --infile, --indir, --indir-recursive, --inlist, --intar\n";
        return false;
    }

    return
        params.file_type != file_type_t::none &&
        !params.output_archive.empty();
}

// *****************************************************************
bool parse_params_get(int argc, char** argv)
{
    params.working_mode = working_mode_t::get;

    for (int i = 2; i < argc; ++i)
    {
        if (argv[i] == "--type"s && i + 1 < argc)
        {
            if (argv[i + 1] == "cif"s)
                params.file_type = file_type_t::CIF;
            else if (argv[i + 1] == "pdb"s)
                params.file_type = file_type_t::PDB;
            else if (argv[i + 1] == "pae"s)
                params.file_type = file_type_t::PAE;
            else if (argv[i + 1] == "conf"s)
                params.file_type = file_type_t::CONF;
            else if (argv[i + 1] == "ALL"s)
                params.file_type = file_type_t::ALL;
            else
            {
                cerr << "Wrong file type: " << argv[i + 1] << endl;
                return false;
            }
            ++i;
        }
        else if (argv[i] == "--in"s && i + 1 < argc)
        {
            params.input_archive = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--outdir"s && i + 1 < argc)
        {
            params.outdir = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--file"s && i + 1 < argc)
        {
            params.infile = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--list"s && i + 1 < argc)
        {
            params.inlist = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--all"s)
        {
            params.get_all = true;
        }
        else if ((argv[i] == "-t"s || argv[i] == "--threads"s) && i + 1 < argc)
        {
            params.no_threads = stoi(argv[i + 1]);
            ++i;
            if (params.no_threads == 0)
                params.no_threads = thread::hardware_concurrency();
            if (params.no_threads == 0)
                params.no_threads = 1;
        }
        else if ((argv[i] == "--verbose"s || argv[i] == "-v"s) && i + 1 < argc)
        {
            params.verbose = stoi(argv[i + 1]);
            ++i;
        }
        else
        {
            cerr << "Unknown parameter: " << argv[i] << endl;
            return false;
        }
    }

    if (params.no_threads == 0)
        params.no_threads = thread::hardware_concurrency();
    if (params.no_threads == 0)
        params.no_threads = 1;

    return
        params.file_type != file_type_t::none &&
        !params.input_archive.empty() &&
        (!params.infile.empty() || !params.inlist.empty() || params.get_all);
}

// *****************************************************************
bool parse_params_list(int argc, char** argv)
{
    params.working_mode = working_mode_t::list;

    for (int i = 2; i < argc; ++i)
    {
        if (argv[i] == "--type"s && i + 1 < argc)
        {
            if (argv[i + 1] == "cif"s)
                params.file_type = file_type_t::CIF;
            else if (argv[i + 1] == "pdb"s)
                params.file_type = file_type_t::PDB;
            else if (argv[i + 1] == "pae"s)
                params.file_type = file_type_t::PAE;
            else if (argv[i + 1] == "conf"s)
                params.file_type = file_type_t::CONF;
            else if (argv[i + 1] == "ALL"s)
                params.file_type = file_type_t::ALL;
            else
            {
                cerr << "Wrong file type: " << argv[i + 1] << endl;
                return false;
            }
            ++i;
        }
        else if (argv[i] == "--in"s && i + 1 < argc)
        {
            params.input_archive = argv[i + 1];
            ++i;
        }
        else if (argv[i] == "--show-file-info"s)
        {
            params.show_file_info = true;
            ++i;
        }
        else
        {
            cerr << "Unknown parameter: " << argv[i] << endl;
            return false;
        }
    }

    return 
        params.file_type != file_type_t::none &&
        !params.input_archive.empty();
}

// *****************************************************************
bool parse_params_info(int argc, char** argv)
{
    params.working_mode = working_mode_t::info;

    for (int i = 2; i < argc; ++i)
    {
        if (argv[i] == "--in"s && i + 1 < argc)
        {
            params.input_archive = argv[i + 1];
            ++i;
        }
        else
        {
            cerr << "Unknown parameter: " << argv[i] << endl;
            return false;
        }
    }

    return 
        !params.input_archive.empty();
}

// *****************************************************************
bool do_add()
{
    vector<string> input_file_names;

    if (!params.infile.empty())
        input_file_names.push_back(params.infile);
    else if (!params.inlist.empty())
    {
        ifstream ifs(params.inlist);
        if (!ifs)
        {
            cerr << "Cannot open file: " << params.inlist << endl;
            return false;
        }
        istream_iterator<string> ifs_end;

        input_file_names.assign(istream_iterator<string>(ifs), ifs_end);
    }
    else if (!params.indir.empty())
    {
        filesystem::path fp(params.indir);

        try
        {
            filesystem::directory_iterator fsdi(fp);

            for (const auto& fs : fsdi)
                input_file_names.push_back(fs.path().string());

        } catch(...)
        { 
            cerr << "Non-existing directory: " << params.indir << endl;
            return false;
        }
    }
    else if (!params.indir_recursive.empty())
    {
        filesystem::path fp(params.indir_recursive);

        try 
        {
            filesystem::recursive_directory_iterator fsdi(fp);

            for (const auto& fs : fsdi)
                input_file_names.push_back(fs.path().string());
        }
        catch (...)
        {
            cerr << "Non-existing or empty directory: " << params.indir_recursive << endl;
            return false;
        }
    }

    // Filter file names according to extension
    auto p_end = remove_if(input_file_names.begin(), input_file_names.end(), [&](const auto& fn) {
        return !file_type_checker::matches(fn, params.file_type);
        });

    input_file_names.erase(p_end, input_file_names.end());

    params.input_file_name_types.clear();

    for (const auto& fn : input_file_names)
    {
        file_type_t ft = file_type_checker::check(fn).first;

        params.input_file_name_types.emplace_back(fn, ft);
    }

    CPSACompressor compressor(params);

    if (!params.input_archive.empty())
    {
        if(!compressor.open_existing(params.input_archive))
        {
            cerr << "Cannot open input archive: " << params.input_archive << endl;
            return false;
        }

        if (!compressor.create(params.output_archive))
        {
            cerr << "Cannot create new archive: " << params.output_archive << endl;
            return false;
        }

        if (!compressor.copy_from_existing())
        {
            cerr << "Cannot transfer data from existing to new archive" << endl;
            return false;
        }
    }
    else
    {
        if (!compressor.create(params.output_archive))
        {
            cerr << "Cannot create new archive: " << params.output_archive << endl;
            return false;
        }
    }

    if (!compressor.add_new())
    {
        cerr << "Error during compression" << endl;
        return false;
    }

    if (!compressor.close())
    {
        cerr << "Cannot save collection description data" << endl;
        return false;
    }

    return true;
}

// *****************************************************************
bool do_get()
{
    if (!params.infile.empty())
        params.input_file_name_types.emplace_back(params.infile, params.file_type);
    else if (!params.inlist.empty())
    {
        ifstream ifs(params.inlist);
        if (!ifs)
        {
            cerr << "Cannot open file: " << params.inlist << endl;
            return false;
        }
        istream_iterator<string> ifs_end;

        vector<string> input_file_names;

        input_file_names.assign(istream_iterator<string>(ifs), ifs_end);

        for (const auto& fn : input_file_names)
            params.input_file_name_types.emplace_back(fn, params.file_type);
    }

    if (params.input_archive.empty())
    {
        cerr << "Input archive is not specified" << endl;
        return false;
    }

    CPSADecompressor decompressor(params, true);

    if (!decompressor.open_existing(params.input_archive, 1024))
    {
        cerr << "Cannot open archive: " << params.input_archive << endl;
        return false;
    }

    if (!decompressor.Deserialize())
    {
        cerr << "Archive is broken" << endl;
        return false;
    }

    uint32_t archive_ver = decompressor.GetArchiveVersion();

    if (archive_ver < MIN_SUPPORTED_ARCHIVE_VERSION || archive_ver > MAX_SUPPORTED_ARCHIVE_VERSION)
    {
        cerr << UNSUPPORTED_ARCHIVE_INFO(archive_ver);
        return false;
    }

    if (params.get_all)
        decompressor.ListFilesExt(params.file_type, params.input_file_name_types);

    if (!decompressor.get_files())
    {
        cerr << "Error during decompression" << endl;
        return false;
    }

    return true;
}

// *****************************************************************
bool do_list()
{
    if (params.input_archive.empty())
    {
        cerr << "Input archive is not specified" << endl;
        return false;
    }

    CPSADecompressor decompressor(params, true);

    vector<string> cmd_lines;

    if (!decompressor.open_existing(params.input_archive))
    {
        cerr << "Cannot open archive: " << params.input_archive << endl;
        return false;
    }

    if (!decompressor.Deserialize())
    {
        cerr << "Archive is broken" << endl;
        return false;
    }

    uint32_t archive_ver = decompressor.GetArchiveVersion();

    if (archive_ver < MIN_SUPPORTED_ARCHIVE_VERSION || archive_ver > MAX_SUPPORTED_ARCHIVE_VERSION)
    {
        cerr << UNSUPPORTED_ARCHIVE_INFO(archive_ver);
        return false;
    }

    if(params.show_file_info)
    { 
        vector<unique_ptr<file_desc_t>> file_desc;

        decompressor.GetFilesInfo(params.file_type, file_desc);

        for (const auto& desc : file_desc)
            cout << desc->str() << endl;
    }
    else
    {
        vector<string> file_list;

        decompressor.ListFiles(params.file_type, file_list);

        for (const auto& s : file_list)
            cout << s << endl;
    }

    return true;
}

// *****************************************************************
bool do_info()
{
    if (params.input_archive.empty())
    {
        cerr << "Input archive is not specified" << endl;
        return false;
    }

    CPSADecompressor decompressor(params, true);

    vector<string> cmd_lines;

    if (!decompressor.open_existing(params.input_archive))
    {
        cerr << "Cannot open archive: " << params.input_archive << endl;
        return false;
    }

    if (!decompressor.Deserialize())
    {
        cerr << "Archive is broken" << endl;
        return false;
    }

    decompressor.GetCmdLines(cmd_lines);
    
    map<file_type_t, size_t> no_files;
    decompressor.GetNoFiles(no_files);

    uint32_t archive_ver = decompressor.GetArchiveVersion();

    cout << "Archive      : " << params.input_archive << endl;
    cout << "Archive ver. : " << archive_ver / 100 << "." << archive_ver % 100 << endl;
    cout << "No CIF files : " << no_files[file_type_t::CIF] << endl;
    cout << "No PDB files : " << no_files[file_type_t::PDB] << endl;
    cout << "No PAE files : " << no_files[file_type_t::PAE] << endl;
    cout << "No CONF files: " << no_files[file_type_t::CONF] << endl;
    cout << "Command lines: " << endl;
    for (const auto& cmd : cmd_lines)
        cout << "   " << cmd << endl;

    return true;
}

// *****************************************************************
int main(int argc, char **argv) 
{
    if (!parse_params(argc, argv))
        return 0;

    if (params.working_mode == working_mode_t::add)
        do_add();
    else if (params.working_mode == working_mode_t::get)
        do_get();
    else if (params.working_mode == working_mode_t::list)
        do_list();
    else if (params.working_mode == working_mode_t::info)
        do_info();
    else
        cerr << "Unknown working mode" << endl;

    return 0;
}

// EOF