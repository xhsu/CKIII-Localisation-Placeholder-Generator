// CKIII Localisation Placeholder Generator.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


// C++ STL
#include <chrono>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

// MSVC STL modules
//import std.core;
//import std.filesystem;

using namespace std;

unordered_map<string, string> g_rgszFileSuffix =
{
	{"english",			"_l_english.yml"},
	{"french",			"_l_french.yml"},
	{"german",			"_l_german.yml"},
	{"korean",			"_l_korean.yml"},
	{"russian",			"_l_russian.yml"},
	{"simp_chinese",	"_l_simp_chinese.yml"},
	{"spanish",			"_l_spanish.yml"},
};

unordered_map<string, string> g_rgszLocalisationFolder =
{
	{"english",			"english\\"},
	{"french",			"french\\"},
	{"german",			"german\\"},
	{"korean",			"korean\\"},
	{"russian",			"russian\\"},
	{"simp_chinese",	"simp_chinese\\"},
	{"spanish",			"spanish\\"},
};

unordered_map<string, string> g_rgszFieldKey =
{
	{"english",			"l_english:"},
	{"french",			"l_french:"},
	{"german",			"l_german:"},
	{"korean",			"l_korean:"},
	{"russian",			"l_russian:"},
	{"simp_chinese",	"l_simp_chinese:"},
	{"spanish",			"l_spanish:"},
};

string UTIL_LanguageForShort(char c)
{
	switch (c)
	{
	case 'f':
		return "french";

	case 'g':
		return "german";

	case 'k':
		return "korean";

	case 'r':
		return "russian";

	case 'c':
		return "simp_chinese";

	case 's':
		return "spanish";

	default:
		cout << "Unknown language code '" << c << "' !" << endl;	// don't input random stuff, thanks for your cooperation.

	case 'e':
		return "english";
	}
}

void UTIL_ReplaceAll(string& str, const string& from, const string& to)
{
	if (from.empty())
		return;

	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();	// In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

void UTIL_Trim(string& str)
{
	static const auto fnNotSpace = [](unsigned char c) { return !std::isspace(c); };

	str.erase(str.begin(), find_if(str.begin(), str.end(), fnNotSpace));	// L trim
	str.erase(find_if(str.rbegin(), str.rend(), fnNotSpace).base(), str.end());	// R trim. std::reverse_iterator<Iter>::base() represents the true position of reversed iterator.
}

auto UTIL_GetSpaceCount(const string& str)	// L space
{
	static const auto fnNotSpace = [](unsigned char c) { return !std::isspace(c); };

	return distance(str.begin(), find_if(str.begin(), str.end(), fnNotSpace));
}

auto LoadYAMLIntoMap(const filesystem::path& hPath, unordered_map<string, string>& m)
{
	invoke_result_t<decltype(UTIL_GetSpaceCount), const string&> iResult = 0;

	if (fstream hFileStream(hPath, ios::in); hFileStream)
	{
		while (!hFileStream.eof())
		{
			string szBuffer;
			getline(hFileStream, szBuffer);

			if (!iResult)
				iResult = UTIL_GetSpaceCount(szBuffer);	// Gets space counts before trimming.

			UTIL_Trim(szBuffer);

			if (szBuffer.ends_with(':'))	// Skip the first line, like "l_english:" stuff.
				continue;

			size_t iBreakPos = szBuffer.find_first_of(':');
			m[szBuffer.substr(0U, iBreakPos)] = move(szBuffer);
		}
	}

	return iResult;
}

void CreatePlaceholder(const string& from, const string& to)
{
	string szBuffer;

	for (const auto& hPath : filesystem::recursive_directory_iterator(from))	// "english"
	{
		if (hPath.is_directory() || !hPath.path().string().ends_with(g_rgszFileSuffix[from]))	// "_l_english.yml"
			continue;

		string szOutputFilePath = hPath.path().string();
		UTIL_ReplaceAll(szOutputFilePath, g_rgszFileSuffix[from], g_rgszFileSuffix[to]);	// "_l_english.yml", "_l_simp_chinese.yml"
		UTIL_ReplaceAll(szOutputFilePath, g_rgszLocalisationFolder[from], g_rgszLocalisationFolder[to]);	// "english\\", "simp_chinese\\"

		auto hOutputFilePath = filesystem::path(szOutputFilePath);

		if (!filesystem::exists(hOutputFilePath))
		{
			if (fstream hSourceStream(hPath.path(), ios::in); hSourceStream)
			{
				if (auto hOutputFileDir = hOutputFilePath.parent_path(); !filesystem::exists(hOutputFileDir))
				{
					filesystem::create_directories(hOutputFileDir);	// Use create_directories() to create entire folder structure.
					cout << "Folder Created: " << hOutputFileDir.string() << endl;
				}

				ofstream output_file(szOutputFilePath, ios::out);

				if (!output_file)
					continue;

				while (!hSourceStream.eof())
				{
					getline(hSourceStream, szBuffer);
					UTIL_ReplaceAll(szBuffer, g_rgszFieldKey[from], g_rgszFieldKey[to]);	// "l_english:", "l_simp_chinese:"

					output_file << szBuffer << endl;
				}

				cout << "Created: " << szOutputFilePath << endl;
			}
			else
				cout << "Error: Could not open source file: " << hPath.path() << endl;
		}
		else	// i.e. Need to compare with the source one.
		{
			bool bFirstInserted = false;
			unordered_map<string, string> mSource, mOutput;

			LoadYAMLIntoMap(hPath.path(), mSource);
			auto iSpaceCounts = LoadYAMLIntoMap(hOutputFilePath, mOutput);
			const string szSpaces = string(iSpaceCounts, ' ');

			if (ofstream hOutputStream(szOutputFilePath, ios::out | ios::app); hOutputStream)	// std::ios::app means output this file with append mode. This will not have the original one deleted.
			{
				for (auto& [szKey, szValue] : mSource)
				{
					if (mOutput.contains(szKey))
						continue;

					if (!bFirstInserted)
					{
						auto hTimeNow = chrono::system_clock::to_time_t(chrono::system_clock::now());	// #FIXME MSVC C++20 have no std::chrono::operator<< (std::chrono::year_month_day)

						hOutputStream << endl << szSpaces << "# Automatically added at: " << ctime(&hTimeNow);	// end of line included.
						bFirstInserted = true;
					}

					hOutputStream << szSpaces << szValue << endl;
					cout << "Inserting: " << quoted(szKey) << " into file: " << szOutputFilePath << endl;
				}
			}
		}
	}
}

int main()
{
	char from = 0, to = 0;
	string szFrom, szTo;

LAB_ASK_FOR_FROM:;
	cout << "From what language?" << endl;
	cin >> from;
	szFrom = UTIL_LanguageForShort(from);

	if (!filesystem::exists(szFrom))
	{
		cout << "Localisation folder " << quoted(szFrom) << " no found!" << endl;
		goto LAB_ASK_FOR_FROM;
	}

	cout << "Generate from language: " << quoted(szFrom) << endl << endl;

	cout << "To what language?" << endl;
	cin >> to;

	if (to == 'a')	// ALL languages.
	{
		for (auto& [szKey, szValue] : g_rgszLocalisationFolder)
		{
			if (szKey == szFrom)	// Skipping original language.
				continue;

			cout << "Generating placeholder for language " << quoted(szKey) << " ..." << endl << endl;
			CreatePlaceholder(szFrom, szKey);
			cout << endl;
		}
	}
	else
	{
		szTo = UTIL_LanguageForShort(to);
		cout << "Generating placeholder for language " << quoted(szTo) << " ..." << endl << endl;

		CreatePlaceholder(szFrom, szTo);
		cout << endl;
	}

	system("pause");
	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
