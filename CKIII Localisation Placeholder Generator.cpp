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

// My custom modules
import UtlWinConsole;

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

string UTIL_LanguageForShort(char c) noexcept
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
		cout_r() << "Unknown language code '" << c << "' !" << endl;	// don't input random stuff, thanks for your cooperation.
		[[fallthrough]];

	case 'e':
		return "english";
	}
}

void UTIL_ReplaceAll(string& str, const string& from, const string& to) noexcept
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

void UTIL_Trim(string& str) noexcept
{
	static const auto fnNotSpace = [](unsigned char c) { return !std::isspace(c); };

	str.erase(str.begin(), find_if(str.begin(), str.end(), fnNotSpace));	// L trim
	str.erase(find_if(str.rbegin(), str.rend(), fnNotSpace).base(), str.end());	// R trim. std::reverse_iterator<Iter>::base() represents the true position of reversed iterator.
}

auto UTIL_GetSpaceCount(const string& str) noexcept	// L space
{
	static const auto fnNotSpace = [](unsigned char c) { return !std::isspace(c); };

	return distance(str.begin(), find_if(str.begin(), str.end(), fnNotSpace));
}

auto LoadYAMLIntoMap(const filesystem::path& hPath, unordered_map<string, string>& m) noexcept
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

			if (szBuffer.starts_with('#'))	// Skip the commented line in original texts.
				continue;

			if (szBuffer.ends_with(':'))	// Skip the first line, like "l_english:" stuff.
				continue;

			if (szBuffer.empty())
				continue;

			size_t iBreakPos = szBuffer.find_first_of(':');
			m[szBuffer.substr(0U, iBreakPos)] = move(szBuffer);
		}
	}

	return iResult;
}

void CreatePlaceholder(const filesystem::path& mod, const string& from, const string& to) noexcept
{
	string szBuffer;

	for (const auto& hPath : filesystem::recursive_directory_iterator(mod / "localization" / from))	// "english"
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
					cout_silver() << "Folder Created: " << hOutputFileDir.string() << endl;
				}

				ofstream output_file(szOutputFilePath, ios::out);

				if (!output_file)
					continue;

				while (!hSourceStream.eof())
				{
					getline(hSourceStream, szBuffer);
					UTIL_ReplaceAll(szBuffer, g_rgszFieldKey[from], g_rgszFieldKey[to]);	// "l_english:", "l_simp_chinese:"

					if (szBuffer.length())	// Skip empty lines.
						output_file << szBuffer << endl;
				}

				cout_silver() << "Created: " << szOutputFilePath << endl;
			}
			else
				cout_r() << "Error: Could not open source file: " << hPath.path() << endl;
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
					cout_silver() << "Inserting: " << quoted(szKey) << " into file: " << szOutputFilePath << endl;
				}

				if (!bFirstInserted)	// Nothing had inserted.
					cout_gray() << "Skipping: " << szOutputFilePath << endl;
			}
		}
	}
}

string GetModName(const filesystem::path& mod) noexcept
{
	if (!atoi(mod.filename().string().c_str()))
		return mod.filename().string();	// It must be a mod with proper text name.

	auto hDesc = mod / "descriptor.mod";

	if (!filesystem::exists(hDesc))
		return "ERROR - NO DESCRIPTOR FOUNDED!";

	string ret = "";

	if (FILE* f = fopen(hDesc.string().c_str(), "rb"); f != nullptr)
	{
		fseek(f, 0, SEEK_END);
		auto iLength = ftell(f);
		char* pbuf = (char*)calloc(iLength + 1, sizeof(char));

		fseek(f, 0, SEEK_SET);
		fread(pbuf, sizeof(char), iLength, f);
		fclose(f);
		f = nullptr;

		for (const char* p = pbuf; p != pbuf + iLength + 1; ++p)
		{
			if (!_strnicmp(p, "name", 4))
			{
				for (; p != pbuf + iLength + 1; ++p)
				{
					if (*p == '"')
					{
						++p;
						break;
					}
				}

				ret = p;
				break;
			}
		}

		free(pbuf);
		ret.erase(ret.find_first_of('"'));
	}

	return ret;
}

int main(int argc, char** argv) noexcept
{
	ios_base::sync_with_stdio(false);	// We are not using C function "printf" thus just turn it off.

	const bool bBatchMode = !filesystem::exists("descriptor.mod");

	char from = 0, to = 0;
	string szFrom, szTo;

	switch (argc)
	{
	case 3:
		to = *(argv[2]);
		[[fallthrough]];
	case 2:
		szFrom = UTIL_LanguageForShort(from = *(argv[1]));
		[[fallthrough]];
	default:
		break;
	}

	switch (argc)
	{
	case 3:
		goto LAB_ANALYZING_ARG;
	case 2:
		goto LAB_ASK_FOR_TO;
	default:
		break;
	}

LAB_ASK_FOR_FROM:;
	cout_w() << "From what language?" << endl;
	cin >> from;
	szFrom = UTIL_LanguageForShort(from);

	if (!bBatchMode && !filesystem::exists(szFrom))
	{
		cout_r() << "Localisation folder " << quoted(szFrom) << " no found!" << endl;
		goto LAB_ASK_FOR_FROM;
	}

	cout_w() << "Generate from language: " << quoted(szFrom) << endl << endl;

LAB_ASK_FOR_TO:;
	cout_w() << "To what language?" << endl;
	cin >> to;

	if (to == from)
	{
		cout_r() << "Same languague between source and target." << endl;
		goto LAB_ASK_FOR_TO;
	}

LAB_ANALYZING_ARG:;

	const auto fnCreatePlaceholderForMod = [&](const filesystem::path& hModPath = filesystem::current_path())
	{
		if (hModPath != filesystem::current_path())
			cout_skyblue() << "\nProcessing mod: " << cyan_text << GetModName(hModPath) << endl;

		if (!filesystem::exists(hModPath / "localization" / szFrom))
		{
			cout_r() << "Skipping: Source language " << quoted(szFrom) << " no found." << endl;
			return;
		}

		if (to == 'a')	// ALL languages.
		{
			for (auto& [szKey, szValue] : g_rgszLocalisationFolder)
			{
				if (szKey == szFrom)	// Skipping original language.
					continue;

				cout_skyblue() << "Generating placeholder for language " << cyan_text << quoted(szKey) << skyblue_text << " ..." << endl;
				CreatePlaceholder(hModPath, szFrom, szKey);
				cout << endl;
			}
		}
		else
		{
			szTo = UTIL_LanguageForShort(to);
			cout_skyblue() << "Generating placeholder for language " << cyan_text << quoted(szTo) << skyblue_text << " ..." << endl;

			CreatePlaceholder(hModPath, szFrom, szTo);
			cout << endl;
		}
	};

	if (bBatchMode)	// For all mods in this folder.
	{
		for (const auto& hEntry : filesystem::directory_iterator(filesystem::current_path()))
		{
			if (!hEntry.is_directory())
				continue;

			if (!filesystem::exists(hEntry.path() / "descriptor.mod"))	// Is this folder a mod?
			{
				cout_gold() << "Skipping non-mod folder " << quoted(hEntry.path().filename().string()) << '.' << endl;
				continue;
			}
			
			if (!filesystem::exists(hEntry.path() / "localization"))	// Is this mod contains a localization folder?
			{
				cout_gold() << "Skipping non-localizable mod " << quoted(hEntry.path().filename().string()) << '.' << endl;
				continue;
			}

			fnCreatePlaceholderForMod(hEntry);
		}
	}
	else
		fnCreatePlaceholderForMod();

	cout_w();
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
