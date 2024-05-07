#pragma once

#include <algorithm>
#include "columns.h"


/******************************************************************/
class Entry {
public:
    const bool isLoop;
    const std::string name;

    Entry(bool isLoop, const std::string& name) : isLoop(isLoop), name(name) {}
    virtual ~Entry() {}
};

/******************************************************************/
class BlockEntry : public Entry {
public:
    const char* data;
    const size_t size;

    BlockEntry(const std::string& name, size_t size, char*& raw) : Entry(false, name), data(raw), size(size) {
        raw += size;
        *raw = 0;
        ++raw;
    }
};

/******************************************************************/
class SimpleEntry : public Entry {
public:
    const std::vector<std::string> attributes;
    const std::vector<std::string> values;

    SimpleEntry(
        const std::string& name,
        std::vector<std::string>&& attributes,
        std::vector<std::string>&& values) :
        Entry(false, name), attributes(std::move(attributes)), values(std::move(values)) {}
};

/******************************************************************/
class LoopEntry : public Entry {
public:
    enum class Type {
        Standard, Atom, Hetatm, Sigatm, Anisou, Siguij
    };

    Type getType() const { return type; }
    void setType(Type t) { type = t; }

private:
    Type type;
   
    std::vector<AbstractColumn*> columns;
public:
    
    static Type str2type(const std::string& s) {
        if (s == "ATOM")            { return LoopEntry::Type::Atom; } 
        else if (s == "SIGATM")     { return LoopEntry::Type::Sigatm; }
        else if (s == "HETATM")     { return LoopEntry::Type::Hetatm; }
        else if (s == "ANISOU")     { return LoopEntry::Type::Anisou; } 
        else if (s == "SIGUIJ")     { return LoopEntry::Type::Siguij; } 
      
        return LoopEntry::Type::Standard;
    }

    static std::string type2str(Type type) {
        switch (type) {
        case Type::Atom: return "ATOM";
        case Type::Hetatm: return "HETATM";
        case Type::Anisou: return "ANISOU";
        case Type::Siguij: return "SIGUIJ";
        case Type::Sigatm: return "SIGATM";
        case Type::Standard: return "<standard>";
        }

        return "";
    }

    const std::vector<AbstractColumn*>& getColumns() const { return columns; }

    void addColumn(AbstractColumn* col) { columns.push_back(col); }

    AbstractColumn* findColumn(const std::string& name) const {
        auto it = std::find_if(columns.begin(), columns.end(),
            [&name](const AbstractColumn* c)->bool { return c->name == name; });

        return (it != columns.end()) ? (*it) : nullptr;
    }

    int getRowCount() const { return columns.size() == 0 ? 0 : columns[0]->numRows; }

    LoopEntry(const std::string& name, Type type) : Entry(true, name), type(type) {}
    ~LoopEntry() {
        for (auto c : columns) { delete c; }
    }
};
