#include <iostream>
#include <iterator>
#include <chrono>
#include <fstream>

#include <filesystem>
#include <algorithm>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>

#include "../parsers/cif.h"
#include "../common/atom_extractor.h"
#include "model_learn.h"

using namespace std;

string in_dir;
string out_name;

ModelLearn model_learn;

int no_threads = 4;
int no_threads2 = 8;

vector<thread> threads;

// ************************************************************************************
void usage()
{
    cerr << "psarch-model-learn <in-dir-with-cif-files> <output-file-name> [no_threads] [no_final_threads]\n";
}

// ************************************************************************************
bool parse_args(int argc, char** argv)
{
    if (argc < 3)
    {
        usage();
        return false;
    }

    in_dir = argv[1];
    out_name = argv[2];

    if (argc == 4)
    {
        no_threads = atoi(argv[3]);
        no_threads2 = no_threads;
    }
    else if(argc > 4)
    {
        no_threads = atoi(argv[3]);
        no_threads2 = atoi(argv[4]);
    }

    return true;
}
 
// ************************************************************************************
void gather_distances(const string& file_name)
{
    Cif cif(true);
    Protein protein;

//    cerr << file_name << "                        \r";

    cif.load(file_name);
    cif.parse();

    auto entry = cif.findEntry(cif.getAtomEntryName());

    if (!entry)
        return;

    protein.parse_chains(dynamic_cast<const LoopEntry*>(entry));

    protein.expand_decimals(cart_working_precision);

    model_learn.parse_for_ref_atoms(protein);
}

// ************************************************************************************
void gather_tetrahedrons(const string& file_name)
{
    Cif cif(true);
    Protein protein;

//    cerr << file_name << "                        \r";

    cif.load(file_name);
    cif.parse();

//    protein.parse_chains(&cif);

    auto entry = cif.findEntry(cif.getAtomEntryName());

    if (!entry)
        return;

    protein.parse_chains(dynamic_cast<const LoopEntry*>(entry));

    protein.expand_decimals(cart_working_precision);

    model_learn.parse_for_tetrahedrons(protein);
}

// ************************************************************************************
int main(int argc, char **argv) 
{
    if (!parse_args(argc, argv))
        return 0;

    model_learn.set_no_threads(no_threads, no_threads2);

    vector<string> files;

    file_type_checker ftc;

    filesystem::path fp(in_dir);
    filesystem::recursive_directory_iterator fsdi(fp);

    for (const auto& fs : fsdi)
        if (ftc.matches(fs.path().string(), file_type_t::CIF))
            files.emplace_back(fs.path().string());

    if (files.size() < 100)
    {
        cerr << "Too few training files. Please use at least 100 CIFs." << endl;
        return 0;
    }

    shuffle(files.begin(), files.end(), mt19937_64());

//    files.resize(10000);

    cerr << "Collecting data for ref. atoms selection\n";

    atomic<uint64_t> a_fn_id = 0;

    for (int i = 0; i < no_threads; ++i)
        threads.emplace_back([&]() {
            while (true)
            {
                uint64_t id = atomic_fetch_add(&a_fn_id, 1);
                if (id >= files.size())
                    break;
                cerr << to_string(id) + " " + files[id] + "                                   \r";
                gather_distances(files[id]);
            }
        });

    for (auto& t : threads)
        t.join();
    threads.clear();

/*    for (const auto& fn : files)
    {
        cerr << cnt++ << " ";
        gather_distances(fn);
    }*/

    cerr << endl << "Determination of ref. atoms\n";
    model_learn.choose_ref_atoms();

    cerr << "Collecting data for tetrahedrons\n";

    a_fn_id = 0;

    for (int i = 0; i < no_threads; ++i)
        threads.emplace_back([&]() {
        while (true)
        {
            uint64_t id = atomic_fetch_add(&a_fn_id, 1);
            if (id >= files.size())
                break;
            cerr << to_string(id) + " " + files[id] + "                                   \r";
            gather_tetrahedrons(files[id]);
        }
            });

    for (auto& t : threads)
        t.join();
    threads.clear();

/*    for (const auto& fn : files)
    {
        cerr << cnt++ << " ";
        gather_tetrahedrons(fn);
    }*/

    cerr << endl << "Determination of centroids\n";
    model_learn.calculate_centroids();
    cerr << endl;

    cerr << endl << "Saving model\n";
    model_learn.serialize_model(out_name);

    return 0;
}

// EOF
