#pragma once
#include "conversion.h"

#include <string>
#include <vector>

inline const char* str2chr(const char* s) { return s; }
inline const char* str2chr(const std::string& s) { return s.c_str(); }

/******************************************************************/
class AbstractColumn {
public:
    //using OutputFunc = std::function<void(int, char*&)>;

    const bool isNumeric;
    const std::string name;
    const int numRows;
    const int width;

    AbstractColumn(bool isNumeric, const std::string& name, int numRows, int width) 
        : isNumeric(isNumeric), name(name), numRows(numRows), width(width) {}
    virtual ~AbstractColumn() {}

    virtual void output(int id, char*& dst) const = 0; 
};


template <class T>
class Column : public AbstractColumn {
protected:
    std::vector<T> values;
public:
    const std::vector<T>& getValues() const { return values; }

    Column(bool isNumeric, const std::string& name, int numRows, int width) 
        : AbstractColumn(isNumeric, name, numRows, width), values(numRows) {}

    Column(bool isNumeric, const std::string& name, std::vector<T>&& values, int width)
        : AbstractColumn(isNumeric, name, (int) values.size(), width), values(std::move(values)) {}
};

/******************************************************************/
class NumericColumn : public Column<int> {
public:

    const int numDecimals{ 0 };

    void output(int id, char*& dst) const override {
        
        int bytes = FixedPoint<int>::toString(values[id], numDecimals, dst);
      
        if (width > 0) {
            // right align number 
            int delta = width - bytes;
            for (int i = bytes - 1; i >= 0; --i) {
                dst[i + delta] = dst[i];
            }

            for (int i = 0; i < delta; ++i) {
                dst[i] = ' ';
            }

            dst += width;
        }
        else if (width < 0) {
            // left align number
            dst += bytes;
            int delta = -width - bytes;
            for (int i = 0; i < delta; ++i) {
                *dst = ' ';
                ++dst;
            }
        } else{
            dst += bytes;
        }
    }

    template <class string_t>
    static NumericColumn* create(const std::string& name, const std::vector<string_t>& table, int width, int column_id, int row_stride) {
        // try to parse first element as number
        int n_decimals;
        int v = FixedPoint<int>::fromString(str2chr(table[column_id]), n_decimals);

        if (v != FixedPoint<int>::INVALID) {
            NumericColumn* out = new NumericColumn(name, (int) table.size() / row_stride, n_decimals, width);
            out->values[0] = v;

            // fill consecutive values
            int ir = 1;
            for (int i = column_id + row_stride; i < (int) table.size(); i += row_stride) {
                int local_decimals;
                int v = FixedPoint<int>::fromString(str2chr(table[i]), local_decimals);

//                if ((v != FixedPoint<int>::INVALID) && (local_decimals != n_decimals)) {      
                if ((v != FixedPoint<int>::INVALID) && (local_decimals == n_decimals)) {        // !!! SD
                    //FixedPoint<int>::alterPrecision(v, local_decimals, n_decimals);
                    out->values[ir++] = v;
                }
                else {
                    delete out;
                    return nullptr;
                }
            }

            return out;
        }

        return nullptr;
    }

    static NumericColumn* create(const std::string& name, const std::vector<char*>& rows, int width, int offset)
    {
        int n_decimals;
        int v = FixedPoint<int>::fromString(rows[0] + offset, width, n_decimals);

        if (v != FixedPoint<int>::INVALID) {

            NumericColumn* out = new NumericColumn(name, (int) rows.size(), n_decimals, width);

            for (int ir = 0; ir < (int) rows.size(); ++ir) {
                v = FixedPoint<int>::fromString(rows[ir] + offset, width, n_decimals);
                out->values[ir] = v;
            }

            return out;
        }

        return nullptr;
    }
    
    /*
    static NumericColumn* create(const std::string& name, const char* input, int numRows, int width, int stride) 
    {
        int n_decimals;
        int v = FixedPoint<int>::fromString(input, width, n_decimals);
        
        if (v != FixedPoint<int>::INVALID) {

            NumericColumn* out = new NumericColumn(name, numRows, n_decimals, width);

            for (int ir = 0; ir < numRows; ++ir) {
                v = FixedPoint<int>::fromString(input, width, n_decimals);
                out->values[ir] = v;
                input += stride;
            }

            return out;
        }

        return nullptr;
    }
    */


    // fills column on the basis of the reference vector
    NumericColumn(const std::string& name, int numDecimals, int width, std::vector<int>&& values)
        : Column<int>(true, name, std::move(values), width), numDecimals(numDecimals) {

    }

private:
    NumericColumn(const std::string& name, int numRows, int numDecimals, int width)
        : Column<int>(true, name, numRows, width), numDecimals(numDecimals) {}

};

/******************************************************************/
class StringColumn : public Column<char*> {
public:
    char* const raw;
    size_t size_bytes;

    void output(int id, char*& dst) const override {
        char* v = values[id];
        while (*v) {
            *dst++ = *v++;
        }
    }

    template <class string_t>
    StringColumn(const std::string& name, const std::vector<string_t>& table, int width, int column_id, int row_stride, char*& raw)
        : Column<char*>(false, name, (int) table.size() / row_stride, width), raw(raw) {

        int ir = 0;
        auto p = raw;
        for (size_t i = column_id; i < table.size(); i += row_stride) {
            values[ir++] = p;
            const char* src = str2chr(table[i]);
            while ((*(p++) = *(src++))) {
            }
        }

        size_bytes = p - raw;
        raw += size_bytes;
    }

    StringColumn(const std::string& name, const std::vector<char*> rows, int width, int offset, char*& raw) :
        Column<char*>(false, name, rows.size(), width), raw(raw) {

        auto p = raw;
        for (size_t ir = 0; ir < rows.size(); ++ir) {
            values[ir] = p;

//            memcpy(p, rows[ir] + offset, width);
            char* q = rows[ir] + offset;
            for (int j = 0; j < width; ++j)
                *p++ = *q++;

//            p += width;
            *p = 0;
            ++p;
        }
        size_bytes = p - raw;
        raw += size_bytes;
    }

    // Fills pointers vector on the basis od raw data
    StringColumn(const std::string& name, int numRows, int width, char*& raw) 
        : Column<char*>(false, name, numRows, width), raw(raw) {
        for (int i = 0; i < numRows; ++i) {
            values[i] = raw;
            while (*raw++) {
            }
        }
        size_bytes = raw - this->raw;
    }
};
