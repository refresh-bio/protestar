#include <iostream>
#include <chrono>
#include <fstream>

#include "../parsers/cif.h"
#include "../parsers/pdb.h"

using namespace std;

void test_cif(const std::vector<string>& files) {

    Cif cif(false);
    Cif outCif(false);
    size_t total_read = 0;
  
    double t_load = 0, t_parse = 0, t_store = 0, t_save = 0;

    for (const auto& file : files) {
        
        auto t = std::chrono::high_resolution_clock::now();
        size_t bytesRead = cif.load("../../data/" + file);
        t_load += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        t = std::chrono::high_resolution_clock::now();
        cif.parse();
        t_parse += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        outCif.reset(cif.getDataBytes(), bytesRead);
        char* raw = outCif.getDataBuffer();

        // make a raw buffer copy instead of decompression
        memcpy(raw, cif.getDataBuffer(), cif.getDataBytes());
        
        for (const Entry* e : cif.getEntries()) {
            if (e->isLoop) {
                const LoopEntry* le = dynamic_cast<const LoopEntry*>(e);
                
                // make a new entry
                LoopEntry* newEntry = new LoopEntry(le->name, le->getType());
                for (const AbstractColumn* c : le->getColumns()) {
                    if (c->isNumeric) {
                        const NumericColumn* nc = dynamic_cast<const NumericColumn*>(c);
                        auto vals = nc->getValues();
                        NumericColumn* newColumn = new NumericColumn(nc->name, nc->numDecimals, nc->width, std::move(vals));
                        newEntry->addColumn(newColumn); 
                    }
                    else {
                        const StringColumn* sc = dynamic_cast<const StringColumn*>(c);
                        char*& dst = raw;

                        StringColumn* newColumn = new StringColumn(sc->name, sc->numRows, sc->width, dst);
                        newEntry->addColumn(newColumn);
                    }
                }
                outCif.addEntry(newEntry);
            }
            else {
              
                const BlockEntry* be = dynamic_cast<const BlockEntry*>(e);

                // make a new entry
                BlockEntry* newEntry = new BlockEntry(be->name, be->size, raw);
                outCif.addEntry(newEntry);
            }
        }
      
        t = std::chrono::high_resolution_clock::now();
        outCif.store();
        t_store += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        t = std::chrono::high_resolution_clock::now();
        outCif.save("../../data/" + file + ".new");
        t_save += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        total_read += bytesRead;
    }

    /*
    cout << cif.getColumn(Cif::ENTRY_ATOM_SITE, Cif::COLUMN_CARTN_X) << endl
        << cif.getColumn(Cif::ENTRY_ATOM_SITE, "bla") << endl
        << cif.getColumn("X", Cif::COLUMN_CARTN_X) << endl;
    */

    double mb = (double)total_read / 1000000;
    cout << "Load throughput [MB/s]:" << mb / t_load << endl;
    cout << "Parse throughput [MB/s]:" << mb / t_parse << endl;
    cout << "Store throughput [MB/s]:" << mb / t_store << endl;
    cout << "Save throughput [MB/s]:" << mb / t_save << endl << endl;
    //cout << total_read << endl << endl;
}



void test_pdb(const std::vector<string>& files) {

    Pdb pdb(false);
    Pdb outPdb(false);
    size_t total_read = 0;

    double t_load = 0, t_parse = 0, t_store = 0, t_save = 0;

    for (const auto& file : files) {

        auto t = std::chrono::high_resolution_clock::now();
        size_t bytesRead = pdb.load("../../data/" + file);
        t_load += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        t = std::chrono::high_resolution_clock::now();
        pdb.parse();
        t_parse += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        outPdb.reset(pdb.getDataBytes(), bytesRead);
        char* raw = outPdb.getDataBuffer();

        // make a raw buffer copy instead of decompression
        memcpy(raw, pdb.getDataBuffer(), pdb.getDataBytes());
        
        for (const Entry* e : pdb.getEntries()) {
            if (e->isLoop) {
                const LoopEntry* le = dynamic_cast<const LoopEntry*>(e);

                // make a new entry
                LoopEntry* newEntry = new LoopEntry(le->name, le->getType());
                for (const AbstractColumn* c : le->getColumns()) {
                    if (c->isNumeric) {
                        const NumericColumn* nc = dynamic_cast<const NumericColumn*>(c);
                        auto vals = nc->getValues();
                        NumericColumn* newColumn = new NumericColumn(nc->name, nc->numDecimals, nc->width, std::move(vals));
                        newEntry->addColumn(newColumn);
                    }
                    else {
                        const StringColumn* sc = dynamic_cast<const StringColumn*>(c);
                        char*& dst = raw;

                        StringColumn* newColumn = new StringColumn(sc->name, sc->numRows, sc->width, dst);
                        newEntry->addColumn(newColumn);
                    }
                }
                outPdb.addEntry(newEntry);
            }
            else {
                const BlockEntry* be = dynamic_cast<const BlockEntry*>(e);
            
                // make a new entry
                BlockEntry* newEntry = new BlockEntry(be->name, be->size, raw);
                outPdb.addEntry(newEntry);
            }
        }
     
        t = std::chrono::high_resolution_clock::now();
        outPdb.store();
        t_store += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        t = std::chrono::high_resolution_clock::now();
        outPdb.save("../../data/" + file + ".new");
        t_save += std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - t).count();

        total_read += bytesRead;
    }

    /*
    cout << cif.getColumn(Cif::ENTRY_ATOM_SITE, Cif::COLUMN_CARTN_X) << endl
        << cif.getColumn(Cif::ENTRY_ATOM_SITE, "bla") << endl
        << cif.getColumn("X", Cif::COLUMN_CARTN_X) << endl;
    */

    double mb = (double)total_read / 1000000;
    cout << "Load throughput [MB/s]:" << mb / t_load << endl;
    cout << "Parse throughput [MB/s]:" << mb / t_parse << endl;
    cout << "Store throughput [MB/s]:" << mb / t_store << endl;
    cout << "Save throughput [MB/s]:" << mb / t_save << endl << endl;
    //cout << total_read << endl << endl;
}


int main() {

    std::vector<string> cif_files{
        "cif/7xqp.cif",
        "cif/7cyl.cif",
        "cif/7cyl-sf.cif",
        "cif/AF-A0A1B0GUL3-F1-model_v3.cif",
        "cif/AF-A0A1B0GUN5-F1-model_v3.cif",
        "cif/AF-A0A1B0GUP4-F1-model_v3.cif",
        "cif/AF-A0A1B0GV02-F1-model_v3.cif",
        "cif/AF-A0A1B0GV23-F1-model_v3.cif",
        "cif/AF-A0A1B0GV68-F1-model_v3.cif",
        "cif/AF-A0A1B0GVY1-F1-model_v3.cif",
        "cif/AF-A0A023HHL0-F1-model_v3.cif",
        "cif/AF-A0A023I7H2-F1-model_v3.cif" 
    };

    std::vector<string> pdb_files{
       //  "pdb/MGYP000127130086.pdb",
        "pdb/MGYP004603780001.pdb",
         "pdb/MGYP002262106581.pdb",
         "pdb/7xqp.pdb",
         "pdb/AF-A0A024R1R8-F1-model_v4.pdb",
         "pdb/8g8e.pdb",
         "pdb/MGYP000911143359.pdb",
      
    };

    try {
        cout << "CIF" << endl;
        test_cif(cif_files);
        cout << endl << "PDB" << endl;
        test_pdb(pdb_files);
        
    }
    catch (std::runtime_error& err) {
        cout << "[ERROR] " << err.what();
    }

    return 0;
}