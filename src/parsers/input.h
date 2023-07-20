#pragma once
#include "entries.h"

#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>

#include <set>
#include <stdexcept>

class StructFile {
public:
 
    struct Columns {
        static constexpr const char* group_PDB          { "group_PDB" };
        static constexpr const char* id                 { "id" };
        static constexpr const char* auth_atom_id       { "auth_atom_id" };
        static constexpr const char* label_alt_id       { "label_alt_id" };
        static constexpr const char* auth_comp_id       { "auth_comp_id" };
        static constexpr const char* auth_asym_id       { "auth_asym_id" };
        static constexpr const char* auth_seq_id        { "auth_seq_id" };
        static constexpr const char* pdbx_PDB_ins_code  { "pdbx_PDB_ins_code" };

        static constexpr const char* Cartn_x            { "Cartn_x" };
        static constexpr const char* Cartn_y            { "Cartn_y" };
        static constexpr const char* Cartn_z            { "Cartn_z" };

        static constexpr const char* occupancy          { "occupancy" };
        static constexpr const char* B_iso_or_equiv     { "B_iso_or_equiv" };
        static constexpr const char* segment            { "segment" };
        static constexpr const char* type_symbol        { "type_symbol" };
        static constexpr const char* pdbx_formal_charge { "pdbx_formal_charge" };

    };

    virtual const char* getAtomEntryName() const = 0;


protected:
    bool minimal_mode;

    size_t fileBufferBytes;
   
    std::vector<char> dataBuffer;
    std::vector<char> fileBuffer;

    char* dataBufferPos;

    std::vector<Entry*> entries;

    /*
    void realloc_filebuf_if_necessary(char *&p, size_t extra_size = 128)
    {
        int aa = fileBuffer.size();

        if (p + extra_size >= fileBuffer.data() + fileBuffer.size())
        {
            throw std::runtime_error("File buffer reallocation requested - this should never happen");
        }
    }*/

public:
    StructFile(bool minimal_mode, size_t bufSize = 1 << 24) :
//    Input(size_t bufSize = 1 << 10) :
        minimal_mode{ minimal_mode },
        fileBufferBytes{ 0 },
        dataBufferPos{ dataBuffer.data() }
    {
        dataBuffer.reserve(bufSize);
        fileBuffer.reserve(bufSize);
    }
       
    virtual ~StructFile() {
        clearEntries();
    }

    void clearEntries() {
        for (auto entry : entries) {
            delete entry;
        }
        entries.clear();
    }

    const std::vector<Entry*>& getEntries() const { return entries; }

    void addEntry(Entry* entry) { entries.push_back(entry); }

    Entry* findEntry(const std::string& name) const {
        auto it = std::find_if(entries.begin(), entries.end(),
            [&name](const Entry* e)->bool { return e->name == name; });

        return (it != entries.end()) ? (*it) : nullptr;
    }

    AbstractColumn* findColumn(const std::string& entryName, const std::string& colName) const {
        Entry* e = findEntry(entryName);
        if (e != nullptr && e->isLoop) {
            return dynamic_cast<LoopEntry*>(e)->findColumn(colName);
        }

        return nullptr;
    }

    const char* getDataBuffer() const { return dataBuffer.data(); }
    char* getDataBuffer() { return dataBuffer.data(); }

    size_t getDataBytes() const { return dataBufferPos - dataBuffer.data(); }


    void reset(size_t bufSize, size_t fileBufSize) {
        clearEntries();
        
        dataBuffer.resize(bufSize + 1);
        fileBuffer.resize(fileBufSize + 1);

        // reset buffer pointers
        dataBufferPos = dataBuffer.data();
    }

    virtual size_t load(const std::string& fileName);
    virtual size_t load(const std::vector<uint8_t>& file_data);

    virtual void parse() = 0;

    virtual size_t store() = 0;

    virtual void save(const std::string& fileName) {

/*        FILE* out = fopen(fileName.c_str(), "wb");
        if (out)
            fwrite(fileBuffer.data(), 1, fileBuffer.size(), out);
            
        fclose(out);*/

        std::ofstream ofs(fileName, std::ios_base::binary);
        if (ofs) {
            ofs.write(fileBuffer.data(), fileBufferBytes);
        }
    }

    virtual void contents(std::vector<char> &contents) {
        contents.assign(fileBuffer.data(), fileBuffer.data() + fileBufferBytes);
    }
};