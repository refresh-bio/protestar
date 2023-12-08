#pragma once
#include <vector>
#include <string>

using namespace std;

class ExtDictBase {
protected:
    vector<char> type_specific_dict;

    void dict_append(vector<char>& dict, const string& str, char sep)
    {
        dict.insert(dict.end(), str.begin(), str.end());
        dict.push_back(sep);
    }

    void dict_append(const string& str, char sep)
    {
        type_specific_dict.insert(type_specific_dict.end(), str.begin(), str.end());
        type_specific_dict.push_back(sep);
    }

public:
    ExtDictBase() = default;
};