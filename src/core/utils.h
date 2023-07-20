#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <regex>
#include <filesystem>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "params.h"

using namespace std;

// *****************************************************************
template<typename T>
T pow10(int e)
{
    T r = 1;
    for (int i = 0; i < e; ++i)
        r *= 10;

    return r;
}

// *****************************************************************
struct coords_6_t
{
    double x2;
    double x3, y3;
    double x0, y0, z0;
};

bool operator<(const struct coords_6_t& l, const struct coords_6_t& r);

// *****************************************************************
struct distances_6_t
{
    double b12, b13, b23;
    double q1, q2, q3;

    distances_6_t(double _b12, double _b13, double _b23, double _q1, double _q2, double _q3) :
        b12(_b12), b13(_b13), b23(_b23), q1(_q1), q2(_q2), q3(_q3)
    {}
};

vector<string> split(const string& str);

void trim(char* str);
string trim_to_str(const char *str);

// *****************************************************************
constexpr uint32_t char_to_code(char c)
{
    //                      A  -  C  D  E  F  G  H  I  -  K   L   M   N  -   P   Q   R   S   T   U   V   W  -   Y  -
    const uint32_t arr[] = {1, 0, 2, 3, 4, 5, 6, 7, 8, 0, 9, 10, 11, 12, 0, 13, 14, 15, 16, 17, 18, 19, 20, 0, 21, 0};
    
    if (c < 'A' || c > 'Z')
        return 0;

    return arr[c - 'A'];
}

string char_to_str(char c);
char str_to_char(const string& aa_name);

class file_type_checker
{
    inline const static string ext_cif = ".cif"s;
    inline const static string ext_CIF = ".CIF"s;
    inline const static string ext_pdb = ".pdb"s;
    inline const static string ext_PDB = ".pdb"s;
    inline const static string ext_pae = ".json"s;
    inline const static string ext_PAE = ".JSON"s;
    inline const static string ext_conf = ".json"s;
    inline const static string ext_CONF = ".JSON"s;
    inline const static string ext_json = ".json"s;
    inline const static string ext_JSON = ".JSON"s;

    inline const static regex re_pae{ "(.*)predicted_aligned_error(.*)" };
    inline const static regex re_conf{ "(.*)confidence(.*)" };

    inline const static regex re_CIF{ "(.*)\\.[cC][iI][fF](\\.gz)?" };
    inline const static regex re_PDB{ "(.*)\\.[pP][dD][bB](\\.gz)?" };
    inline const static regex re_PAE{ "(.*)predicted_aligned_error(.*)\\.[jJ][sS][oO][nN](\\.gz)?" };
    inline const static regex re_CONF{ "(.*)confidence(.*)\\.[jJ][sS][oO][nN](\\.gz)?" };

public:
    file_type_checker() = default;
    ~file_type_checker() = default;

    static pair<file_type_t, bool> check(const string& fn)
    {
        smatch sm;

        if (regex_match(fn, sm, re_CIF))
            return make_pair(file_type_t::CIF, sm[2].length() != 0);
        if (regex_match(fn, sm, re_PDB))
            return make_pair(file_type_t::PDB, sm[2].length() != 0);
        if (regex_match(fn, sm, re_PAE))
            return make_pair(file_type_t::PAE, sm[3].length() != 0);
        if (regex_match(fn, sm, re_CONF))
            return make_pair(file_type_t::CONF, sm[3].length() != 0);

        return make_pair(file_type_t::other, false);
    }

    static bool matches(const string& fn, file_type_t expected_ft)
    {
        auto ft = check(fn);

        if (expected_ft == file_type_t::CIF)
            return ft.first == file_type_t::CIF;
        else if (expected_ft == file_type_t::PDB)
            return ft.first == file_type_t::PDB;
        else if (expected_ft == file_type_t::PAE)
            return ft.first == file_type_t::PAE;
        else if (expected_ft == file_type_t::CONF)
            return ft.first == file_type_t::CONF;
        else if (expected_ft == file_type_t::ALL)
            return ft.first == file_type_t::CIF || ft.first == file_type_t::PDB || ft.first == file_type_t::PAE || ft.first == file_type_t::CONF;

        return false;
    }

    static bool is_gzipped(const string& fn)
    {
        if (fn.length() <= 3)
            return false;

        return fn.substr(fn.size() - 3, 3) == ".gz";
    }

    static string extract_stem(const string& fn)
    {
        if (is_gzipped(fn))
            return std::filesystem::path(fn).stem().stem().string();
        else
            return std::filesystem::path(fn).stem().string();
    }
};

template<typename T>
class bounded_queue
{
    size_t max_size;
    queue<T> q;
    mutex mtx;
    condition_variable cv_empty;
    condition_variable cv_full;
    bool completed;

public:
    bounded_queue(size_t max_size) :
        max_size(max_size),
        completed(false)
    {}

    void push(const T& x)
    {
        unique_lock<mutex> lck(mtx);
        cv_full.wait(lck, [&] {return q.size() < max_size; });
        q.push(x);

        cv_empty.notify_one();
    }

    void mark_completed()
    {
        lock_guard<mutex> lck(mtx);
        completed = true;

        cv_empty.notify_all();
    }

    bool pop(T& x)
    {
        unique_lock<mutex> lck(mtx);
        cv_empty.wait(lck, [&] {return !q.empty() || completed; });

        if (!q.empty())
        {
            x = q.front();
            q.pop();
            cv_full.notify_one();

            return true;
        }

        return false;
    }
};

// EOF