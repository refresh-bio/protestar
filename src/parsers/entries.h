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
        Standard, Atom, Hetatm
    };

    Type getType() const { return type; }
    void setType(Type t) { type = t; }

private:
    Type type;
   
    std::vector<AbstractColumn*> columns;
public:
    
    static Type str2type(const std::string& s) {
        return (s == "ATOM")
            ? LoopEntry::Type::Atom
            : ( (s == "HETATM")
                ? LoopEntry::Type::Hetatm
                : LoopEntry::Type::Standard);
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
