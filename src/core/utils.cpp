#include "utils.h"
#include "version.h"
#include <cstring>

#include <filesystem>

// *****************************************************************
std::string UNSUPPORTED_ARCHIVE_INFO(uint32_t archive_ver)
{
	return
		"Archive version: " + to_string(archive_ver / 100) + "." + to_string(archive_ver % 100) + " is not supported by the tool.\n" +
		APP_NAME + " " + APP_VERSION_STR + " supports archives from ver. " + MIN_SUPPORTED_ARCHIVE_VERSION_STR + " to ver. " + MAX_SUPPORTED_ARCHIVE_VERSION_STR + "\n";
}

// *****************************************************************
bool operator<(const struct coords_6_t& l, const struct coords_6_t& r)
{
	if (l.x2 != r.x2)   return l.x2 < r.x2;
	if (l.x3 != r.x3)   return l.x3 < r.x3;
	if (l.y3 != r.y3)   return l.y3 < r.y3;

	if (l.x0 != r.x0)   return l.x0 < r.x0;
	if (l.y0 != r.y0)   return l.y0 < r.y0;
	return l.z0 < r.z0;
}

// *****************************************************************
vector<string> split(const string& str)
{
	vector<string> parts;

	parts.reserve(16);

	bool was_space = true;

	for (const auto c : str)
	{
		if (c == ' ')
		{
			was_space = true;
		}
		else
		{
			if (c > 32)							// avoid \n etc.
			{
				if (was_space)
				{
					parts.emplace_back("");

					was_space = false;
				}

				parts.back().push_back(c);
			}
		}
	}

	return parts;
}

// *****************************************************************
string char_to_str(char c)
{
	static const string arr[] = { "ALA", "", "CYS", "ASP", "GLU", "PHE", "GLY", "HIS", "ILE", "", "LYS", "LEU", "MET", "ASN", "", "PRO", "GLN", "ARG", "SER", "THR", "SEC", "VAL", "TRP", "", "TYR", "" };

	if (c < 'A' || c > 'Z')
		return "";

	return arr[c - 'A'];
}

// *****************************************************************
char str_to_char(const string &aa_name)
{
	static const vector<string> arr = { "ALA", "", "CYS", "ASP", "GLU", "PHE", "GLY", "HIS", "ILE", "", "LYS", "LEU", "MET", "ASN", "", "PRO", "GLN", "ARG", "SER", "THR", "SEC", "VAL", "TRP", "", "TYR", "" };

	auto p = find(arr.begin(), arr.end(), aa_name);

	if (p == arr.end())
		return ' ';
	else
		return 'A' + (char) (p - arr.begin());
}

// *****************************************************************
void trim(char* str)
{
	auto len = strlen(str);

	while (len && str[len - 1] == ' ')
		str[--len] = 0;

	char* p = str;
	while (*p && *p == ' ')
		++p;

	if (p != str)
	{
		while (*p)
			*str++ = *p++;
		*str = 0;
	}
}

// *****************************************************************
string trim_to_str(const char* str)
{
	string r;

	bool left_space = true;

	for (const char* p = str; *p; ++p)
	{
		if (left_space)
		{
			if (*p != ' ')
			{
				r.push_back(*p);
				left_space = false;
			}
		}
		else
			r.push_back(*p);
	}

	while (!r.empty() && r.back() == ' ')
		r.pop_back();

	return r;
}

// EOF
